//
// Created by Monika on 11.09.2026.
//

#include <Utils/Network/IHTTPClient.h>
#include <Utils/Platform/Platform.h>

#include <emscripten/emscripten.h>
#include <emscripten/threading.h>

#ifdef DELETE
    #undef DELETE
#endif

/*
 * IHTTPClient::Send() блокирующий, а в браузере дождаться ответа внутри одного вызова можно
 * только синхронным запросом. emscripten_fetch() с EMSCRIPTEN_FETCH_SYNCHRONOUS отказывается
 * работать в главном потоке браузера, а pthreads в сборке по умолчанию выключены, поэтому
 * используется синхронный XMLHttpRequest напрямую.
 *
 * Результат забирается в два прохода: первый выполняет запрос и сообщает размеры,
 * второй копирует данные в память, выделенную уже на стороне C++.
 * Так не нужно тянуть malloc/free через границу JS.
 */

EM_JS(int, SRWebHttpSyncRequest, (const char* pMethod, const char* pUrl, const char* pHeaders,
    const void* pBody, int bodySize, int* pOutStatus, int* pOutDataSize, int* pOutHeadersSize), {
    HEAP32[pOutStatus >> 2] = 0;
    HEAP32[pOutDataSize >> 2] = 0;
    HEAP32[pOutHeadersSize >> 2] = 0;

    globalThis.__srHttpResult = null;

    try {
        var xhr = new XMLHttpRequest();
        xhr.open(UTF8ToString(pMethod), UTF8ToString(pUrl), false);

        /// В главном потоке браузера синхронный XHR не даёт менять responseType,
        /// поэтому тело приходится получать текстом в бинарно-безопасной кодировке.
        var isBinary = false;
        try {
            xhr.responseType = "arraybuffer";
            isBinary = xhr.responseType === "arraybuffer";
        } catch (e) {
            isBinary = false;
        }

        if (!isBinary) {
            xhr.overrideMimeType("text/plain; charset=x-user-defined");
        }

        var headersBlock = UTF8ToString(pHeaders);
        if (headersBlock.length > 0) {
            var lines = headersBlock.split("\r\n");
            for (var i = 0; i < lines.length; ++i) {
                var separator = lines[i].indexOf(":");
                if (separator > 0) {
                    try {
                        xhr.setRequestHeader(lines[i].substring(0, separator), lines[i].substring(separator + 1));
                    } catch (e) {
                        /// Браузер запрещает часть заголовков (Host, Content-Length и прочие).
                    }
                }
            }
        }

        if (bodySize > 0) {
            xhr.send(HEAPU8.slice(pBody, pBody + bodySize));
        } else {
            xhr.send(null);
        }

        var bytes = null;

        if (isBinary) {
            bytes = xhr.response ? new Uint8Array(xhr.response) : new Uint8Array(0);
        } else {
            var text = xhr.responseText || "";
            bytes = new Uint8Array(text.length);
            for (var j = 0; j < text.length; ++j) {
                bytes[j] = text.charCodeAt(j) & 0xFF;
            }
        }

        var rawHeaders = xhr.getAllResponseHeaders() || "";

        globalThis.__srHttpResult = { data: bytes, headers: rawHeaders };

        HEAP32[pOutStatus >> 2] = xhr.status;
        HEAP32[pOutDataSize >> 2] = bytes.length;
        HEAP32[pOutHeadersSize >> 2] = lengthBytesUTF8(rawHeaders);

        return 1;
    } catch (e) {
        console.error("SRWebHttpSyncRequest() : request failed: " + e);
        globalThis.__srHttpResult = null;
        return 0;
    }
});

EM_JS(void, SRWebHttpTakeResult, (void* pData, int dataSize, char* pHeaders, int headersSize), {
    var result = globalThis.__srHttpResult;
    globalThis.__srHttpResult = null;

    if (!result) {
        return;
    }

    if (dataSize > 0) {
        HEAPU8.set(result.data.subarray(0, dataSize), pData);
    }

    if (headersSize > 0) {
        stringToUTF8(result.headers, pHeaders, headersSize + 1);
    }
});

EM_JS_DEPS(srengine_http, "$lengthBytesUTF8,$stringToUTF8,$UTF8ToString");

namespace SR_NETWORK_NS {
    class EmscriptenHTTPClient : public IHTTPClient {
    public:
        bool Send(const HTTPRequest& request, HTTPResponse& response, HTTPDataCallback onData) override;

