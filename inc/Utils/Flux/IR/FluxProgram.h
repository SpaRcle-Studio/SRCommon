//
// Created by Monika on 08.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_PROGRAM_H
#define SR_ENGINE_COMMON_FLUX_PROGRAM_H

#include <Utils/Flux/IR/FluxInstruction.h>

namespace SR_FLUX_NS {
    struct FluxFunction {
        StringAtom name;
        Vector<FluxInstruction> instructions;
    };

    struct FluxVariable {
        String type;
        String value;
    };

    struct FluxProgram {
        SR_HTYPES_NS::RawPointerHolder<IAllocator> allocator;
        Vector<FluxFunction> functions;
        Vector<FluxVariable> constants;
        Vector<FluxVariable> storage;
    };
}

#endif //SR_ENGINE_COMMON_FLUX_PROGRAM_H
