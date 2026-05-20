//
// Created by Monika on 16.05.2026.
//

#ifndef SR_ENGINE_COMMON_WINDOWS_MEMORY_ALLOCATION_HOOKS_H
#define SR_ENGINE_COMMON_WINDOWS_MEMORY_ALLOCATION_HOOKS_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    SR_COMMON_DLL_API extern void InitMemoryHooks();
    SR_COMMON_DLL_API extern void DeInitMemoryHooks();
}

#endif //SR_ENGINE_COMMON_WINDOWS_MEMORY_ALLOCATION_HOOKS_H
