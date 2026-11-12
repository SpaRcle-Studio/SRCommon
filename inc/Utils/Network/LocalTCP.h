//
// Created by OpenAI Codex on 12.11.2026.
//

#ifndef SR_UTILS_NETWORK_LOCAL_TCP_H
#define SR_UTILS_NETWORK_LOCAL_TCP_H

#include <Utils/stdInclude.h>

namespace SR_NETWORK_NS::LocalTCP {
    using Handle = uintptr_t;
    constexpr Handle InvalidHandle = static_cast<Handle>(-1);

    Handle Listen(uint16_t& port);
    Handle Accept(Handle listener);
    int32_t Receive(Handle socket, char* buffer, uint32_t size);
    int32_t Send(Handle socket, const char* buffer, uint32_t size);
    void Close(Handle socket);
    void CloseListener(Handle socket);
}

#endif
