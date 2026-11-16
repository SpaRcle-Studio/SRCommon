//
// Created by Monika on 18.12.2025.
//

#ifndef SR_ENGINE_UTILS_UI_ASPECT_RATIO_FILTER_H
#define SR_ENGINE_UTILS_UI_ASPECT_RATIO_FILTER_H

#include <Utils/ECS/Component.h>
#include <Utils/UI/ILayoutElement.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(AspectRatioMode, uint8_t,
        None,
        WidthControlsHeight,
        HeightControlsWidth,
        FitInParent,
        EnvelopeParent
    );

    /// @category(UI)
    class SR_COMMON_DLL_API AspectRatioFitter : public SR_UTILS_NS::Component, public ILayoutController {
        SR_CLASS()
        using Super = SR_UTILS_NS::Component;
    public:
        AspectRatioFitter() = default;
        ILayoutController* GetLayoutController() noexcept override { return this; }

    public:
        bool IsSelfController() const override { return true; }
        void SetLayout(bool horizontal) override;

        SR_NODISCARD bool ExecuteInEditMode() const override { return true; }

        SR_NODISCARD float_t GetAspectRatio() const { return m_aspectRatio; }
        void SetAspectRatio(float_t aspectRatio) {
            if (m_aspectRatio != aspectRatio) {
                m_aspectRatio = aspectRatio;
                SetLayoutDirty();
            }
        }

    private:
        /// @property @onChanged(SetLayoutDirty)
        AspectRatioMode m_mode = AspectRatioMode::None;
        /// @property @setter(SetAspectRatio)
        float_t m_aspectRatio = 1.f;
        /// @property @onChanged(SetLayoutDirty) @condition(This.m_mode == SR_UTILS_NS::AspectRatioMode::FitInParent)
        SR_MATH_NS::FVector2 m_referenceSize;

    };
}

#endif //SR_ENGINE_UTILS_UI_ASPECT_RATIO_FILTER_H
