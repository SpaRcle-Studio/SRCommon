//
// Created by Codex on 14.11.2026.
//

#ifndef SR_ENGINE_UTILS_PLATFORM_STDIO_TCP_BRIDGE_H
#define SR_ENGINE_UTILS_PLATFORM_STDIO_TCP_BRIDGE_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS::Platform {
    SR_COMMON_DLL_API int32_t RunStdioTCPBridge(uint16_t port);
}

#endif
