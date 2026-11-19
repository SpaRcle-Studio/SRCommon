//
// Created by Monika on 23.08.2026.
//

#include <Utils/Flux/Runtime/FluxUtils.h>

#include <Codegen/FluxUtils.generated.hpp>

namespace SR_FLUX_NS {
    SR_MAYBE_UNUSED auto&& fluxUtilsInitInstance = FluxUtils::Instance();

    void FluxUtils::SetActiveFluxComponent(FluxComponent::Ptr pComponent) {
        m_activeFluxComponent = std::move(pComponent);
    }

    FluxComponent::Ptr FluxUtils::GetActiveFluxComponent() const {
        return m_activeFluxComponent;
    }

    Reflection::Value FluxUtils::Cast(const Reflection::Value& value, StringAtom target) const {
        if (!value.IsValid()) {
            return Reflection::Value();
        }
        auto&& pTargetMeta = Factory::Instance().GetType(target);
        return pTargetMeta ? pTargetMeta->Cast(value) : Reflection::Value();
    }

    uint64_t FluxUtils::GetContainerSize(const Reflection::Value& value) const {
        if (const auto array = value.AsArray()) {
            return array.Size();
        }
        if (const auto map = value.AsMap()) {
            return map.Size();
        }
        return 0;
    }

    Reflection::Value FluxUtils::VectorAt(const Reflection::Value& vector, uint64_t index) const {
        const auto array = vector.AsArray();
        if (!array || index >= array.Size()) {
            SR_WARN("FluxUtils::VectorAt() : index {} is out of range or value is not a vector!", index);
            return Reflection::Value();
        }
        return array.At(index).ToValue();
    }
}