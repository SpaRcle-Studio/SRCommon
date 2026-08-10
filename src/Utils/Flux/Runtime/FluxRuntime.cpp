//
// Created by Monika on 08.08.2026.
//

#include <Utils/Flux/Runtime/FluxRuntime.h>
#include <Utils/Reflection/TypeInfoSerialization.h>

namespace SR_FLUX_NS {
    FluxRuntime::FluxRuntime(FluxProgram* pProgram)
        : m_program(pProgram)
    { }

    void FluxRuntime::Emit(StringView labelName, const Vector<Reflection::Value>& args) {
        SR_TRACY_ZONE;
        auto&& pIt = std::find_if(m_program->labels.begin(), m_program->labels.end(), [&](const FluxLabel& label) {
            return label.name == labelName;
        });
        if (pIt == m_program->labels.end()) {
            SR_ERROR("FluxRuntime::Emit() : label \"{}\" not found!", labelName);
            return;
        }
        if (m_executions.size() >= m_maxExecutions) {
            SR_ERROR("FluxRuntime::Emit() : max executions {} reached!", m_maxExecutions);
            return;
        }
        auto&& execution = m_executions.emplace_back();
        execution.instructionPointer = pIt->instructionPointer;
        execution.registers.reserve(m_maxRegisters);
        for (auto&& arg : args) {
            execution.valueStack.emplace_back(arg);
        }
    }

    void FluxRuntime::Update(float_t dt) {
        SR_TRACY_ZONE;

        if (!Initialize()) {
            return;
        }

        if (m_executions.empty()) {
            m_timeAccumulator = 0.f;
            return;
        }

        m_timeAccumulator += dt;

        const float_t floatingTicks = m_timeAccumulator / m_tickDuration;
        const auto ticks = static_cast<uint32_t>(floatingTicks);
        int64_t budget = m_budgetPerTick * ticks;
        m_timeAccumulator -= static_cast<float_t>(ticks) * m_tickDuration;

        uint32_t executionIndex = m_executions.size();
        bool hasAvailable = true;

        while (budget > 0) {
            const auto budgetPerExecution = SR_MAX(1, m_executions.size() / budget);

            if (++executionIndex >= m_executions.size()) {
                executionIndex = 0;
                if (!hasAvailable) {
                    break;
                }
                hasAvailable = false;
            }

            FluxExecution& execution = m_executions[executionIndex];
            if (!execution.CanBeExecuted()) {
                continue;
            }
            hasAvailable = true;

            budget -= Execute(execution, budgetPerExecution);
        }

        m_executions.erase_if([](const FluxExecution& execution) {
            return execution.IsFinished();
        });
    }

    uint32_t FluxRuntime::Execute(FluxExecution& execution, uint32_t budget) {
        const FluxProgram& program = *m_program;
        auto&& instructions = program.instructions;

        uint32_t usedBudget = 0;

        for (; usedBudget < budget && execution.instructionPointer < instructions.size(); ++usedBudget) {
            const FluxInstruction& instruction = instructions[execution.instructionPointer];
            if (!ExecuteInstruction(execution, instruction)) {
                break;
            }
            ++execution.instructionPointer;
        }

        return usedBudget;
    }

    bool FluxRuntime::ExecuteInstruction(FluxExecution& execution, const FluxInstruction& instruction) {
        SR_TRACY_ZONE;

        if (!ValidateInstruction(execution, instruction)) {
            return false;
        }

        switch (instruction.opcode) {
            case FluxOpcode::Copy:
                break;
            case FluxOpcode::Move:
            case FluxOpcode::Swap:
            case FluxOpcode::Ref:
            case FluxOpcode::Call:
            case FluxOpcode::Return:
            case FluxOpcode::Jump:
            case FluxOpcode::Branch:
            case FluxOpcode::Push:
            case FluxOpcode::Pop:
            default:
                SR_ERROR("FluxRuntime::ExecuteInstruction() : unhandled opcode!");
                execution.state = FluxExecutionState::Error;
                return false;
        }

        return true;
    }

