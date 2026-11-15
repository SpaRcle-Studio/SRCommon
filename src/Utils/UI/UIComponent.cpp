//
// Created by Codex on 15.11.2026.
//

#include <Utils/UI/UIComponent.h>
#include <Utils/TypeTraits/SRClassMeta.h>
#include <Codegen/UIComponent.generated.hpp>

namespace SR_UTILS_NS {
    UIComponent* UIComponent::TryGet(Component* pComponent) noexcept {
        return pComponent && pComponent->GetMeta()->IsSameOrInherited(GetClassStaticName())
            ? static_cast<UIComponent*>(pComponent) : nullptr;
    }

    void UIComponent::PrepareUIInput() {
    }

    void UIComponent::AfterLayout() {
    }
}
