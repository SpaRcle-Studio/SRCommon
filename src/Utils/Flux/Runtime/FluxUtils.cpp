//
// Created by Monika on 23.08.2026.
//

#include <Utils/Flux/Runtime/FluxUtils.h>

#include <Codegen/FluxUtils.generated.hpp>

namespace SR_FLUX_NS {
    SR_MAYBE_UNUSED auto&& fluxUtilsInitInstance = FluxUtils::Instance();

    void FluxUtils::SetActiveFluxComponent(Component::Ptr pComponent) {
        m_activeFluxComponent = std::move(pComponent);
    }

    Component::Ptr FluxUtils::GetActiveFluxComponent() const {
        return m_activeFluxComponent;
    }
}