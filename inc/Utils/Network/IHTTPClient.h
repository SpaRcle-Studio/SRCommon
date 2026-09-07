//
// Created by Monika on 07.09.2026.
//

#ifndef SR_ENGINE_NETWORKING_I_HTTP_CLIENT_H
#define SR_ENGINE_NETWORKING_I_HTTP_CLIENT_H

#include <Utils/Common/NonCopyable.h>

#ifdef DELETE
    #undef DELETE
#endif

namespace SR_UTILS_NS {
    class IAllocator;
}

namespace SR_NETWORK_NS {
    enum class HTTPMethod {
        GET,
        POST,
        PUT,
        PATCH,
        DELETE
    };

    struct HTTPRequest {
        HTTPMethod method = HTTPMethod::GET;
        String url;
        Vector<Pair<String, String>> headers;
        std::span<const std::byte> body;
    };

    struct HTTPResponse {
        int32_t statusCode = 0;
        Vector<Pair<String, String>> headers;
    };

    using HTTPDataCallback = SR_HTYPES_NS::Function<bool(std::span<const std::byte>)>;

    class IHTTPClient : public NonCopyable {
    public:
        ~IHTTPClient() override = default;

    public:
        virtual bool Send(const HTTPRequest& request, HTTPResponse& response, HTTPDataCallback onData) = 0;

    };

    extern SR_COMMON_DLL_API IHTTPClient* CreateHTTPClient();
}

#endif //SR_ENGINE_NETWORKING_I_HTTP_CLIENT_H
