//
// Created by Monika on 08.08.2026.
//

#include <Utils/Flux/Runtime/FluxRuntime.h>
#include <Utils/Reflection/TypeInfoSerialization.h>
#include <Utils/Reflection/Method.h>
#include <Utils/Serialization/JsonSerialization.h>

namespace SR_FLUX_NS {
    FluxRuntime::FluxRuntime(const FluxProgram* pProgram)
        : m_program(pProgram)
    {
        m_maxRegisters = m_program->requiredRegisters;
    }

    FluxRuntime::~FluxRuntime() {
        m_constants.clear();
        m_storage.clear();
        m_executions.clear();
        m_callArguments.clear();
    }

    void FluxRuntime::Emit(StringView labelName, const Vector<Reflection::Value>& args, bool ignoreExisting) {
        SR_TRACY_ZONE;
        if (m_hasErrors) {
            return;
        }
        auto&& pIt = std::find_if(m_program->labels.begin(), m_program->labels.end(), [&](const FluxLabel& label) {
            return label.name == labelName;
        });
        if (pIt == m_program->labels.end()) {
            if (!ignoreExisting) {
                SR_ERROR("FluxRuntime::Emit() : label \"{}\" not found!", labelName);
            }
            return;
        }
        if (m_executions.size() >= m_maxExecutions) {
            SR_ERROR("FluxRuntime::Emit() : max executions {} reached!", m_maxExecutions);
            return;
        }
        auto&& execution = m_executions.emplace_back();
        execution.instructionPointer = pIt->instructionPointer;
        execution.registers.resize(m_maxRegisters);
        execution.emittedLabel = pIt->name;
        for (auto&& arg : args) {
            execution.valueStack.emplace_back(arg);
        }
    }

