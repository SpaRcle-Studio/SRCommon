//
// Created by Monika on 18.12.2025.
//

#include <Utils/UI/AspectRatioFitter.h>
#include <Utils/ECS/TransformRect.h>

#include <Codegen/AspectRatioFitter.generated.hpp>

namespace SR_UTILS_NS {
    void AspectRatioFitter::Update(float_t dt) {
        SR_TRACY_ZONE;

        auto&& pTransform = GetTransform();
        if (!pTransform || pTransform->GetMeasurement() != SR_UTILS_NS::Measurement::Space2D) {
            Super::Update(dt);
            return;
        }

        auto&& pTransformRect = static_cast<SR_UTILS_NS::TransformRect*>(pTransform);

        const SR_MATH_NS::FVector2 size = pTransformRect->GetSize();
        float_t width  = size.x;
        float_t height = size.y;

        switch (m_mode) {
            default:
                SRHalt("Unknown aspect ratio mode!");
                break;

            case AspectRatioMode::None:
                break;

            case AspectRatioMode::WidthControlsHeight: {
                height = width / m_aspectRatio;
                pTransformRect->SetSize({ width, height });
                break;
            }

            case AspectRatioMode::HeightControlsWidth: {
                width = height * m_aspectRatio;
                pTransformRect->SetSize({ width, height });
                break;
            }

            case AspectRatioMode::FitInParent:
            case AspectRatioMode::EnvelopeParent: {
                auto&& pParent = pTransformRect->GetParentTransform();
                if (!pParent || pParent->GetMeasurement() != SR_UTILS_NS::Measurement::Space2D) {
                    break;
                }

                auto&& pParentTransformRect = static_cast<SR_UTILS_NS::TransformRect*>(pParent);
                const SR_MATH_NS::FVector2 parentSize = pParentTransformRect->GetLayoutRect().Size();

                const float_t parentAspect = parentSize.x / parentSize.y;

                bool fitWidth;

                if (m_mode == AspectRatioMode::FitInParent) {
                    fitWidth = parentAspect > m_aspectRatio; /// ВПИСЫВАЕМСЯ
                }
                else {
                    fitWidth = parentAspect < m_aspectRatio; /// ОБВОЛАКИВАЕМ
                }

                if (fitWidth) {
                    height = parentSize.y;
                    width  = height * m_aspectRatio;
                }
                else {
                    width  = parentSize.x;
                    height = width / m_aspectRatio;
                }

                pTransformRect->SetSize({ width, height });
                break;
            }
        }

        Super::Update(dt);
    }
}
