//
// Created by Monika on 26.01.2026.
//


#ifndef SR_ENGINE_UTILS_UI_CONTENT_SIZE_FILTER_H
#define SR_ENGINE_UTILS_UI_CONTENT_SIZE_FILTER_H

#include <Utils/ECS/Component.h>
#include <Utils/UI/ILayoutElement.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(FitMode, uint8_t,
        Unconstrained, // Не меняем
        MinSize,
        PreferredSize
    );

    /// @category(UI)
    class SR_COMMON_DLL_API ContentSizeFitter : public SR_UTILS_NS::Component, public ILayoutController {
    SR_CLASS()
        using Super = SR_UTILS_NS::Component;
    public:
        ContentSizeFitter() = default;
        ILayoutController* GetLayoutController() noexcept override { return this; }

    public:
        bool IsSelfController() const override { return true; }
        void SetLayout(bool horizontal) override;

        SR_NODISCARD bool ExecuteInEditMode() const override { return true; }

        SR_NODISCARD FitMode GetHorizontalFit() const { return m_horizontalFit; }
        void SetHorizontalFit(FitMode fit) {
            if (m_horizontalFit != fit) {
                m_horizontalFit = fit;
                SetLayoutDirty();
            }
        }

        SR_NODISCARD FitMode GetVerticalFit() const { return m_verticalFit; }
        void SetVerticalFit(FitMode fit) {
            if (m_verticalFit != fit) {
                m_verticalFit = fit;
                SetLayoutDirty();
            }
        }

    private:
        /// @property @setter(SetHorizontalFit)
        FitMode m_horizontalFit = FitMode::Unconstrained;
        /// @property @setter(SetVerticalFit)
        FitMode m_verticalFit = FitMode::Unconstrained;


    };
}

#endif //SR_ENGINE_UTILS_UI_CONTENT_SIZE_FILTER_H
