//
// Created by Monika on 08.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_PROGRAM_H
#define SR_ENGINE_COMMON_FLUX_PROGRAM_H

#include <Utils/Flux/IR/FluxInstruction.h>

namespace SR_FLUX_NS {
    struct FluxVariable {
        String type;
        String value;
    };

    struct FluxLabel {
        String name;
        uint32_t instructionPointer = 0;
    };

    struct FluxProgram {
        RawPointerHolder<IAllocator> allocator;
        Vector<FluxInstruction> instructions;
        Vector<FluxVariable> constants;
        Vector<FluxVariable> storage;
        Vector<FluxLabel> labels;
        uint32_t requiredRegisters = 0;

        void SaveToString(String& out) const;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_PROGRAM_H