    bool FluxRuntime::ValidateInstruction(FluxExecution& execution, const FluxInstruction& instruction) const {
        if (!m_validation) {
            return true;
        }

        SR_TRACY_ZONE;

        if (instruction.opcode == FluxOpcode::Unknown) {
            SR_ERROR("FluxRuntime::ValidateInstruction() : unknown opcode!");
            execution.state = FluxExecutionState::Error;
            return false;
        }

        if (instruction.opcode >= FluxOpcode::Copy && instruction.opcode <= FluxOpcode::Ref) {
            if (instruction.operands.size() != 2) {
                SR_ERROR("FluxRuntime::ValidateInstruction() : invalid number of operands for opcode {}!", static_cast<uint32_t>(instruction.opcode));
                execution.state = FluxExecutionState::Error;
                return false;
            }
        }

        if (instruction.opcode >= FluxOpcode::Push && instruction.opcode <= FluxOpcode::Pop || instruction.opcode == FluxOpcode::Branch) {
            if (instruction.operands.size() != 1) {
                SR_ERROR("FluxRuntime::ValidateInstruction() : invalid number of operands for opcode {}!", static_cast<uint32_t>(instruction.opcode));
                execution.state = FluxExecutionState::Error;
                return false;
            }
        }

        for (auto&& operand : instruction.operands) {
            if (GetRegisterType(execution, operand) == RegisterType::Invalid) {
                return false;
            }
        }

        return true;
    }

    RegisterType FluxRuntime::GetRegisterType(FluxExecution& execution, FluxRegisterId registerId) const {
        const auto constantCount = m_program->constants.size();
        const auto storageCount = m_program->storage.size();
        const auto registerCount = execution.registers.size();

        if (registerId < constantCount) {
            return RegisterType::Constant;
        }
        else if (registerId < constantCount + storageCount) {
            return RegisterType::Storage;
        }
        else if (registerId < constantCount + storageCount + registerCount) {
            if (registerId >= m_maxRegisters) {
                SR_ERROR("FluxRuntime::GetRegisterType() : register id {} exceeds max registers {}!", registerId, m_maxRegisters);
                execution.state = FluxExecutionState::Error;
                return RegisterType::Invalid;
            }
            return RegisterType::Register;
        }

        SR_ERROR("FluxRuntime::GetRegisterType() : invalid register id {}!", registerId);
        execution.state = FluxExecutionState::Error;
        return RegisterType::Invalid;
    }

    Reflection::Value& FluxRuntime::GetRegister(FluxExecution& execution, FluxRegisterId registerId, RegisterType type) {
        static Reflection::Value dummy;

        switch (type) {
            case RegisterType::Constant: {
                return m_constants[registerId];
            }
            case RegisterType::Storage:
                return m_storage[registerId - m_program->constants.size()];
            case RegisterType::Register: {
                const auto index = (registerId - m_constants.size()) - m_storage.size();
                if (index >= execution.registers.size()) {
                    execution.registers.resize(index + 1);
                }
                return execution.registers[index];
            }
            default:
                break;
        }

        SR_ERROR("FluxRuntime::GetRegister() : invalid register type!");
        execution.state = FluxExecutionState::Error;
        return dummy;
    }

    bool FluxRuntime::Initialize() {
        if (m_initialized) {
            return true;
        }
        m_constants.clear();
        m_storage.clear();

        for (auto&& constant : m_program->constants) {
            Reflection::TypeInfo* pTypeInfo = Reflection::LoadTypeInfo(constant.type);
            if (!pTypeInfo) {
                SR_ERROR("FluxRuntime::Initialize() : failed to load type info for constant \"{}\"!", constant.type);
                return false;
            }

            Reflection::FreeTypeInfo(pTypeInfo);
        }

        for (auto&& storage : m_program->storage) {
            Reflection::TypeInfo* pTypeInfo = Reflection::LoadTypeInfo(storage.type);
            if (!pTypeInfo) {
                SR_ERROR("FluxRuntime::Initialize() : failed to load type info for storage \"{}\"!", storage.type);
                return false;
            }

            Reflection::FreeTypeInfo(pTypeInfo);
        }

        m_initialized = true;
        return true;
    }
}
