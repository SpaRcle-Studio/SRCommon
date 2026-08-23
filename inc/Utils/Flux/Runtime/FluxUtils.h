//
// Created by Monika on 23.08.2026.
//

#ifndef SR_ENGINE_FLUX_RUNTIME_FLUX_UTILS_H
#define SR_ENGINE_FLUX_RUNTIME_FLUX_UTILS_H

#include <Utils/ECS/Component.h>
#include <Utils/Common/Singleton.h>
#include <Utils/TypeTraits/SRClass.h>

namespace SR_FLUX_NS {
    /// @noCopyable @noMovable
    class FluxUtils : public Singleton<FluxUtils>, public SRClass {
        SR_REGISTER_SINGLETON(FluxUtils)
        SR_CLASS()
    public:
        void SetActiveFluxComponent(Component::Ptr pComponent);

        /// @method @evaluate
        SR_NODISCARD Component::Ptr GetActiveFluxComponent() const;

    private:
        Component::Ptr m_activeFluxComponent = nullptr;

    };
}

#endif //SR_ENGINE_FLUX_RUNTIME_FLUX_UTILS_H
