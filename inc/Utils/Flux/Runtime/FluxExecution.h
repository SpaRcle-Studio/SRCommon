//
// Created by Monika on 09.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_EXECUTION_H
#define SR_ENGINE_COMMON_FLUX_EXECUTION_H

#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Reflection/Value.h>

namespace SR_FLUX_NS {
    enum class FluxExecutionState : uint8_t {
        None,
        Running,
        Paused,
        Finished,
        Error
    };


    SR_ENUM_NS_CLASS_T(UpdateMode, uint8_t,
        Any,
        Update,
        FixedUpdate
    );

    struct FluxExecution {
        FluxExecution() = default;
        FluxExecution(FluxExecution&& other) noexcept;
        FluxExecution(const FluxExecution& other) = delete;
        FluxExecution& operator=(FluxExecution&& other) noexcept;
        FluxExecution& operator=(const FluxExecution& other) = delete;

        Vector<Reflection::Value> registers;
        Vector<Reflection::Value> valueStack;
        Vector<FluxInstructionId> callStack;
        FluxInstructionId instructionPointer = 0;
        FluxExecutionState state = FluxExecutionState::None;
        StringView emittedLabel;

        SR_NODISCARD bool IsFinished() const;
        SR_NODISCARD bool CanBeExecuted() const;

        /// Копия состояния для инструкции fork. Указатель инструкции проставляет вызывающая
        /// сторона - потомок начинает с метки своей ветви
        SR_NODISCARD FluxExecution Fork() const;

        UpdateMode updateMode = UpdateMode::Any;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_EXECUTION_H