    void FluxRuntime::Update(float_t dt) {
        SR_TRACY_ZONE;

        if (m_hasErrors || !Initialize()) {
            m_hasErrors = true;
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

            if (execution.state == FluxExecutionState::Error && !m_continueOnError) {
                m_hasErrors = true;
                break;
            }
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

        if (!ValidateInstruction(execution, instruction)) SR_UNLIKELY_ATTRIBUTE {
            return false;
        }

        switch (instruction.opcode) {
            case FluxOpcode::Copy: {
                auto&& srcType = GetRegisterType(execution, instruction.operands[0]);
                auto&& dstType = GetRegisterType(execution, instruction.operands[1]);
                auto&& src = GetRegister(execution, instruction.operands[0], srcType, RegisterOperation::Read);
                auto&& dst = GetRegister(execution, instruction.operands[1], dstType, RegisterOperation::Write);
                dst = src.Copy();
                break;
            }
            case FluxOpcode::Move: {
                auto&& srcType = GetRegisterType(execution, instruction.operands[0]);
                auto&& dstType = GetRegisterType(execution, instruction.operands[1]);
                auto&& src = GetRegister(execution, instruction.operands[0], srcType, RegisterOperation::Write);
                auto&& dst = GetRegister(execution, instruction.operands[1], dstType, RegisterOperation::Write);
                dst = std::move(src);
                break;
            }
            case FluxOpcode::Swap: {
                auto&& srcType = GetRegisterType(execution, instruction.operands[0]);
                auto&& dstType = GetRegisterType(execution, instruction.operands[1]);
                auto&& src = GetRegister(execution, instruction.operands[0], srcType, RegisterOperation::Write);
                auto&& dst = GetRegister(execution, instruction.operands[1], dstType, RegisterOperation::Write);
                std::swap(src, dst);
                break;
            }
            case FluxOpcode::Ref: {
                auto&& srcType = GetRegisterType(execution, instruction.operands[0]);
                auto&& dstType = GetRegisterType(execution, instruction.operands[1]);
                auto&& src = GetRegister(execution, instruction.operands[0], srcType, RegisterOperation::Read);
                auto&& dst = GetRegister(execution, instruction.operands[1], dstType, RegisterOperation::Write);
                dst = src.Ref();
                break;
            }
            case FluxOpcode::Return: {
                if (execution.callStack.empty()) {
                    execution.state = FluxExecutionState::Finished;
                    return false;
                }
                execution.instructionPointer = execution.callStack.back() - 1; // -1 because we will increment it after this instruction
                execution.callStack.pop_back();
                break;
            }
            case FluxOpcode::Push: {
                auto&& srcType = GetRegisterType(execution, instruction.operands[0]);
                auto&& src = GetRegister(execution, instruction.operands[0], srcType, RegisterOperation::Read);
                execution.valueStack.emplace_back(src.Copy());
                break;
            }
            case FluxOpcode::Pop: {
                if (m_validation && execution.valueStack.empty()) SR_UNLIKELY_ATTRIBUTE {
                    SR_ERROR("FluxRuntime::ExecuteInstruction() : value stack is empty!");
                    execution.state = FluxExecutionState::Error;
                    return false;
                }
                auto&& dstType = GetRegisterType(execution, instruction.operands[0]);
                auto&& dst = GetRegister(execution, instruction.operands[0], dstType, RegisterOperation::Write);
                dst = std::move(execution.valueStack.back());
                execution.valueStack.pop_back();
                break;
            }
            case FluxOpcode::Jump: {
                auto&& labelId = instruction.operands[0];
                execution.instructionPointer = m_program->labels[labelId].instructionPointer - 1;
                break;
            }
            case FluxOpcode::Branch: {
                auto&& labelId = instruction.operands[0];
                auto&& conditionValue = GetResultRegister(execution);
                auto&& type = conditionValue.GetTypeInfo();
                if (m_validation) SR_UNLIKELY_ATTRIBUTE {
                    if (type.category != Reflection::ReflectedCategoryType::Arithmetic || type.detailedType != "bool") {
                        SR_ERROR("FluxRuntime::ExecuteInstruction() : branch condition must be a boolean type!");
                        if (!m_continueOnError) {
                            execution.state = FluxExecutionState::Error;
                            return false;
                        }
                        return true;
                    }
                }
                const bool condition = *conditionValue.Cast<bool>();
                if (condition) {
                    execution.instructionPointer = m_program->labels[labelId].instructionPointer - 1;
                }
                break;
            }
            case FluxOpcode::Call:
                if (!CallMethod(execution, instruction)) {
                    return false;
                }
                break;
            default:
                SR_ERROR("FluxRuntime::ExecuteInstruction() : unhandled opcode!");
                execution.state = FluxExecutionState::Error;
                return false;
        }

        return true;
    }

    bool FluxRuntime::ValidateInstruction(FluxExecution& execution, const FluxInstruction& instruction) const {
        if (m_validation) {
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
        else if (instruction.opcode == FluxOpcode::Jump || instruction.opcode == FluxOpcode::Branch) {
            if (instruction.operands.size() != 1) {
                SR_ERROR("FluxRuntime::ValidateInstruction() : invalid number of operands for opcode {}!", static_cast<uint32_t>(instruction.opcode));
                execution.state = FluxExecutionState::Error;
                return false;
            }
            auto&& labelId = instruction.operands[0];
            if (labelId >= m_program->labels.size()) {
                SR_ERROR("FluxRuntime::ValidateInstruction() : invalid label id {}!", labelId);
                execution.state = FluxExecutionState::Error;
                return false;
            }
        }
        else if (instruction.opcode >= FluxOpcode::Push && instruction.opcode <= FluxOpcode::Pop || instruction.opcode == FluxOpcode::Branch) {
            if (instruction.operands.size() != 1) {
                SR_ERROR("FluxRuntime::ValidateInstruction() : invalid number of operands for opcode {}!", static_cast<uint32_t>(instruction.opcode));
                execution.state = FluxExecutionState::Error;
                return false;
            }
        }

        if (instruction.opcode != FluxOpcode::Jump && instruction.opcode != FluxOpcode::Branch) {
            for (auto&& operand : instruction.operands) {
                if (GetRegisterType(execution, operand) == RegisterType::Invalid) {
                    return false;
                }
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

    Reflection::Value& FluxRuntime::GetRegister(FluxExecution& execution, FluxRegisterId registerId, RegisterType type, RegisterOperation operation) {
        static Reflection::Value dummy;

        switch (type) {
            case RegisterType::Constant: {
                if (operation == RegisterOperation::Write) {
                    SR_ERROR("FluxRuntime::GetRegister() : cannot write to constant register {}!", registerId);
                    execution.state = FluxExecutionState::Error;
                    return dummy;
                }
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

        SR_TRACY_ZONE;

        m_constants.clear();
        m_storage.clear();
        m_constants.resize(m_program->constants.size());
        m_storage.resize(m_program->storage.size());

        for (SizeType i = 0; i < m_program->constants.size(); ++i) {
            if (m_constants[i] = LoadFluxVariable(m_program->constants[i]); !m_constants[i].IsValid()) {
                SR_ERROR("FluxRuntime::Initialize() : failed to load constant variable {}!", m_program->constants[i].type);
                return false;
            }
        }

        for (SizeType i = 0; i < m_program->storage.size(); ++i) {
            if (m_storage[i] = LoadFluxVariable(m_program->storage[i]); !m_storage[i].IsValid()) {
                SR_ERROR("FluxRuntime::Initialize() : failed to load storage variable {}!", m_program->storage[i].type);
                return false;
            }
        }

        m_initialized = true;
        return true;
    }

    Reflection::Value& FluxRuntime::GetResultRegister(FluxExecution& execution) {
        if (execution.registers.empty()) {
            SR_ERROR("FluxRuntime::GetResultRegister() : no registers available!");
            execution.state = FluxExecutionState::Error;
            static Reflection::Value dummy;
            return dummy;
        }
        return execution.registers.front();
    }

    bool FluxRuntime::CallMethod(FluxExecution& execution, const FluxInstruction& instruction) {
        SR_TRACY_ZONE;

        m_callArguments.clear();

        SRClass* pCallable = SR_UTILS_NS::GetSingletonManager()->GetSingletonMeta(instruction.callable.object.GetHash());
        const bool isSingleton = pCallable;

        for (auto&& operand : instruction.operands | std::views::drop(isSingleton ? 0 : 1)) {
            auto&& argType = GetRegisterType(execution, operand);
            auto&& pArgValue = GetRegister(execution, operand, argType, RegisterOperation::Read);
            m_callArguments.emplace_back(&pArgValue);
        }

        if (!pCallable) {
            if (instruction.operands.empty()) SR_UNLIKELY_ATTRIBUTE {
                SR_ERROR("FluxRuntime::CallMethod() : no callable provided!");
                execution.state = FluxExecutionState::Error;
                return false;
            }

            auto&& callableType = GetRegisterType(execution, instruction.operands[0]);
            Reflection::Value& callable = GetRegister(execution, instruction.operands[0], callableType, RegisterOperation::Read);

            auto&& typeInfo = callable.GetTypeInfo();
            if (typeInfo.category == Reflection::ReflectedCategoryType::Object) {
                pCallable = (SRClass*)typeInfo.vtable.pGetTypeController(callable.GetStorage());
            }
            else if (typeInfo.category == Reflection::ReflectedCategoryType::Container && typeInfo.detailedType == "SharedPtr") {
                auto&& pBase = (SR_HTYPES_NS::SharedPtrBase*)typeInfo.vtable.pGetTypeController(callable.GetStorage());
                pCallable = pBase->GetSRClass();
                if (!pCallable) {
                    SR_ERROR("FluxRuntime::CallMethod() : failed to get callable type from SharedPtr!");
                    execution.state = FluxExecutionState::Error;
                    return false;
                }
            }
        }

        if (!pCallable) {
            SR_ERROR("FluxRuntime::CallMethod() : failed to get callable type!");
            execution.state = FluxExecutionState::Error;
            return false;
        }

        auto&& pFunction = pCallable->GetMeta()->FindMethod(instruction.callable.function);
        if (!pFunction) {
            SR_ERROR("FluxRuntime::CallMethod() : method \"{}\" not found in class \"{}\"!", instruction.callable.function, pCallable->GetMeta()->GetFactoryName());
            execution.state = FluxExecutionState::Error;
            return false;
        }

        const bool hasReturn = pFunction->HasReturn();
        const auto paramsCount = pFunction->GetParamsCount();

        if (m_callArguments.size() != paramsCount) {
            SR_ERROR("FluxRuntime::CallMethod() : method \"{}\" expects {} arguments, but {} were provided!", instruction.callable.function, paramsCount, m_callArguments.size());
            execution.state = FluxExecutionState::Error;
            return false;
        }

        if (hasReturn) {
            GetResultRegister(execution) = paramsCount > 0 ?
                pFunction->Invoke(*pCallable, m_callArguments) :
                pFunction->Invoke(*pCallable);
        }
        else {
            paramsCount > 0 ?
                pFunction->InvokeVoid(*pCallable, m_callArguments) :
                pFunction->InvokeVoid(*pCallable);
        }

        return true;
    }

    Reflection::Value FluxRuntime::LoadFluxVariable(const FluxVariable& variable) {
        Reflection::TypeInfo* pTypeInfo = Reflection::LoadTypeInfo(variable.type);
        if (!pTypeInfo || !Reflection::FindVTable(*pTypeInfo)) {
            SR_ERROR("FluxRuntime::LoadFluxVariable() : failed to load type info for \"{}\"!", variable.type);
            Reflection::FreeTypeInfo(pTypeInfo);
            return Reflection::Value();
        }
        Reflection::Value value = Reflection::Value::CreateDefault(pTypeInfo);
        Reflection::FreeTypeInfo(pTypeInfo);
        if (!variable.value.empty()) {
            JsonDeserializer deserializer;
            if (!deserializer.LoadFromStringView(variable.value)) {
                SR_ERROR("FluxRuntime::LoadFluxVariable() : failed to load value for \"{}\"! Value: {}", variable.type, variable.value);
                return Reflection::Value();
            }
            if (!Reflection::DeserializeValue(value, deserializer)) {
                SR_ERROR("FluxRuntime::LoadFluxVariable() : failed to deserialize value for \"{}\"! Value: {}", variable.type, variable.value);
                return Reflection::Value();
            }
        }
        return value;
    }

    void FluxRuntime::SetStorage(uint32_t index, const Reflection::Value& value) {
        if (index >= m_storage.size()) {
            SR_ERROR("FluxRuntime::SetStorage() : index {} out of bounds!", index);
            return;
        }
        m_storage[index] = value.Copy();
    }

    bool FluxRuntime::IsEmitted(StringView labelName) const {
        auto&& pIt = m_executions.find_if([labelName](const FluxExecution& execution) {
            return execution.emittedLabel == labelName;
        });
        return pIt != m_executions.end();
    }
}
