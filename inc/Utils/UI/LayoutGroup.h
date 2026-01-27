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
    class LayoutGroup final : public SR_UTILS_NS::Component {
    SR_CLASS()
        using Super = SR_UTILS_NS::Component;
    public:
        LayoutGroup() = default;

    public:
        void Update(float_t dt) override;

        SR_NODISCARD bool ExecuteInEditMode() const override { return true; }

    private:
        /// @property
        LayoutGroupDirection m_direction = LayoutGroupDirection::Vertical;
        /// @property
        LayoutGroupChildAlighHorizontal m_childAlignHorizontal = LayoutGroupChildAlighHorizontal::Center;
        /// @property
        LayoutGroupChildAlighVertical m_childAlignVertical = LayoutGroupChildAlighVertical::Middle;
        /// @property
        float_t m_spacing = 0.f;
        /// @property
        float_t m_paddingTop = 0.f;
        /// @property
        float_t m_paddingBottom = 0.f;
        /// @property
        float_t m_paddingLeft = 0.f;
        /// @property
        float_t m_paddingRight = 0.f;

    };
}

#endif //SR_ENGINE_UTILS_UI_LAYOUT_GROUP_H
