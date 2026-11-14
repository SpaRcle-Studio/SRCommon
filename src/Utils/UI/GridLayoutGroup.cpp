//
// Created by Codex on 15.11.2026.
//

#include <Utils/UI/GridLayoutGroup.h>
#include <Utils/ECS/TransformRect.h>
#include <Utils/ECS/TransformUtils.h>
#include <Utils/ECS/GameObject.h>

#include <Codegen/GridLayoutGroup.generated.hpp>

namespace SR_UTILS_NS {
    void GridLayoutGroup::CalculateLayoutInput(const bool horizontal) {
        SR_TRACY_ZONE;
        auto&& pObject = GetGameObject();
        auto&& pRect = GetTransformAs<TransformRect>();
        if (!pObject || !pRect) {
            return;
        }
        if (horizontal) {
            m_children.clear();
            for (auto&& pChildSO : pObject->GetChildrenRef()) {
                if (pChildSO->GetMeta() != GameObject::GetMetaStatic()) {
                    continue;
                }
                auto&& pChild = static_cast<GameObject*>(pChildSO.Get());
                if (!pChild->IsActive() || LayoutUtility::IsIgnored(*pChild)) {
                    continue;
                }
                if (auto&& pChildRect = ExtractTransformAs<TransformRect>(pChild)) {
                    m_children.emplace_back(pChildRect);
                }
            }
        }
        const int32_t count = static_cast<int32_t>(m_children.size());
        const int32_t constraint = SR_MAX(1, m_constraintCount);
        float_t minimum;
        float_t preferred;
        if (horizontal) {
            if (m_constraint == GridConstraint::FixedColumnCount) {
                minimum = preferred = static_cast<float_t>(constraint);
            }
            else if (m_constraint == GridConstraint::FixedRowCount) {
                minimum = preferred = std::ceil(static_cast<float_t>(count) / constraint - 0.001f);
            }
            else {
                minimum = 1.f;
                preferred = std::ceil(std::sqrt(static_cast<float_t>(count)));
            }
            const float_t padding = m_paddingMin.x + m_paddingMax.x;
            m_metrics[0] = { padding + (m_cellSize.x + m_spacing.x) * minimum - m_spacing.x,
                padding + (m_cellSize.x + m_spacing.x) * preferred - m_spacing.x, -1.f };
        }
        else {
            if (m_constraint == GridConstraint::FixedRowCount) {
                minimum = static_cast<float_t>(constraint);
            }
            else if (m_constraint == GridConstraint::FixedColumnCount) {
                minimum = std::ceil(static_cast<float_t>(count) / constraint - 0.001f);
            }
            else {
                const float_t width = pRect->GetLayoutRect().Width() - m_paddingMin.x - m_paddingMax.x;
                const float_t columns = m_cellSize.x + m_spacing.x <= 0.f ? static_cast<float_t>(INT32_MAX)
                    : SR_MAX(1.f, std::floor((width + m_spacing.x + 0.001f) / (m_cellSize.x + m_spacing.x)));
                minimum = std::ceil(static_cast<float_t>(count) / columns);
            }
            const float_t space = m_paddingMin.y + m_paddingMax.y + (m_cellSize.y + m_spacing.y) * minimum - m_spacing.y;
            m_metrics[1] = { space, space, -1.f };
        }
    }