    private:
        static const char* GetMethod(HTTPMethod method);
        static void ParseHeaders(std::string_view rawHeaders, HTTPResponse& response);

    };

    const char* EmscriptenHTTPClient::GetMethod(HTTPMethod method) {
        switch (method) {
            case HTTPMethod::GET: return "GET";
            case HTTPMethod::POST: return "POST";
            case HTTPMethod::PUT: return "PUT";
            case HTTPMethod::PATCH: return "PATCH";
            case HTTPMethod::DELETE: return "DELETE";
            default:
                SRHalt("EmscriptenHTTPClient::GetMethod() : unknown method!");
                return nullptr;
        }
    }

    void EmscriptenHTTPClient::ParseHeaders(std::string_view rawHeaders, HTTPResponse& response) {
        size_t lineStart = 0;

        while (lineStart < rawHeaders.size()) {
            size_t lineEnd = rawHeaders.find('\n', lineStart);

            if (lineEnd == std::string_view::npos) {
                lineEnd = rawHeaders.size();
            }

            /// Убираем \r из CRLF.
            size_t valueEnd = lineEnd;
            if (valueEnd > lineStart && rawHeaders[valueEnd - 1] == '\r') {
                --valueEnd;
            }

            const size_t separator = rawHeaders.find(':', lineStart);

            if (separator != std::string_view::npos && separator > lineStart && separator < valueEnd) {
                size_t valueStart = separator + 1;

                while (valueStart < valueEnd && (rawHeaders[valueStart] == ' ' || rawHeaders[valueStart] == '\t')) {
                    ++valueStart;
                }

                String name(rawHeaders.substr(lineStart, separator - lineStart));
                String value(rawHeaders.substr(valueStart, valueEnd - valueStart));

                response.headers.emplace_back(Pair<String, String>(std::move(name), std::move(value)));
            }

            lineStart = lineEnd + 1;
        }
    }

    bool EmscriptenHTTPClient::Send(const HTTPRequest& request, HTTPResponse& response, HTTPDataCallback onData) {
        response = {};

        if (request.url.empty()) {
            SR_ERROR("EmscriptenHTTPClient::Send() : url is empty!");
            return false;
        }

        const char* pMethod = GetMethod(request.method);
        if (!pMethod) {
            return false;
        }

        String headers;

        for (const auto& [name, value] : request.headers) {
            if (name.empty()) {
                continue;
            }

            if (!headers.empty()) {
                headers.append("\r\n");
            }

            headers.append(name);
            headers.append(":");
            headers.append(value);
        }

        int32_t statusCode = 0;
        int32_t dataSize = 0;
        int32_t headersSize = 0;

        const int result = SRWebHttpSyncRequest(
            pMethod,
            request.url.c_str(),
            headers.c_str(),
            request.body.empty() ? nullptr : request.body.data(),
            static_cast<int>(request.body.size()),
            &statusCode,
            &dataSize,
            &headersSize);

        response.statusCode = statusCode;

        if (!result) {
            SR_ERROR("EmscriptenHTTPClient::Send() : request failed!\n\tURL: {}", request.url);
            return false;
        }

        std::vector<std::byte> body(static_cast<size_t>(dataSize));
        std::string rawHeaders(static_cast<size_t>(headersSize) + 1, '\0');

        SRWebHttpTakeResult(
            body.empty() ? nullptr : body.data(),
            dataSize,
            rawHeaders.data(),
            headersSize);

        ParseHeaders(std::string_view(rawHeaders.data(), static_cast<size_t>(headersSize)), response);

        if (statusCode == 0) {
            /// Нулевой статус в браузере означает сетевую ошибку либо блокировку CORS.
            SR_ERROR("EmscriptenHTTPClient::Send() : request was blocked or the network is unavailable!"
                "\n\tCheck the CORS policy of the server.\n\tURL: {}", request.url);
            return false;
        }

        if (!onData || body.empty()) {
            return true;
        }

        /*
         * Синхронный XHR не стримит, тело уже целиком в памяти.
         * Отдаём его чанками, чтобы вызывающая сторона видела тот же контракт,
         * что и на остальных платформах.
         */
        constexpr size_t chunkSize = 64 * 1024;

        for (size_t offset = 0; offset < body.size(); offset += chunkSize) {
            const size_t size = std::min(chunkSize, body.size() - offset);

            if (!onData({body.data() + offset, size})) {
                /// Callback запросил отмену.
                return false;
            }
        }

        return true;
    }

    IHTTPClient* CreateHTTPClient() {
        return new EmscriptenHTTPClient();
    }
}
