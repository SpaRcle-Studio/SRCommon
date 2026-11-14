//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_UI_LAYOUT_TEST_METRICS_H
#define SR_ENGINE_UTILS_UI_LAYOUT_TEST_METRICS_H

#include <Utils/ECS/Component.h>
#include <Utils/UI/ILayoutElement.h>

namespace SR_UTILS_NS {
    /// Synthetic monospace metrics for layout conformance fixtures, not a text renderer.
    /// @category(Tests)
    class SR_COMMON_DLL_API LayoutTestMetrics final : public Component, public ILayoutElement {
        SR_CLASS()
    public:
        ILayoutElement* GetLayoutElement() noexcept override { return this; }
        bool ExecuteInEditMode() const override { return true; }
        void CalculateLayoutInput(bool horizontal) override;
        float_t GetMinWidth() const override { return m_glyphWidth; }
        float_t GetPreferredWidth() const override { return m_glyphWidth * m_glyphCount; }
        float_t GetFlexibleWidth() const override { return -1.f; }
        float_t GetMinHeight() const override { return m_lineHeight; }
        float_t GetPreferredHeight() const override { return m_preferredHeight; }
        float_t GetFlexibleHeight() const override { return -1.f; }
        int32_t GetLayoutPriority() const override { return 0; }

    private:
        /// @property @onChanged(SetLayoutDirty)
        uint32_t m_glyphCount = 100;
        /// @property @onChanged(SetLayoutDirty)
        float_t m_glyphWidth = 10.f;
        /// @property @onChanged(SetLayoutDirty)
        float_t m_lineHeight = 20.f;
        float_t m_preferredHeight = 20.f;
    };
}

#endif // SR_ENGINE_UTILS_UI_LAYOUT_TEST_METRICS_H
