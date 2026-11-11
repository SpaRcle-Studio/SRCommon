//
// Created by Monika on 11.09.2026.
//

#include <Utils/Network/IHTTPClient.h>
#include <Utils/Platform/Platform.h>

#include <emscripten/emscripten.h>

#ifdef DELETE
    #undef DELETE
#endif

// Asyncify preserves IHTTPClient's synchronous contract while fetch yields to the browser.
// Each request owns a reader; only one bounded chunk is copied into WASM at a time.
EM_ASYNC_JS(int32_t, SRWebHTTPBegin, (const char* methodPointer, const char* urlPointer,
    const char* headersPointer, const void* bodyPointer, int32_t bodySize, int32_t* status, int32_t* headerSize), {
    const controller = new AbortController();
    const timer = setTimeout(function() { controller.abort(); }, 30000);
    try {
        const headers = new Headers();
        for (const line of UTF8ToString(headersPointer).split('\r\n')) {
            const colon = line.indexOf(':');
            if (colon > 0) headers.append(line.slice(0, colon), line.slice(colon + 1));
        }
        const response = await fetch(UTF8ToString(urlPointer), {
            method: UTF8ToString(methodPointer), headers: headers,
            body: bodySize ? HEAPU8.slice(bodyPointer, bodyPointer + bodySize) : undefined,
            signal: controller.signal
        });
        let rawHeaders = '';
        response.headers.forEach(function(value, name) { rawHeaders += name + ': ' + value + '\r\n'; });
        const requests = Module['srHTTPRequests'] ||= new Map();
        const id = Module['srNextHTTPRequest'] = (Module['srNextHTTPRequest'] || 0) + 1;
        requests.set(id, {reader: response.body?.getReader(), headers: rawHeaders,
            chunk: null, offset: 0, controller: controller, timer: timer});
        HEAP32[status >> 2] = response.status;
        HEAP32[headerSize >> 2] = lengthBytesUTF8(rawHeaders);
        return id;
    } catch (error) {
        clearTimeout(timer);
        console.error('HTTP request failed', error);
        return 0;
    }
});

EM_JS(void, SRWebHTTPHeaders, (int32_t id, char* output, int32_t capacity), {
    const request = Module['srHTTPRequests']?.get(id);
    if (request) stringToUTF8(request.headers, output, capacity);
});

EM_ASYNC_JS(int32_t, SRWebHTTPRead, (int32_t id, void* output, int32_t capacity), {
    const request = Module['srHTTPRequests']?.get(id);
    if (!request) return -1;
    try {
        while (!request.chunk || request.offset === request.chunk.length) {
            if (!request.reader) return 0;
            const next = await request.reader.read();
            if (next.done) return 0;
            request.chunk = next.value; request.offset = 0;
        }
        const count = Math.min(capacity, request.chunk.length - request.offset);
        HEAPU8.set(request.chunk.subarray(request.offset, request.offset + count), output);
        request.offset += count;
        return count;
    } catch (error) { console.error('HTTP response read failed', error); return -1; }
});

EM_JS(void, SRWebHTTPEnd, (int32_t id), {
    const request = Module['srHTTPRequests']?.get(id);
    if (!request) return;
    clearTimeout(request.timer);
    request.reader?.cancel().catch(function() {});
    request.controller.abort();
    Module['srHTTPRequests'].delete(id);
});

EM_JS_DEPS(srengine_http, "$lengthBytesUTF8,$stringToUTF8,$UTF8ToString");

namespace SR_NETWORK_NS {
    class EmscriptenHTTPClient : public IHTTPClient {
    public:
        bool Send(const HTTPRequest& request, HTTPResponse& response, HTTPDataCallback onData) override {
            response = {};
            if (request.url.empty() || request.body.size() > static_cast<size_t>(SR_INT32_MAX)) { return false; }
            const char* method = nullptr;
            switch (request.method) {
                case HTTPMethod::GET: method = "GET"; break;
                case HTTPMethod::POST: method = "POST"; break;
                case HTTPMethod::PUT: method = "PUT"; break;
                case HTTPMethod::PATCH: method = "PATCH"; break;
                case HTTPMethod::DELETE: method = "DELETE"; break;
                default: return false;
            }
            String headers;
            for (const auto& [name, value] : request.headers) {
                if (name.empty() || name == "Host" || name == "Content-Length" || name == "User-Agent") { continue; }
                headers.append(name); headers.append(":"); headers.append(value); headers.append("\r\n");
            }
            int32_t headersSize = 0;
            const int32_t id = SRWebHTTPBegin(method, request.url.c_str(), headers.c_str(),
                request.body.data(), static_cast<int32_t>(request.body.size()), &response.statusCode, &headersSize);
            if (!id) { SR_ERROR("Browser HTTP failed (network, CORS or timeout): {}", request.url); return false; }
            String rawHeaders;
            rawHeaders.resize(static_cast<size_t>(headersSize) + 1);
            SRWebHTTPHeaders(id, rawHeaders.data(), headersSize + 1);
            StringView remaining(rawHeaders.data(), headersSize);
            while (!remaining.empty()) {
                const auto end = remaining.find('\n');
                auto line = remaining.substr(0, end);
                if (!line.empty() && line[line.size() - 1] == '\r') { line.remove_suffix(1); }
                const auto colon = line.find(':');
                if (colon != StringView::npos) {
                    auto value = line.substr(colon + 1);
                    while (!value.empty() && value[0] == ' ') { value.remove_prefix(1); }
                    response.headers.emplace_back(Pair<String, String>(String(line.substr(0, colon)), String(value)));
                }
                if (end == StringView::npos) { break; }
                remaining.remove_prefix(end + 1);
            }
            Vector<std::byte> chunk;
            chunk.resize(64 * 1024);
            bool success = true;
            for (;;) {
                const int32_t count = SRWebHTTPRead(id, chunk.data(), static_cast<int32_t>(chunk.size()));
                if (count <= 0) { success = count == 0; break; }
                if (onData && !onData({chunk.data(), static_cast<size_t>(count)})) { success = false; break; }
            }
            SRWebHTTPEnd(id);
            return success;
        }
    };

    IHTTPClient* CreateHTTPClient() { return new EmscriptenHTTPClient(); }
}
