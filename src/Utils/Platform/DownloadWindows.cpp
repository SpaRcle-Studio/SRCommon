#include <Utils/Network/Download.h>
#include <Utils/FileSystem/VFS.h>

#ifdef SR_WIN32
    #include <Windows.h>
    #include <wininet.h>

namespace SR_NETWORK_NS {
    bool DownloadInternal(const DownloadRequest& request, const SR_UTILS_NS::Path& temporary) {
        auto output = SR_UTILS_NS::VFS::Instance().OpenFile(temporary, SR_UTILS_NS::FileMode::Write);
        if (!output) {
            return false;
        }
        HINTERNET internet = InternetOpenW(L"SREngine", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
        if (!internet) {
            return false;
        }
        DWORD timeout = 15000;
        InternetSetOptionW(internet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionW(internet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
        HINTERNET response = InternetOpenUrlA(internet, request.url.c_str(), nullptr, 0,
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
        bool success = false;
        if (response) {
            DWORD code = 0;
            DWORD size = sizeof(code);
            if (HttpQueryInfoW(response, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &code, &size, nullptr) && code == 200) {
                char buffer[65536];
                char length[32] = {};
                DWORD lengthSize = sizeof(length);
                const uint64_t expected = HttpQueryInfoA(response, HTTP_QUERY_CONTENT_LENGTH, length, &lengthSize, nullptr)
                    ? strtoull(length, nullptr, 10) : 0;
                uint64_t total = 0;
                while (!request.isCancelled || !request.isCancelled()) {
                    DWORD received = 0;
                    if (!InternetReadFile(response, buffer, sizeof(buffer), &received)) {
                        break;
                    }
                    if (!received) {
                        success = true;
                        break;
                    }
                    if (output.Write(buffer, received) != received) {
                        break;
                    }
                    total += received;
                    if (request.progress) {
                        request.progress(total);
                    }
                    if (request.transferProgress) {
                        request.transferProgress(total, expected);
                    }
                }
            }
            InternetCloseHandle(response);
        }
        InternetCloseHandle(internet);
        return success;
    }
}
#endif
