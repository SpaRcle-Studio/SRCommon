//
// Created by Monika on 18.12.2025.
//

#include <Utils/UI/ContentSizeFitter.h>
#include <Utils/UI/LayoutUtility.h>
#include <Utils/ECS/TransformRect.h>
#include <Utils/ECS/TransformUtils.h>
#include <Utils/ECS/GameObject.h>

#include <Codegen/ContentSizeFitter.generated.hpp>

namespace SR_UTILS_NS {
    void ContentSizeFitter::SetLayout(const bool horizontal) {
        SR_TRACY_ZONE;

        auto&& pTransform = GetTransformAs<TransformRect>();
        auto&& pGameObject = GetGameObject();

        if (!pGameObject || !pTransform) {
            return;
        }

        if (m_horizontalFit == FitMode::Unconstrained && m_verticalFit == FitMode::Unconstrained) {
            return;
        }

        if (horizontal && m_horizontalFit != FitMode::Unconstrained) {
            const auto metrics = LayoutUtility::GetMetrics(*pGameObject, true);
            pTransform->SetSizeWithCurrentAnchors(true,
                m_horizontalFit == FitMode::MinSize ? metrics.minimum : metrics.preferred);
        }
        if (!horizontal && m_verticalFit != FitMode::Unconstrained) {
            const auto metrics = LayoutUtility::GetMetrics(*pGameObject, false);
            pTransform->SetSizeWithCurrentAnchors(false,
                m_verticalFit == FitMode::MinSize ? metrics.minimum : metrics.preferred);
        }

    }
}
