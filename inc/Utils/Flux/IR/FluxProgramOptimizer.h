//
// Created by Monika on 03.09.2026.
//

#ifndef SR_ENGINE_FLUX_PROGRAM_OPTIMIZER_H
#define SR_ENGINE_FLUX_PROGRAM_OPTIMIZER_H

#include <Utils/stdInclude.h>

namespace SR_FLUX_NS {
    struct FluxProgram;

    /// return true if program was optimized, false if no optimizations were applied
    extern SR_COMMON_DLL_API bool OptimizeProgram(FluxProgram& program);
}

#endif //SR_ENGINE_FLUX_PROGRAM_OPTIMIZER_H
