//
// Created by Monika on 02.01.2026.
//

#include <Utils/Math/Mathematics.h>

namespace SR_MATH_NS {
    bool HasSSE41() {
        int cpuInfo[4];
        __cpuid(cpuInfo, 1);
        return (cpuInfo[2] & (1 << 19)) != 0;
    }
}