//
// Created by Monika on 08.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_INSTRUCTION_H
#define SR_ENGINE_COMMON_FLUX_INSTRUCTION_H

#include <Utils/Flux/IR/FluxOpcode.h>

namespace SR_FLUX_NS {
    struct FluxCallable {
        StringAtom object;
        StringAtom function;
    };

    using FluxRegisterId = uint32_t;
    using FluxInstructionId = uint32_t;

    struct FluxInstruction {
        FluxOpcode opcode = FluxOpcode::Unknown;
        Vector<FluxRegisterId> operands;
        FluxCallable callable;
    };
}

#endif //SR_ENGINE_COMMON_FLUX_INSTRUCTION_H
