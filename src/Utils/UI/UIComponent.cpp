//
// Created by Codex on 15.11.2026.
//

#include <Utils/UI/UIComponent.h>
#include <Utils/TypeTraits/SRClassMeta.h>
#include <Utils/World/Scene.h>
#include <Utils/World/SceneUpdater.h>
#include <Codegen/UIComponent.generated.hpp>

namespace SR_UTILS_NS {
    void UIComponent::OnPostLoad() {
        Component::OnPostLoad();
        if (auto&& pScene = TryGetScene()) {
            pScene->GetSceneUpdater()->InvalidateUIInput(true);
        }
    }

    void UIComponent::InvalidateUIInput(bool targetsDirty) {
        if (auto&& pScene = TryGetScene()) {
            pScene->GetSceneUpdater()->InvalidateUIInput(targetsDirty);
        }
    }

    void UIComponent::OnMatrixDirty() {
        Component::OnMatrixDirty();
        InvalidateUIInput();
    }

    void UIComponent::OnMaskDirty() {
        Component::OnMaskDirty();
        InvalidateUIInput();
    }

    void UIComponent::OnPriorityChanged() {
        Component::OnPriorityChanged();
        if (auto&& pScene = TryGetScene()) {
            pScene->GetSceneUpdater()->InvalidateUIInput(true);
        }
    }

    UIComponent* UIComponent::TryGet(Component* pComponent) noexcept {
        return pComponent && pComponent->GetMeta()->IsSameOrInherited(GetClassStaticName())
            ? static_cast<UIComponent*>(pComponent) : nullptr;
    }

    void UIComponent::PrepareUIInput() {
    }

    void UIComponent::AfterLayout() {
    }
}
