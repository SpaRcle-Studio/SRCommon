//
// Created by Monika on 08.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_IR_EXECUTOR_H
#define SR_ENGINE_COMMON_FLUX_IR_EXECUTOR_H

#include <Utils/ECS/Component.h>
#include <Utils/FileSystem/Path.h>

namespace SR_FLUX_NS {
    /// @category(Scripting)
    class FluxIRExecutor : public SR_UTILS_NS::Component {
        using Super = Component;
        SR_CLASS()

    public:
        void OnAttached() override;

    private:
        /// @property
        /// @customArgs(pick: enabled, filter name: Flux IR, relative: resources)
        /// @customArg(filter value: fluxir)
        SR_UTILS_NS::Path m_programPath;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_IR_EXECUTOR_H
