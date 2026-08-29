//
// Created by Monika on 09.08.2026.
//

#include <Utils/Flux/Runtime/FluxExecution.h>

namespace SR_FLUX_NS {
    FluxExecution::FluxExecution(FluxExecution&& other) noexcept
        : registers(std::move(other.registers))
        , valueStack(std::move(other.valueStack))
        , callStack(std::move(other.callStack))
        , instructionPointer(other.instructionPointer)
        , state(other.state)
    {
        other.instructionPointer = 0;
    }

    FluxExecution& FluxExecution::operator=(FluxExecution&& other) noexcept {
        if (this != &other) {
            registers = std::move(other.registers);
            valueStack = std::move(other.valueStack);
            callStack = std::move(other.callStack);

            instructionPointer = other.instructionPointer;
            other.instructionPointer = 0;

            state = other.state;
            other.state = FluxExecutionState::None;
        }
        return *this;
    }

    FluxExecution FluxExecution::Fork() const {
        FluxExecution child;

        /// значения копируются так же, как при передаче аргументов события: собственные данные
        /// дублируются, а ссылки на внешние объекты остаются ссылками и продолжают указывать
        /// на тот же объект
        child.registers.reserve(registers.size());
        for (auto&& value : registers) {
            child.registers.emplace_back(value);
        }

        child.valueStack.reserve(valueStack.size());
        for (auto&& value : valueStack) {
            child.valueStack.emplace_back(value);
        }

        child.callStack.reserve(callStack.size());
        for (auto&& pointer : callStack) {
            child.callStack.emplace_back(pointer);
        }

        child.emittedLabel = emittedLabel;
        child.updateMode = updateMode;

        return child;
    }

    bool FluxExecution::IsFinished() const {
        return state == FluxExecutionState::Finished || state == FluxExecutionState::Error;
    }

    bool FluxExecution::CanBeExecuted() const {
        return !IsFinished();
    }
}