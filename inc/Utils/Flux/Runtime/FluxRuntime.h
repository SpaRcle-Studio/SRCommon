//
// Created by Monika on 08.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_RUNTIME_H
#define SR_ENGINE_COMMON_FLUX_RUNTIME_H

#include <Utils/Flux/Runtime/FluxExecution.h>

namespace SR_FLUX_NS {
    enum class RegisterType : uint8_t {
        Invalid,
        Constant,
        Storage,
        Register,
    };

    enum class RegisterOperation : uint8_t {
        Read,
        Write,
    };

    class FluxRuntime : public NonCopyable {
    public:
        FluxRuntime() = default;
        explicit FluxRuntime(const FluxProgram* pProgram);
        ~FluxRuntime() override;

        void Emit(StringView labelName, const Vector<Reflection::Value>& args, bool ignoreExisting = true);
        void Update(float_t dt);
        void SetStorage(uint32_t index, const Reflection::Value& value);
        bool Initialize();

        SR_NODISCARD bool IsEmitted(StringView labelName) const;

    private:
        uint32_t Execute(FluxExecution& execution, uint32_t budget);
        bool ExecuteInstruction(FluxExecution& execution, const FluxInstruction& instruction);
        bool ValidateInstruction(FluxExecution& execution, const FluxInstruction& instruction) const;
        bool CallMethod(FluxExecution& execution, const FluxInstruction& instruction);
        SR_NODISCARD RegisterType GetRegisterType(FluxExecution& execution, FluxRegisterId registerId) const;
        SR_NODISCARD Reflection::Value& GetRegister(FluxExecution& execution, FluxRegisterId registerId, RegisterType type, RegisterOperation operation);
        SR_NODISCARD Reflection::Value& GetResultRegister(FluxExecution& execution);
        SR_NODISCARD Reflection::Value LoadFluxVariable(const FluxVariable& variable);

    public:
        uint32_t m_maxRegisters = 16;
        uint32_t m_maxExecutions = 256;
        uint32_t m_budgetPerTick = 1024;
        float_t m_tickDuration = 0.016f; // 1 / 60 FPS

        float_t m_timeAccumulator = 0.0f;

        Vector<FluxExecution> m_executions;
        Vector<Reflection::Value*> m_callArguments;

        bool m_initialized = false;
        bool m_validation = false;
        bool m_continueOnError = false;
        bool m_hasErrors = false;

        const FluxProgram* m_program = nullptr;

        Vector<Reflection::Value> m_constants;
        Vector<Reflection::Value> m_storage;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_RUNTIME_H
