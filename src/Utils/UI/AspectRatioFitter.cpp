//
// Created by Monika on 18.12.2025.
//

#include <Utils/UI/AspectRatioFitter.h>
#include <Utils/ECS/TransformRect.h>

#include <Codegen/AspectRatioFitter.generated.hpp>

namespace SR_UTILS_NS {
    void AspectRatioFitter::SetLayout(const bool horizontal) {
        SR_TRACY_ZONE;

        auto&& pTransform = GetTransform();
        if (!pTransform || !pTransform->GetMeta()->IsSameOrInherited(TransformRect::GetClassStaticName())) {
            return;
        }

        auto&& pTransformRect = static_cast<SR_UTILS_NS::TransformRect*>(pTransform);

        const SR_MATH_NS::FVector2 size = pTransformRect->GetLayoutRect().Size();
        const float_t aspectRatio = SR_MAX(0.001f, m_aspectRatio);
        float_t width  = size.x;
        float_t height = size.y;

        switch (m_mode) {
            default:
                SRHalt("AspectRatioFitter::SetLayout() : unknown aspect ratio mode!");
                break;

            case AspectRatioMode::None:
                break;

            case AspectRatioMode::WidthControlsHeight: {
                if (!horizontal) {
                    pTransformRect->SetSizeWithCurrentAnchors(false, width / aspectRatio);
                }
                break;
            }

            case AspectRatioMode::HeightControlsWidth: {
                if (horizontal) {
                    pTransformRect->SetSizeWithCurrentAnchors(true, height * aspectRatio);
                }
                break;
            }

            case AspectRatioMode::FitInParent:
            case AspectRatioMode::EnvelopeParent: {
                auto&& pParent = pTransformRect->GetParentTransform();
                if (!pParent || !pParent->GetMeta()->IsSameOrInherited(TransformRect::GetClassStaticName())) {
                    break;
                }

                auto&& pParentTransformRect = static_cast<SR_UTILS_NS::TransformRect*>(pParent);
                const SR_MATH_NS::FVector2 parentSize = pParentTransformRect->GetLayoutRect().Size();

                if (parentSize.x <= 0.f || parentSize.y <= 0.f) {
                    break;
                }

                const float_t parentAspect = parentSize.x / parentSize.y;

                if (m_mode == AspectRatioMode::FitInParent && m_referenceSize.x > 0.f && m_referenceSize.y > 0.f) {
                    const float_t scale = SR_MIN(parentSize.x / m_referenceSize.x, parentSize.y / m_referenceSize.y);
                    pTransformRect->SetAnchors(RectAnchors(SR_MATH_NS::FVector2(0.5f), SR_MATH_NS::FVector2(0.5f)));
                    pTransformRect->SetAnchoredPosition(SR_MATH_NS::FVector2::Zero());
                    pTransformRect->SetSize(m_referenceSize);
                    pTransformRect->SetScale(SR_MATH_NS::FVector3(scale, scale, 1.f));
                    break;
                }

                bool fitWidth;

                if (m_mode == AspectRatioMode::FitInParent) {
                    fitWidth = parentAspect > aspectRatio;
                }
                else {
                    fitWidth = parentAspect < aspectRatio;
                }

                if (fitWidth) {
                    height = parentSize.y;
                    width  = height * aspectRatio;
                }
                else {
                    width  = parentSize.x;
                    height = width / aspectRatio;
                }

                pTransformRect->SetAnchors(RectAnchors::Fill());
                pTransformRect->SetAnchoredPosition(SR_MATH_NS::FVector2::Zero());
                pTransformRect->SetSizeWithCurrentAnchors(true, width);
                pTransformRect->SetSizeWithCurrentAnchors(false, height);
                break;
            }
        }

    }
}
