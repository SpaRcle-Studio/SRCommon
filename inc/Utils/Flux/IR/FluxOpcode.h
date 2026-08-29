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

        /// приведение типа: dst = FluxUtils::Cast(src, callable.object).
        /// Успешность приведения кладётся в результирующий регистр, поэтому за инструкцией
        /// может следовать br
        Cast,

        Call,

        Push,
        Pop,

        Jump,
        Branch,

        /// ветвление исполнения: каждый операнд является меткой, с которой начнётся новое
        /// исполнение с копией текущего состояния. Родитель продолжает со следующей инструкции
        Fork,

        Return,
    };

    struct OpcodeMapping {
        StringView name;
        FluxOpcode opcode;
    };

    static constexpr OpcodeMapping OPCODE_MAPPINGS[] = {
        { "cp", FluxOpcode::Copy },
        { "mv", FluxOpcode::Move },
        { "swap", FluxOpcode::Swap },
        { "ref", FluxOpcode::Ref },
        { "cast", FluxOpcode::Cast },
        { "call", FluxOpcode::Call },
        { "ret", FluxOpcode::Return },
        { "jmp", FluxOpcode::Jump },
        { "br", FluxOpcode::Branch },
        { "fork", FluxOpcode::Fork },
        { "push", FluxOpcode::Push },
        { "pop", FluxOpcode::Pop },
    };
}

#endif //SR_ENGINE_COMMON_FLUX_OPCODE_H
