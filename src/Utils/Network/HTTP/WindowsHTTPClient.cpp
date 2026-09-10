//
// Created by Monika on 07.09.2026.
//

#include <Utils/Network/IHTTPClient.h>

#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

#ifdef DELETE
    #undef DELETE
#endif

namespace SR_NETWORK_NS {
    class WindowsHTTPClient : public IHTTPClient {
    public:
        WindowsHTTPClient();
        ~WindowsHTTPClient() override;

        bool Send(const HTTPRequest& request, HTTPResponse& response, HTTPDataCallback onData) override;

    private:
        HINTERNET m_session = nullptr;

    };

    std::wstring UTF8ToWide(std::string_view string) {
        if (string.empty())
            return {};
        const int size = MultiByteToWideChar( CP_UTF8, 0, string.data(), static_cast<int>(string.size()), nullptr, 0);
        if (size <= 0)
            return {};
        std::wstring result(size, L'\0');
        MultiByteToWideChar( CP_UTF8, 0, string.data(), static_cast<int>(string.size()), result.data(), size);
        return result;
    }

    std::string WideToUTF8(std::wstring_view string) {
        if (string.empty())
            return {};
        const int size = WideCharToMultiByte( CP_UTF8, 0, string.data(), static_cast<int>(string.size()), nullptr, 0, nullptr, nullptr);
        if (size <= 0)
            return {};
        std::string result(size, '\0');
        WideCharToMultiByte( CP_UTF8, 0, string.data(), static_cast<int>(string.size()), result.data(), size, nullptr, nullptr);
        return result;
    }

    LPCWSTR GetMethod(HTTPMethod method) {
        switch (method) {
            case HTTPMethod::GET: return L"GET";
            case HTTPMethod::POST: return L"POST";
            case HTTPMethod::PUT: return L"PUT";
            case HTTPMethod::PATCH: return L"PATCH";
            case HTTPMethod::DELETE: return L"DELETE";
            default:
                SRHalt("WindowsHTTPClient::GetMethod() : unknown method!");
                return nullptr;
        }
    }

