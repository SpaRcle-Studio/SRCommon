//
// Created by Monika on 18.12.2025.
//

#include <Utils/UI/ContentSizeFitter.h>
#include <Utils/ECS/TransformRect.h>
#include <Utils/ECS/TransformUtils.h>
#include <Utils/ECS/GameObject.h>

#include <Codegen/ContentSizeFitter.generated.hpp>

namespace SR_UTILS_NS {
    void ContentSizeFitter::Update(float_t dt) {
        SR_TRACY_ZONE;

        auto&& pTransform = GetTransformAs<TransformRect>();
        auto&& pGameObject = GetGameObject();

        if (!pGameObject || !pTransform) {
            Super::Update(dt);
            return;
        }

        if (m_horizontalFit == FitMode::Unconstrained && m_verticalFit == FitMode::Unconstrained) {
            Super::Update(dt);
            return;
        }

        auto&& pTransformRect = static_cast<SR_UTILS_NS::TransformRect*>(pTransform);
        SR_MATH_NS::FVector2 newSize = pTransformRect->GetSize();

        float_t minX = +FLT_MAX;
        float_t minY = +FLT_MAX;
        float_t maxX = -FLT_MAX;
        float_t maxY = -FLT_MAX;

        bool hasChildren = false;

        for (auto&& pChild : pGameObject->GetChildrenRef()) {
            if (!pChild->IsActive()) {
                continue;
            }

            if (auto&& pChildTransform = ExtractTransformAs<TransformRect>(pChild.Get())) {
                const auto& pos  = pChildTransform->GetTranslation();
                const auto& size = pChildTransform->GetSize();

                minX = std::min(minX, pos.x);
                minY = std::min(minY, pos.y);

                maxX = std::max(maxX, pos.x + size.x);
                maxY = std::max(maxY, pos.y + size.y);

                hasChildren = true;
            }
        }

        if (hasChildren) {
            if (m_horizontalFit != FitMode::Unconstrained) {
                newSize.x = maxX - minX;
            }

            if (m_verticalFit != FitMode::Unconstrained) {
                newSize.y = maxY - minY;
            }

            pTransformRect->SetSize(newSize);
        }

        Super::Update(dt);
    }
}
