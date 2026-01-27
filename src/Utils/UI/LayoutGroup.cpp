//
// Created by Monika on 18.12.2025.
//

#include <Utils/UI/LayoutGroup.h>
#include <Utils/ECS/TransformRect.h>
#include <Utils/ECS/TransformUtils.h>
#include <Utils/ECS/GameObject.h>

#include <Codegen/LayoutGroup.generated.hpp>

namespace SR_UTILS_NS {
    void LayoutGroup::Update(float_t dt) {
        SR_TRACY_ZONE;

        auto&& pTransform = GetTransformAs<TransformRect>();
        auto&& pGameObject = GetGameObject();

        if (!pGameObject || !pTransform) {
            Super::Update(dt);
            return;
        }

        const SR_MATH_NS::FRect layoutRect = pTransform->GetLayoutRect();

        // 1. Вычисляем размеры контента
        const float_t contentWidth  = layoutRect.Width() - m_paddingLeft - m_paddingRight;
        const float_t contentHeight = layoutRect.Height() - m_paddingTop - m_paddingBottom;

        // 2. Считаем общий размер детей
        float_t totalWidth = 0.f;
        float_t totalHeight = 0.f;

        int activeCount = 0;
        for (auto&& pChild : pGameObject->GetChildrenRef()) {
            if (!pChild->IsActive()) {
                continue;
            }

            if (auto&& pChildTransform = ExtractTransformAs<TransformRect>(pChild.Get())) {
                pChildTransform->SetAnchors(RectAnchors({0.0f, 1.0f}, {0.0f, 1.0f}));
                pChildTransform->SetPivot({0.0f, 1.0f});
                pChildTransform->SetOffsetMax(SR_MATH_NS::FVector2::Zero());
                pChildTransform->SetOffsetMin(SR_MATH_NS::FVector2::Zero());

                totalWidth += pChildTransform->GetLayoutRect().Width();
                totalHeight += pChildTransform->GetLayoutRect().Height();
                activeCount++;
            }
        }

        if (activeCount > 1) {
            totalWidth += m_spacing * static_cast<float_t>(activeCount - 1);
            totalHeight += m_spacing * static_cast<float_t>(activeCount - 1);
        }

        if (m_direction == LayoutGroupDirection::Vertical) {
            // 3. Определяем стартовую позицию Y в зависимости от вертикального выравнивания
            float_t y = 0.f;
            switch (m_childAlignVertical) {
                case LayoutGroupChildAlighVertical::Upper:  y = -m_paddingTop; break;
                case LayoutGroupChildAlighVertical::Middle: y = -(m_paddingTop + (contentHeight - totalHeight) * 0.5f); break;
                case LayoutGroupChildAlighVertical::Lower:  y = -(layoutRect.Height() - m_paddingBottom - totalHeight); break;
                default: break;
            }

            // 4. Расставляем детей
            for (auto&& pChild : pGameObject->GetChildrenRef()) {
                if (!pChild->IsActive()) {
                    continue;
                }

                if (auto&& pChildTransform = ExtractTransformAs<TransformRect>(pChild.Get())) {
                    // Определяем X для горизонтального выравнивания
                    float_t x = 0.f;
                    switch (m_childAlignHorizontal) {
                        case LayoutGroupChildAlighHorizontal::Left:   x = m_paddingLeft; break;
                        case LayoutGroupChildAlighHorizontal::Center: x = m_paddingLeft + (contentWidth - pChildTransform->GetLayoutRect().Width()) * 0.5f; break;
                        case LayoutGroupChildAlighHorizontal::Right:  x = layoutRect.Width() - m_paddingRight - pChildTransform->GetLayoutRect().Width(); break;
                        default: break;
                    }

                    // Устанавливаем позицию
                    pChildTransform->SetTranslation(SR_MATH_NS::FVector3(x, y, pChildTransform->GetTranslation().z));

                    // Сдвигаем Y для следующего ребенка
                    y -= pChildTransform->GetLayoutRect().Height() + m_spacing;
                }
            }
        }
        else if (m_direction == LayoutGroupDirection::Horizontal) {
            // 3. Определяем стартовую позицию X в зависимости от горизонтального выравнивания
            float_t x = 0.f;
            switch (m_childAlignHorizontal) {
                case LayoutGroupChildAlighHorizontal::Left:   x = m_paddingLeft; break;
                case LayoutGroupChildAlighHorizontal::Center: x = m_paddingLeft + (contentWidth - totalWidth) * 0.5f; break;
                case LayoutGroupChildAlighHorizontal::Right:  x = layoutRect.Width() - m_paddingRight - totalWidth; break;
                default: break;
            }

            // 4. Расставляем детей
            for (auto&& pChild : pGameObject->GetChildrenRef()) {
                if (!pChild->IsActive()) {
                    continue;
                }

                if (auto&& pChildTransform = ExtractTransformAs<TransformRect>(pChild.Get())) {
                    // Определяем Y для вертикального выравнивания
                    float_t y = 0.f;
                    switch (m_childAlignVertical) {
                        case LayoutGroupChildAlighVertical::Upper:  y = -m_paddingTop; break;
                        case LayoutGroupChildAlighVertical::Middle: y = -(m_paddingTop + (contentHeight - pChildTransform->GetLayoutRect().Height()) * 0.5f); break;
                        case LayoutGroupChildAlighVertical::Lower:  y = -(layoutRect.Height() - m_paddingBottom - pChildTransform->GetLayoutRect().Height()); break;
                        default: break;
                    }

                    // Устанавливаем позицию
                    pChildTransform->SetTranslation(SR_MATH_NS::FVector3(x, y, pChildTransform->GetTranslation().z));

                    // Сдвигаем X для следующего ребенка
                    x += pChildTransform->GetLayoutRect().Width() + m_spacing;
                }
            }
        }
        else {
            SRHalt("LayoutGroup::Update() : unknown direction!");
        }

        Super::Update(dt);
    }
}