    bool WindowsHTTPClient::Send(const HTTPRequest& request, HTTPResponse& response, HTTPDataCallback onData) {
        response = {};

        if (!m_session) {
            SR_ERROR("WindowsHTTPClient::Send() : session is not valid!");
            return false;
        }

        const std::wstring url = UTF8ToWide(request.url);
        if (url.empty()) {
            SR_ERROR("WindowsHTTPClient::Send() : url is empty!");
            return false;
        }

        URL_COMPONENTS components{};
        components.dwStructSize = sizeof(components);

        components.dwHostNameLength = static_cast<DWORD>(-1);
        components.dwUrlPathLength = static_cast<DWORD>(-1);
        components.dwExtraInfoLength = static_cast<DWORD>(-1);

        if (!WinHttpCrackUrl(url.c_str(), static_cast<DWORD>(url.size()), 0, &components)) {
            const DWORD error = GetLastError();
            SR_ERROR("WindowsHTTPClient::Send() : WinHttpCrackUrl failed! Error: {}", static_cast<int>(error));
            return false;
        }

        std::wstring host(static_cast<size_t>(components.dwHostNameLength), L'\0');
        std::wstring path(static_cast<size_t>(components.dwUrlPathLength), L'\0');
        std::wstring extraInfo(static_cast<size_t>(components.dwExtraInfoLength), L'\0');

        std::memcpy(host.data(), components.lpszHostName, components.dwHostNameLength * sizeof(wchar_t));
        std::memcpy(path.data(), components.lpszUrlPath, components.dwUrlPathLength * sizeof(wchar_t));
        std::memcpy(extraInfo.data(), components.lpszExtraInfo, components.dwExtraInfoLength * sizeof(wchar_t));

        if (host.empty())
            return false;

        const LPCWSTR method = GetMethod(request.method);
        if (!method)
            return false;

        std::wstring wideHost = host;
        HINTERNET connection = WinHttpConnect(m_session, wideHost.c_str(), components.nPort, 0);
        if (!connection)
            return false;

        const bool secure = components.nPort == INTERNET_DEFAULT_HTTPS_PORT;
        const DWORD flags = secure ? WINHTTP_FLAG_SECURE : 0;

        std::wstring requestPath = path;

        if (requestPath.empty())
            requestPath = L"/";

        requestPath += extraInfo;

        HINTERNET handle = WinHttpOpenRequest(
            connection,
            method,
            requestPath.c_str(),
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            flags);

        if (!handle) {
            WinHttpCloseHandle(connection);
            return false;
        }

        bool success = true;

        do {
            /*
             * Headers
             */
            std::wstring headers;

            for (const auto& [name, value] : request.headers) {
                const std::wstring wideName = UTF8ToWide(name);
                const std::wstring wideValue = UTF8ToWide(value);

                if (wideName.empty())
                    continue;

                headers += wideName;
                headers += L": ";
                headers += wideValue;
                headers += L"\r\n";
            }

            /* Request body */
            LPVOID body = nullptr;
            if (!request.body.empty()) {
                body = const_cast<std::byte*>( request.body.data());
            }

            const DWORD bodySize = static_cast<DWORD>(request.body.size());

            /* Send request */
            if (!WinHttpSendRequest(
                handle,
                headers.empty()
                ? WINHTTP_NO_ADDITIONAL_HEADERS
                : headers.c_str(),
                headers.empty()
                ? 0
                : static_cast<DWORD>(-1L),
                body,
                bodySize,
                bodySize,
                0))
            {
                SR_ERROR("WindowsHTTPClient::Send() : WinHttpSendRequest failed! Error: {}", static_cast<int>(GetLastError()));
                break;
            }

            /* Receive response */
            if (!WinHttpReceiveResponse(handle, nullptr)) {
                SR_ERROR("WindowsHTTPClient::Send() : WinHttpReceiveResponse failed! Error: {}", static_cast<int>(GetLastError()));
                break;
            }

            /* Status code */
            DWORD statusCode = 0;
            DWORD statusCodeSize = sizeof(statusCode);

            if (!WinHttpQueryHeaders(
                handle,
                WINHTTP_QUERY_STATUS_CODE |
                WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX,
                &statusCode,
                &statusCodeSize,
                WINHTTP_NO_HEADER_INDEX))
            {
                break;
            }

            response.statusCode = static_cast<int>(statusCode);

            /*
             * Response headers.
             * Получаем весь блок заголовков.
             */
            DWORD headersSize = 0;

            WinHttpQueryHeaders(
                handle,
                WINHTTP_QUERY_RAW_HEADERS_CRLF,
                WINHTTP_HEADER_NAME_BY_INDEX,
                nullptr,
                &headersSize,
                WINHTTP_NO_HEADER_INDEX);

            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                std::wstring rawHeaders( headersSize / sizeof(wchar_t), L'\0');
                if (WinHttpQueryHeaders(
                    handle,
                    WINHTTP_QUERY_RAW_HEADERS_CRLF,
                    WINHTTP_HEADER_NAME_BY_INDEX,
                    rawHeaders.data(),
                    &headersSize,
                    WINHTTP_NO_HEADER_INDEX))
                {
                    // Убираем завершающий '\0'.
                    while (!rawHeaders.empty() && rawHeaders.back() == L'\0') {
                        rawHeaders.pop_back();
                    }

                    SizeType lineStart = 0;
                    bool isFirstLine = true;

                    while (lineStart < rawHeaders.size()) {
                        SizeType lineEnd = rawHeaders.find(L'\n', lineStart);

                        if (lineEnd == std::wstring::npos) {
                            lineEnd = rawHeaders.size();
                        }

                        // Убираем \r из CRLF.
                        if (lineEnd > lineStart && rawHeaders[lineEnd - 1] == L'\r') {
                            --lineEnd;
                        }

                        if (lineEnd > lineStart) {
                            if (!isFirstLine) {
                                const SizeType separator = rawHeaders.find(L':', lineStart);
                                if (separator != std::wstring::npos && separator > lineStart) {
                                    SizeType valueStart = separator + 1;

                                    while (valueStart < lineEnd && (rawHeaders[valueStart] == L' ' || rawHeaders[valueStart] == L'\t')) {
                                        ++valueStart;
                                    }

                                    String name = WideToUTF8({rawHeaders.data() + lineStart, separator - lineStart});
                                    String value = WideToUTF8({rawHeaders.data() + valueStart, lineEnd - valueStart});

                                    response.headers.emplace_back(Pair<String, String>(std::move(name), std::move(value)));
                                }
                            }

                            isFirstLine = false;
                        }

                        lineStart = lineEnd + 1;
                    }
                }
            }

            /*
             * Response body.
             *
             * Не выделяем весь body целиком.
             * WinHTTP отдаёт данные чанками.
             */
            std::array<std::byte, 64 * 1024> buffer{};

            while (true) {
                DWORD available = 0;

                if (!WinHttpQueryDataAvailable( handle, &available)) {
                    success = false;
                    break;
                }

                if (available == 0) {
                    success = true;
                    break;
                }

                while (available > 0) {
                    const DWORD toRead = std::min<DWORD>(available, static_cast<DWORD>(buffer.size()));

                    DWORD read = 0;

                    if (!WinHttpReadData(handle, buffer.data(), toRead, &read)) {
                        success = false;
                        break;
                    }

                    if (read == 0)
                        break;

                    if (onData && !onData({buffer.data(), read})) {
                        // Callback запросил отмену.
                        success = false;
                        break;
                    }

                    available -= read;
                }

                if (!success)
                    break;
            }

        } while (false);

        WinHttpCloseHandle(handle);
        WinHttpCloseHandle(connection);

        return success;
    }

    WindowsHTTPClient::WindowsHTTPClient() {
        m_session = WinHttpOpen(
            L"SREngine",
            WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0);

        if (!m_session) {
            SR_ERROR("WindowsHTTPClient::WindowsHTTPClient() : failed to open session!");
            return;
        }

        // Таймауты в миллисекундах.
        WinHttpSetTimeouts(m_session,
            15000, // resolve
            15000, // connect
            15000, // send
            30000 // receive
        );
    }

    WindowsHTTPClient::~WindowsHTTPClient() {
        if (m_session) {
            WinHttpCloseHandle(m_session);
            m_session = nullptr;
        }
    }

    IHTTPClient* CreateHTTPClient() {
        return new WindowsHTTPClient();
    }
}