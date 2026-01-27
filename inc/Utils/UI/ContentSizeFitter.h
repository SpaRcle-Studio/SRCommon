//
// Created by Monika on 26.01.2026.
//


#ifndef SR_ENGINE_UTILS_UI_CONTENT_SIZE_FILTER_H
#define SR_ENGINE_UTILS_UI_CONTENT_SIZE_FILTER_H

#include <Utils/ECS/Component.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(FitMode, uint8_t,
        Unconstrained, // Не меняем
        MinSize,       // Подстраиваем под минимальный размер детей
        PreferredSize  // Подстраиваем под предпочитаемый размер детей
    );

    /// @category(UI)
    class ContentSizeFitter : public SR_UTILS_NS::Component {
    SR_CLASS()
        using Super = SR_UTILS_NS::Component;
    public:
        ContentSizeFitter() = default;

    public:
        void Update(float_t dt) override;

        SR_NODISCARD bool ExecuteInEditMode() const override { return true; }

        SR_NODISCARD FitMode GetHorizontalFit() const { return m_horizontalFit; }
        void SetHorizontalFit(FitMode fit) { m_horizontalFit = fit; }

        SR_NODISCARD FitMode GetVerticalFit() const { return m_verticalFit; }
        void SetVerticalFit(FitMode fit) { m_verticalFit = fit; }

    private:
        /// @property
        FitMode m_horizontalFit = FitMode::Unconstrained;
        /// @property
        FitMode m_verticalFit = FitMode::Unconstrained;


    };
}

#endif //SR_ENGINE_UTILS_UI_CONTENT_SIZE_FILTER_H
