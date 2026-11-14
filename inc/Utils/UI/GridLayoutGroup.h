//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_UI_GRID_LAYOUT_GROUP_H
#define SR_ENGINE_UTILS_UI_GRID_LAYOUT_GROUP_H

#include <Utils/ECS/Component.h>
#include <Utils/UI/ILayoutElement.h>
#include <Utils/UI/LayoutUtility.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(GridStartCorner, uint8_t, UpperLeft, UpperRight, LowerLeft, LowerRight);
    SR_ENUM_NS_CLASS_T(GridStartAxis, uint8_t, Horizontal, Vertical);
    SR_ENUM_NS_CLASS_T(GridConstraint, uint8_t, Flexible, FixedColumnCount, FixedRowCount);

    /// @category(UI)
    class SR_COMMON_DLL_API GridLayoutGroup final : public Component, public ILayoutElement, public ILayoutController {
        SR_CLASS()
    public:
        ILayoutElement* GetLayoutElement() noexcept override { return this; }
        ILayoutController* GetLayoutController() noexcept override { return this; }
        bool ExecuteInEditMode() const override { return true; }
        void CalculateLayoutInput(bool horizontal) override;
        void SetLayout(bool horizontal) override;
        float_t GetMinWidth() const override { return m_metrics[0].minimum; }
        float_t GetPreferredWidth() const override { return m_metrics[0].preferred; }
        float_t GetFlexibleWidth() const override { return -1.f; }
        float_t GetMinHeight() const override { return m_metrics[1].minimum; }
        float_t GetPreferredHeight() const override { return m_metrics[1].preferred; }
        float_t GetFlexibleHeight() const override { return -1.f; }
        int32_t GetLayoutPriority() const override { return 0; }
        void SetConstraintCount(int32_t count) {
            count = SR_MAX(1, count);
            if (m_constraintCount != count) {
                m_constraintCount = count;
                SetLayoutDirty();
            }
        }

    private:
        /// @property @onChanged(SetLayoutDirty)
        GridStartCorner m_startCorner = GridStartCorner::UpperLeft;
        /// @property @onChanged(SetLayoutDirty)
        GridStartAxis m_startAxis = GridStartAxis::Horizontal;
        /// @property @onChanged(SetLayoutDirty)
        SR_MATH_NS::FVector2 m_cellSize = SR_MATH_NS::FVector2(100.f);
        /// @property @onChanged(SetLayoutDirty)
        SR_MATH_NS::FVector2 m_spacing;
        /// @property @onChanged(SetLayoutDirty)
        GridConstraint m_constraint = GridConstraint::Flexible;
        /// @property @onChanged(SetLayoutDirty) @setter(SetConstraintCount)
        int32_t m_constraintCount = 2;
        /// @property @onChanged(SetLayoutDirty)
        SR_MATH_NS::FVector2 m_paddingMin;
        /// @property @onChanged(SetLayoutDirty)
        SR_MATH_NS::FVector2 m_paddingMax;
        /// @property @onChanged(SetLayoutDirty) @range(0, 8)
        int32_t m_childAlignment = 0;
        LayoutUtility::Metrics m_metrics[2];
        Vector<TransformRect*> m_children;
    };
}

#endif // SR_ENGINE_UTILS_UI_GRID_LAYOUT_GROUP_H
