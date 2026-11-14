//
// Created by Monika on 26.01.2026.
//


#ifndef SR_ENGINE_UTILS_UI_LAYOUT_GROUP_H
#define SR_ENGINE_UTILS_UI_LAYOUT_GROUP_H

#include <Utils/ECS/Component.h>
#include <Utils/UI/ILayoutElement.h>
#include <Utils/UI/LayoutUtility.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(LayoutGroupDirection, uint8_t,
        Horizontal,
        Vertical
    );

    SR_ENUM_NS_CLASS_T(LayoutGroupChildAlighVertical, uint8_t,
        Upper,
        Middle,
        Lower
    );

    SR_ENUM_NS_CLASS_T(LayoutGroupChildAlighHorizontal, uint8_t,
        Left,
        Center,
        Right
    );

    /// @category(UI)
    class SR_COMMON_DLL_API LayoutGroup final : public SR_UTILS_NS::Component, public ILayoutElement, public ILayoutController {
        SR_CLASS()
        using Super = SR_UTILS_NS::Component;
    public:
        struct LayoutSize {
            float min = 0.f;
            float preferred = 0.f;
            float flexible = 0.f;
        };
        struct LayoutChild {
            TransformRect* pTransform = nullptr;
            LayoutSize main;
            LayoutSize cross;
        };
    public:
        LayoutGroup() = default;
        ILayoutElement* GetLayoutElement() noexcept override { return this; }
        ILayoutController* GetLayoutController() noexcept override { return this; }

    public:
        void CalculateLayoutInput(bool horizontal) override;
        void SetLayout(bool horizontal) override;
        float_t GetMinWidth() const override { return m_metrics[0].minimum; }
        float_t GetPreferredWidth() const override { return m_metrics[0].preferred; }
        float_t GetFlexibleWidth() const override { return m_metrics[0].flexible; }
        float_t GetMinHeight() const override { return m_metrics[1].minimum; }
        float_t GetPreferredHeight() const override { return m_metrics[1].preferred; }
        float_t GetFlexibleHeight() const override { return m_metrics[1].flexible; }
        int32_t GetLayoutPriority() const override { return 0; }

        SR_NODISCARD bool ExecuteInEditMode() const override { return true; }

        void SetDirection(LayoutGroupDirection direction);
        void SetChildAlignHorizontal(LayoutGroupChildAlighHorizontal align);
        void SetChildAlignVertical(LayoutGroupChildAlighVertical align);
        void SetSpacing(float_t spacing);
        void SetPaddingTop(float_t value);
        void SetPaddingBottom(float_t value);
        void SetPaddingLeft(float_t value);
        void SetPaddingRight(float_t value);
        void SetControlChildWidth(bool value);
        void SetControlChildHeight(bool value);
        void SetChildForceExpandWidth(bool value);
        void SetChildForceExpandHeight(bool value);
        void SetChildScaleWidth(bool value);
        void SetChildScaleHeight(bool value);
        void SetReverseArrangement(bool value);

    private:
        /// @property @onChanged(SetLayoutDirty)
        LayoutGroupDirection m_direction = LayoutGroupDirection::Vertical;
        /// @property @onChanged(SetLayoutDirty)
        LayoutGroupChildAlighHorizontal m_childAlignHorizontal = LayoutGroupChildAlighHorizontal::Left;
        /// @property @onChanged(SetLayoutDirty)
        LayoutGroupChildAlighVertical m_childAlignVertical = LayoutGroupChildAlighVertical::Upper;
        /// @property @onChanged(SetLayoutDirty)
        float_t m_spacing = 0.f;
        /// @property @onChanged(SetLayoutDirty)
        float_t m_paddingTop = 0.f;
        /// @property @onChanged(SetLayoutDirty)
        float_t m_paddingBottom = 0.f;
        /// @property @onChanged(SetLayoutDirty)
        float_t m_paddingLeft = 0.f;
        /// @property @onChanged(SetLayoutDirty)
        float_t m_paddingRight = 0.f;
        /// @property @onChanged(SetLayoutDirty)
        bool m_controlChildWidth = false;
        /// @property @onChanged(SetLayoutDirty)
        bool m_controlChildHeight = false;
        /// @property @onChanged(SetLayoutDirty)
        bool m_childForceExpandWidth = true;
        /// @property @onChanged(SetLayoutDirty)
        bool m_childForceExpandHeight = true;
        /// @property @onChanged(SetLayoutDirty)
        bool m_childScaleWidth = false;
        /// @property @onChanged(SetLayoutDirty)
        bool m_childScaleHeight = false;
        /// @property @onChanged(SetLayoutDirty)
        bool m_reverseArrangement = false;
        LayoutUtility::Metrics m_metrics[2];

        Vector<LayoutChild> m_childrenCache;

    };
}

#endif //SR_ENGINE_UTILS_UI_LAYOUT_GROUP_H
