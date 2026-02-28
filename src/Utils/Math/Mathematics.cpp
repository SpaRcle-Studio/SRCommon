//
// Created by Monika on 02.01.2026.
//

#include <Utils/Math/Mathematics.h>

namespace SR_MATH_NS {
    bool HasSSE41() {
#ifdef SR_EMSCRIPTEN
        return false;
#elif defined(__GNUC__) || defined(__clang__)
        return __builtin_cpu_supports("sse4.1");
#else
        int cpuInfo[4];
        __cpuid(cpuInfo, 1);
        return (cpuInfo[2] & (1 << 19)) != 0;
#endif
    }
}