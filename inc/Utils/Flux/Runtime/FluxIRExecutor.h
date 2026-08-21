//
// Created by Monika on 08.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_IR_EXECUTOR_H
#define SR_ENGINE_COMMON_FLUX_IR_EXECUTOR_H

#include <Utils/ECS/Component.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Resources/FileWatcher.h>

namespace SR_FLUX_NS {
    struct FluxProgram;
    class FluxRuntime;

    /// @category(Scripting)
    class FluxIRExecutor : public SR_UTILS_NS::Component {
        using Super = Component;
        SR_CLASS()

    public:
        void Awake() override;
        void Start() override;
        void OnAttached() override;
        void Update(float_t dt) override;

    private:
        void ReloadProgram();

    private:
        /// @property @onChanged(ReloadProgram)
        /// @customArgs(pick: enabled, filter name: Flux IR, relative: resources)
        /// @customArg(filter value: fluxir)
        Path m_programPath;

        FileWatcher::Ptr m_programWatcher;
        RawPointerHolder<FluxProgram> m_program;
        RawPointerHolder<FluxRuntime> m_runtime;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_IR_EXECUTOR_H
