//
// Created by Codex on 15.11.2026.
//

#include <Utils/UI/LayoutGroup.h>
#include <Utils/ECS/TransformRect.h>
#include <Utils/ECS/TransformUtils.h>
#include <Utils/ECS/GameObject.h>

#include <Codegen/LayoutGroup.generated.hpp>

namespace SR_UTILS_NS {
    void LayoutGroup::SetDirection(const LayoutGroupDirection value) {
        if (m_direction == value) {
            return;
        }
        m_direction = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetChildAlignHorizontal(const LayoutGroupChildAlighHorizontal value) {
        if (m_childAlignHorizontal == value) {
            return;
        }
        m_childAlignHorizontal = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetChildAlignVertical(const LayoutGroupChildAlighVertical value) {
        if (m_childAlignVertical == value) {
            return;
        }
        m_childAlignVertical = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetSpacing(const float_t value) {
        if (m_spacing == value) {
            return;
        }
        m_spacing = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetPaddingTop(const float_t value) {
        if (m_paddingTop == value) {
            return;
        }
        m_paddingTop = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetPaddingBottom(const float_t value) {
        if (m_paddingBottom == value) {
            return;
        }
        m_paddingBottom = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetPaddingLeft(const float_t value) {
        if (m_paddingLeft == value) {
            return;
        }
        m_paddingLeft = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetPaddingRight(const float_t value) {
        if (m_paddingRight == value) {
            return;
        }
        m_paddingRight = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetControlChildWidth(const bool value) {
        if (m_controlChildWidth == value) {
            return;
        }
        m_controlChildWidth = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetControlChildHeight(const bool value) {
        if (m_controlChildHeight == value) {
            return;
        }
        m_controlChildHeight = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetChildForceExpandWidth(const bool value) {
        if (m_childForceExpandWidth == value) {
            return;
        }
        m_childForceExpandWidth = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetChildForceExpandHeight(const bool value) {
        if (m_childForceExpandHeight == value) {
            return;
        }
        m_childForceExpandHeight = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetChildScaleWidth(const bool value) {
        if (m_childScaleWidth == value) {
            return;
        }
        m_childScaleWidth = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetChildScaleHeight(const bool value) {
        if (m_childScaleHeight == value) {
            return;
        }
        m_childScaleHeight = value;
        SetLayoutDirty();
    }

    void LayoutGroup::SetReverseArrangement(const bool value) {
        if (m_reverseArrangement == value) {
            return;
        }
        m_reverseArrangement = value;
        SetLayoutDirty();
    }

    void LayoutGroup::CalculateLayoutInput(const bool horizontal) {
        SR_TRACY_ZONE;
        auto&& pObject = GetGameObject();
        const uint32_t axis = horizontal ? 0 : 1;
        const bool mainAxis = horizontal == (m_direction == LayoutGroupDirection::Horizontal);
        const bool control = horizontal ? m_controlChildWidth : m_controlChildHeight;
        const bool expand = horizontal ? m_childForceExpandWidth : m_childForceExpandHeight;
        const bool useScale = horizontal ? m_childScaleWidth : m_childScaleHeight;
        const float_t padding = horizontal ? m_paddingLeft + m_paddingRight : m_paddingTop + m_paddingBottom;
        auto&& total = m_metrics[axis];
        total = { padding, padding, 0.f };
        if (!pObject) {
            return;
        }
        if (horizontal) {
            m_childrenCache.clear();
            for (auto&& pChildSO : pObject->GetChildrenRef()) {
                if (pChildSO->GetMeta() != GameObject::GetMetaStatic()) {
                    continue;
                }
                auto&& pChild = static_cast<GameObject*>(pChildSO.Get());
                if (!pChild->IsActive() || LayoutUtility::IsIgnored(*pChild)) {
                    continue;
                }
                if (auto&& pRect = ExtractTransformAs<TransformRect>(pChild)) {
                    LayoutChild child;
                    child.pTransform = pRect;
                    m_childrenCache.emplace_back(child);
                }
            }
        }
        for (auto&& child : m_childrenCache) {
            auto&& pChildObject = child.pTransform->GetGameObject();
            auto metrics = control ? LayoutUtility::GetMetrics(*pChildObject, horizontal) : LayoutUtility::Metrics();
            if (!control) {
                const auto size = child.pTransform->GetSizeDelta();
                metrics.minimum = metrics.preferred = horizontal ? size.x : size.y;
            }
            if (expand) {
                metrics.flexible = SR_MAX(metrics.flexible, 1.f);
            }
            auto&& cache = horizontal ? child.main : child.cross;
            cache.min = metrics.minimum;
            cache.preferred = metrics.preferred;
            cache.flexible = metrics.flexible;
            const auto scale = child.pTransform->GetScale();
            const float_t factor = useScale ? (horizontal ? scale.x : scale.y) : 1.f;
            if (mainAxis) {
                total.minimum += metrics.minimum * factor + m_spacing;
                total.preferred += metrics.preferred * factor + m_spacing;
                total.flexible += metrics.flexible * factor;
            }
            else {
                total.minimum = SR_MAX(total.minimum, padding + metrics.minimum * factor);
                total.preferred = SR_MAX(total.preferred, padding + metrics.preferred * factor);
                total.flexible = SR_MAX(total.flexible, metrics.flexible * factor);
            }
        }
        if (mainAxis && !m_childrenCache.empty()) {
            total.minimum -= m_spacing;
            total.preferred -= m_spacing;
        }
        total.preferred = SR_MAX(total.minimum, total.preferred);
    }

    void LayoutGroup::SetLayout(const bool horizontal) {
        SR_TRACY_ZONE;
        auto&& pRect = GetTransformAs<TransformRect>();
        if (!pRect) {
            return;
        }
        const auto rect = pRect->GetLayoutRect();
        const float_t size = horizontal ? rect.Width() : rect.Height();
        const bool mainAxis = horizontal == (m_direction == LayoutGroupDirection::Horizontal);
        const bool control = horizontal ? m_controlChildWidth : m_controlChildHeight;
        const bool useScale = horizontal ? m_childScaleWidth : m_childScaleHeight;
        const float_t leading = horizontal ? m_paddingLeft : m_paddingTop;
        const float_t padding = horizontal ? m_paddingLeft + m_paddingRight : m_paddingTop + m_paddingBottom;
        const float_t alignment = horizontal ? static_cast<float_t>(m_childAlignHorizontal) * 0.5f
                                            : static_cast<float_t>(m_childAlignVertical) * 0.5f;
        const auto total = m_metrics[horizontal ? 0 : 1];
        const float_t surplus = size - total.preferred;
        const float_t multiplier = surplus > 0.f && total.flexible > 0.f ? surplus / total.flexible : 0.f;
        const float_t lerp = total.minimum == total.preferred ? 0.f
            : SR_CLAMP((size - total.minimum) / (total.preferred - total.minimum), 0.f, 1.f);
        float_t cursor = leading;
        if (surplus > 0.f && total.flexible == 0.f) {
            cursor += surplus * alignment;
        }

        for (uint64_t i = 0; i < m_childrenCache.size(); ++i) {
            auto&& child = m_childrenCache[m_reverseArrangement ? m_childrenCache.size() - 1 - i : i];
            auto&& cache = horizontal ? child.main : child.cross;
            auto&& pChild = child.pTransform;
            const auto scale = pChild->GetScale();
            const float_t factor = useScale ? (horizontal ? scale.x : scale.y) : 1.f;
            const float_t cell = mainAxis ? cache.min + (cache.preferred - cache.min) * lerp + cache.flexible * multiplier
                : SR_CLAMP(size - padding, cache.min, cache.flexible > 0.f ? size : cache.preferred);
            float_t position = mainAxis ? cursor : leading + (size - padding - cell * factor) * alignment;

            auto anchors = pChild->GetAnchors();
            if (horizontal) {
                anchors.min.x = anchors.max.x = 0.f;
            }
            else {
                anchors.min.y = anchors.max.y = 1.f;
            }
            pChild->SetAnchors(anchors);
            auto childSize = pChild->GetSizeDelta();
            if (control) {
                if (horizontal) {
                    childSize.x = cell;
                }
                else {
                    childSize.y = cell;
                }
                pChild->SetSize(childSize);
            }
            else {
                position += (cell - (horizontal ? childSize.x : childSize.y)) * alignment;
            }
            const auto pivot = pChild->GetPivot();
            auto translation = pChild->GetTranslation();
            if (horizontal) {
                translation.x = position + childSize.x * pivot.x * factor;
            }
            else {
                translation.y = -position - childSize.y * (1.f - pivot.y) * factor;
            }
            pChild->SetTranslation(translation);
            if (mainAxis) {
                cursor += cell * factor + m_spacing;
            }
        }
    }
}
