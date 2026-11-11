//
// Created by Monika on 26.01.2026.
//


#ifndef SR_ENGINE_UTILS_UI_LAYOUT_GROUP_H
#define SR_ENGINE_UTILS_UI_LAYOUT_GROUP_H

#include <Utils/ECS/Component.h>

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
    class SR_COMMON_DLL_API LayoutGroup final : public SR_UTILS_NS::Component {
        SR_CLASS()
        using Super = SR_UTILS_NS::Component;
    public:
        struct LayoutSize {
            float min = 0.f;
            float preferred = 0.f;
            float flexible = 0.f;
            float result = 0.f;
        };
        struct LayoutChild {
            TransformRect* pTransform = nullptr;
            LayoutSize main;
            LayoutSize cross;
        };
    public:
        LayoutGroup() = default;

    public:
        void Update(float_t dt) override;

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
        void SetLayoutDirty() noexcept { m_layoutDirty = true; }

    private:
        /// @property @onChanged(SetLayoutDirty)
        LayoutGroupDirection m_direction = LayoutGroupDirection::Vertical;
        /// @property @onChanged(SetLayoutDirty)
        LayoutGroupChildAlighHorizontal m_childAlignHorizontal = LayoutGroupChildAlighHorizontal::Center;
        /// @property @onChanged(SetLayoutDirty)
        LayoutGroupChildAlighVertical m_childAlignVertical = LayoutGroupChildAlighVertical::Middle;
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
        bool m_controlChildWidth = true;
        /// @property @onChanged(SetLayoutDirty)
        bool m_controlChildHeight = true;
        /// @property @onChanged(SetLayoutDirty)
        bool m_childForceExpandWidth = false;
        /// @property @onChanged(SetLayoutDirty)
        bool m_childForceExpandHeight = false;
        bool m_layoutDirty = true;

        Vector<LayoutChild> m_childrenCache;
        SR_MATH_NS::FVector2 m_lastLayoutSize = SR_MATH_NS::FVector2(-1.f);
        SR_MATH_NS::FVector2 m_lastChildrenSize = SR_MATH_NS::FVector2(-1.f);
        uint32_t m_lastChildrenCount = 0;
        uint32_t m_lastActiveChildrenCount = 0;
        uint64_t m_lastActiveChildrenHash = 0;

    };
}

#endif //SR_ENGINE_UTILS_UI_LAYOUT_GROUP_H
