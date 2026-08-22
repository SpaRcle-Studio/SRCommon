//
// Created by Monika on 08.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_INSTRUCTION_H
#define SR_ENGINE_COMMON_FLUX_INSTRUCTION_H

#include <Utils/Flux/IR/FluxOpcode.h>

namespace SR_UTILS_NS::Reflection {
    class Method;
}

namespace SR_FLUX_NS {
    struct FluxCallable {
        StringAtom object;
        StringAtom function;

        SR_NODISCARD const Reflection::Method* FindMethodMeta() const;
    };

    using FluxRegisterId = uint32_t;
    using FluxInstructionId = uint32_t;

    struct FluxInstruction {
        FluxOpcode opcode = FluxOpcode::Unknown;
        Vector<FluxRegisterId> operands;
        FluxCallable callable;
        uint32_t debugId = 0;
    };
}

#endif //SR_ENGINE_COMMON_FLUX_INSTRUCTION_H
