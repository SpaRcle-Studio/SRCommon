//
// Created by OpenAI Codex on 12.11.2026.
//

#ifndef SR_UTILS_NETWORK_LOCAL_TCP_H
#define SR_UTILS_NETWORK_LOCAL_TCP_H

#include <Utils/stdInclude.h>

namespace SR_NETWORK_NS::LocalTCP {
    using Handle = uintptr_t;
    constexpr Handle InvalidHandle = static_cast<Handle>(-1);

    SR_COMMON_DLL_API Handle Listen(uint16_t& port);
    SR_COMMON_DLL_API Handle Accept(Handle listener);
    /// Returns 1 when readable, 0 on timeout, -1 on failure.
    SR_COMMON_DLL_API int32_t WaitReadable(Handle socket, uint32_t milliseconds);
    SR_COMMON_DLL_API int32_t Receive(Handle socket, char* buffer, uint32_t size);
    SR_COMMON_DLL_API int32_t Send(Handle socket, const char* buffer, uint32_t size);
    SR_COMMON_DLL_API void Close(Handle socket);
    SR_COMMON_DLL_API void CloseListener(Handle socket);
}

#endif
