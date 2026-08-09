//
// Created by Monika on 08.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_OPCODE_H
#define SR_ENGINE_COMMON_FLUX_OPCODE_H

#include <Utils/stdInclude.h>

namespace SR_FLUX_NS {
    enum class FluxOpcode : uint8_t {
        Unknown,

        Copy,
        Move,
        Swap,
        Ref,

        Call,

        Push,
        Pop,

        Jump,
        Branch,

        Return,
    };
}

#endif //SR_ENGINE_COMMON_FLUX_OPCODE_H