    void GridLayoutGroup::SetLayout(const bool horizontal) {
        SR_TRACY_ZONE;
        auto&& pRect = GetTransformAs<TransformRect>();
        if (!pRect) {
            return;
        }
        if (horizontal) {
            for (auto&& pChild : m_children) {
                pChild->SetAnchors(RectAnchors(SR_MATH_NS::FVector2(0.f, 1.f), SR_MATH_NS::FVector2(0.f, 1.f)));
                pChild->SetSize(m_cellSize);
            }
            return;
        }
        const int32_t count = static_cast<int32_t>(m_children.size());
        if (count == 0) {
            return;
        }
        const int32_t constraint = SR_MAX(1, m_constraintCount);
        int32_t columns = 1;
        int32_t rows = 1;
        const auto size = pRect->GetLayoutRect().Size();
        if (m_constraint == GridConstraint::FixedColumnCount) {
            columns = constraint;
            rows = SR_MAX(1, (count + columns - 1) / columns);
        }
        else if (m_constraint == GridConstraint::FixedRowCount) {
            rows = constraint;
            columns = SR_MAX(1, (count + rows - 1) / rows);
        }
        else {
            columns = m_cellSize.x + m_spacing.x <= 0.f ? INT32_MAX : SR_MAX(1, static_cast<int32_t>(std::floor(
                (size.x - m_paddingMin.x - m_paddingMax.x + m_spacing.x + 0.001f) / (m_cellSize.x + m_spacing.x))));
            rows = m_cellSize.y + m_spacing.y <= 0.f ? INT32_MAX : SR_MAX(1, static_cast<int32_t>(std::floor(
                (size.y - m_paddingMin.y - m_paddingMax.y + m_spacing.y + 0.001f) / (m_cellSize.y + m_spacing.y))));
        }
        const bool rowFirst = m_startAxis == GridStartAxis::Horizontal;
        const int32_t perAxis = rowFirst ? columns : rows;
        int32_t actualColumns = SR_CLAMP(columns, 1, count);
        int32_t actualRows = SR_CLAMP(rows, 1, count);
        if (rowFirst && m_constraint != GridConstraint::FixedRowCount) {
            actualRows = SR_CLAMP(rows, 1, static_cast<int32_t>(std::ceil(static_cast<float_t>(count) / perAxis)));
        }
        if (!rowFirst && m_constraint != GridConstraint::FixedColumnCount) {
            actualColumns = SR_CLAMP(columns, 1, static_cast<int32_t>(std::ceil(static_cast<float_t>(count) / perAxis)));
        }
        const SR_MATH_NS::FVector2 required(actualColumns * m_cellSize.x + (actualColumns - 1) * m_spacing.x,
            actualRows * m_cellSize.y + (actualRows - 1) * m_spacing.y);
        const int32_t alignment = SR_CLAMP(m_childAlignment, 0, 8);
        const SR_MATH_NS::FVector2 align(static_cast<float_t>(alignment % 3) * 0.5f, static_cast<float_t>(alignment / 3) * 0.5f);
        const auto start = m_paddingMin + (size - m_paddingMin - m_paddingMax - required) * align;
        int32_t childrenToMove = 0;
        const int32_t occupied = static_cast<int32_t>(std::ceil(static_cast<float_t>(count) / perAxis));
        if (perAxis > 1 && count > constraint && occupied < constraint) {
            childrenToMove = constraint - occupied;
            childrenToMove += childrenToMove / (perAxis - 1);
            if (count % perAxis == 1) {
                ++childrenToMove;
            }
        }
        for (int32_t i = 0; i < count; ++i) {
            int32_t x = rowFirst ? i % perAxis : i / perAxis;
            int32_t y = rowFirst ? i / perAxis : i % perAxis;
            if (rowFirst && m_constraint == GridConstraint::FixedRowCount && count - i <= childrenToMove) {
                x = 0;
                y = constraint - (count - i);
            }
            if (!rowFirst && m_constraint == GridConstraint::FixedColumnCount && count - i <= childrenToMove) {
                x = constraint - (count - i);
                y = 0;
            }
            if (static_cast<uint32_t>(m_startCorner) % 2 == 1) {
                x = actualColumns - 1 - x;
            }
            if (static_cast<uint32_t>(m_startCorner) / 2 == 1) {
                y = actualRows - 1 - y;
            }
            auto&& pChild = m_children[i];
            const auto pivot = pChild->GetPivot();
            auto translation = pChild->GetTranslation();
            translation.x = start.x + (m_cellSize.x + m_spacing.x) * x + m_cellSize.x * pivot.x;
            translation.y = -start.y - (m_cellSize.y + m_spacing.y) * y - m_cellSize.y * (1.f - pivot.y);
            pChild->SetTranslation(translation);
        }
    }
}
