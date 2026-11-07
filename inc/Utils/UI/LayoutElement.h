//
// Created by Codex on 06.11.2026.
//

#ifndef SR_ENGINE_UTILS_UI_LAYOUT_ELEMENT_H
#define SR_ENGINE_UTILS_UI_LAYOUT_ELEMENT_H

#include <Utils/ECS/Component.h>
#include <Utils/UI/ILayoutElement.h>

namespace SR_UTILS_NS {
    /// @category(UI)
    class LayoutElement final : public Component, public ILayoutElement {
        SR_CLASS()
        using Super = Component;
    public:
        LayoutElement() = default;

        void OnAttached() override;
        void OnDetached() override;
        void OnEnable() override;
        void OnDisable() override;

        SR_NODISCARD bool ExecuteInEditMode() const override { return true; }
        SR_NODISCARD bool IsLayoutIgnored() const override { return m_ignoreLayout; }
        SR_NODISCARD float GetMinWidth() const override { return m_minWidth; }
        SR_NODISCARD float GetPreferredWidth() const override { return m_preferredWidth; }
        SR_NODISCARD float GetFlexibleWidth() const override { return m_flexibleWidth; }
        SR_NODISCARD float GetMinHeight() const override { return m_minHeight; }
        SR_NODISCARD float GetPreferredHeight() const override { return m_preferredHeight; }
        SR_NODISCARD float GetFlexibleHeight() const override { return m_flexibleHeight; }
        SR_NODISCARD int32_t GetLayoutPriority() const override { return m_layoutPriority; }

        void SetIgnoreLayout(bool value);
        void SetMinWidth(float value);
        void SetMinHeight(float value);
        void SetPreferredWidth(float value);
        void SetPreferredHeight(float value);
        void SetFlexibleWidth(float value);
        void SetFlexibleHeight(float value);
        void SetLayoutPriority(int32_t value);

    private:
        void OnLayoutPropertyChanged();

    private:
        /// @property @onChanged(OnLayoutPropertyChanged)
        bool m_ignoreLayout = false;
        /// @property @onChanged(OnLayoutPropertyChanged)
        float m_minWidth = -1.f;
        /// @property @onChanged(OnLayoutPropertyChanged)
        float m_minHeight = -1.f;
        /// @property @onChanged(OnLayoutPropertyChanged)
        float m_preferredWidth = -1.f;
        /// @property @onChanged(OnLayoutPropertyChanged)
        float m_preferredHeight = -1.f;
        /// @property @onChanged(OnLayoutPropertyChanged)
        float m_flexibleWidth = -1.f;
        /// @property @@onChanged(OnLayoutPropertyChanged)
        float m_flexibleHeight = -1.f;
        /// @property @onChanged(OnLayoutPropertyChanged)
        int32_t m_layoutPriority = 1;
    };
}

#endif // SR_ENGINE_UTILS_UI_LAYOUT_ELEMENT_H
