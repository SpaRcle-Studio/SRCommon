//
// Created by Codex on 06.11.2026.
//

#include <Utils/UI/LayoutElement.h>
#include <Utils/UI/LayoutGroup.h>
#include <Utils/ECS/GameObject.h>

#include <Codegen/LayoutElement.generated.hpp>

namespace SR_UTILS_NS {
    void LayoutElement::OnAttached() {
        Super::OnAttached();
        OnLayoutPropertyChanged();
    }

    void LayoutElement::OnDetached() {
        OnLayoutPropertyChanged();
        Super::OnDetached();
    }

    void LayoutElement::OnEnable() {
        Super::OnEnable();
        OnLayoutPropertyChanged();
    }

    void LayoutElement::OnDisable() {
        OnLayoutPropertyChanged();
        Super::OnDisable();
    }

    void LayoutElement::SetIgnoreLayout(const bool value) {
        if (m_ignoreLayout == value) return;
        m_ignoreLayout = value;
        OnLayoutPropertyChanged();
    }

    void LayoutElement::SetMinWidth(const float value) {
        if (m_minWidth == value) return;
        m_minWidth = value < 0.f ? -1.f : value;
        OnLayoutPropertyChanged();
    }

    void LayoutElement::SetMinHeight(const float value) {
        if (m_minHeight == value) return;
        m_minHeight = value < 0.f ? -1.f : value;
        OnLayoutPropertyChanged();
    }

    void LayoutElement::SetPreferredWidth(const float value) {
        if (m_preferredWidth == value) return;
        m_preferredWidth = value < 0.f ? -1.f : value;
        OnLayoutPropertyChanged();
    }

    void LayoutElement::SetPreferredHeight(const float value) {
        if (m_preferredHeight == value) return;
        m_preferredHeight = value < 0.f ? -1.f : value;
        OnLayoutPropertyChanged();
    }

    void LayoutElement::SetFlexibleWidth(const float value) {
        if (m_flexibleWidth == value) return;
        m_flexibleWidth = value < 0.f ? -1.f : value;
        OnLayoutPropertyChanged();
    }

    void LayoutElement::SetFlexibleHeight(const float value) {
        if (m_flexibleHeight == value) return;
        m_flexibleHeight = value < 0.f ? -1.f : value;
        OnLayoutPropertyChanged();
    }

    void LayoutElement::SetLayoutPriority(const int32_t value) {
        if (m_layoutPriority == value) return;
        m_layoutPriority = value;
        OnLayoutPropertyChanged();
    }

    void LayoutElement::OnLayoutPropertyChanged() {
        if (!IsAttached()) return;

        auto&& pGameObject = GetGameObject();
        if (!pGameObject) return;

        auto&& pParent = pGameObject->GetParent();
        if (!pParent) return;

        if (auto&& pLayoutGroup = pParent->GetComponent<LayoutGroup>()) {
            pLayoutGroup->SetLayoutDirty();
        }
    }
}
