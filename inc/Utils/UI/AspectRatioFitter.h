//
// Created by Monika on 18.12.2025.
//

#ifndef SR_ENGINE_UTILS_UI_ASPECT_RATIO_FILTER_H
#define SR_ENGINE_UTILS_UI_ASPECT_RATIO_FILTER_H

#include <Utils/ECS/Component.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(AspectRatioMode, uint8_t,
        None,
        WidthControlsHeight,
        HeightControlsWidth,
        FitInParent,
        EnvelopeParent
    );

    /// @category(UI)
    class AspectRatioFitter : public SR_UTILS_NS::Component {
        SR_CLASS()
        using Super = SR_UTILS_NS::Component;
    public:
        AspectRatioFitter() = default;

    public:
        void Update(float_t dt) override;

        SR_NODISCARD bool ExecuteInEditMode() const override { return true; }

        SR_NODISCARD float_t GetAspectRatio() const { return m_aspectRatio; }
        void SetAspectRatio(float_t aspectRatio) { m_aspectRatio = aspectRatio; }

    private:
        /// @property
        AspectRatioMode m_mode = AspectRatioMode::None;
        /// @property
        float_t m_aspectRatio = 16.f / 9.f;

    };
}

#endif //SR_ENGINE_UTILS_UI_ASPECT_RATIO_FILTER_H
