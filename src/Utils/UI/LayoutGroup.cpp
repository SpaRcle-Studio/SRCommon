#include <Utils/UI/LayoutGroup.h>
#include <Utils/UI/ILayoutElement.h>
#include <Utils/ECS/TransformRect.h>
#include <Utils/ECS/TransformUtils.h>
#include <Utils/ECS/GameObject.h>

#include <Codegen/LayoutGroup.generated.hpp>

namespace SR_UTILS_NS {
    namespace {
        struct LayoutMetric { float value = -1.f; int32_t priority = INT32_MIN; };

        void SelectMetric(LayoutMetric& metric, const float value, const int32_t priority) {
            if (value < 0.f || priority < metric.priority) return;
            if (priority > metric.priority || value > metric.value) {
                metric.value = value;
                metric.priority = priority;
            }
        }

        bool IsLayoutIgnored(const GameObject& child) {
            for (auto&& pComponent : child.GetComponents()) {
                if (!pComponent->IsActive()) continue;
                auto&& pLayoutElement = dynamic_cast<const ILayoutElement*>(pComponent.Get());
                if (pLayoutElement && pLayoutElement->IsLayoutIgnored()) return true;
            }
            return false;
        }

        void ResolveLayoutSize(const GameObject& child, const TransformRect& transform, const bool horizontal, const bool controlSize, const bool forceExpand, LayoutGroup::LayoutSize& size) {
            const SR_MATH_NS::FRect& layoutRect = transform.GetLayoutRect();
            const float current = horizontal ? layoutRect.Width() : layoutRect.Height();
            if (!controlSize) {
                size.min = current;
                size.preferred = current;
                size.result = current;
                return;
            }

            LayoutMetric minimum;
            LayoutMetric preferred;
            LayoutMetric flexible;
            for (auto&& pComponent : child.GetComponents()) {
                if (!pComponent->IsActive()) continue;
                auto&& pLayoutElement = dynamic_cast<const ILayoutElement*>(pComponent.Get());
                if (!pLayoutElement) continue;
                const int32_t priority = pLayoutElement->GetLayoutPriority();
                SelectMetric(minimum, horizontal ? pLayoutElement->GetMinWidth() : pLayoutElement->GetMinHeight(), priority);
                SelectMetric(preferred, horizontal ? pLayoutElement->GetPreferredWidth() : pLayoutElement->GetPreferredHeight(), priority);
                SelectMetric(flexible, horizontal ? pLayoutElement->GetFlexibleWidth() : pLayoutElement->GetFlexibleHeight(), priority);
            }

            size.flexible = flexible.value < 0.f ? 0.f : flexible.value;
            if (forceExpand) size.flexible = SR_MAX(size.flexible, 1.f);
            size.min = minimum.value < 0.f ? (size.flexible > 0.f ? 0.f : current) : minimum.value;
            size.preferred = preferred.value < 0.f ? current : preferred.value;
            size.preferred = SR_MAX(size.min, size.preferred);
            size.result = size.preferred;
        }

        void ApplyLayoutRect(TransformRect& transform, const SR_MATH_NS::FRect& parentRect, const SR_MATH_NS::FRect& targetRect) {
            const RectAnchors& anchors = transform.GetAnchors();
            const SR_MATH_NS::FVector2 anchorMin = SR_MATH_NS::FVector2::Lerp(parentRect.Min(), parentRect.Max(), anchors.min);
            const SR_MATH_NS::FVector2 anchorMax = SR_MATH_NS::FVector2::Lerp(parentRect.Min(), parentRect.Max(), anchors.max);
            const SR_MATH_NS::FVector2 targetSize(targetRect.Width(), targetRect.Height());
            const SR_MATH_NS::FVector2 pivot = transform.GetPivot();
            SR_MATH_NS::FVector2 size = transform.GetSize();
            SR_MATH_NS::FVector3 translation = transform.GetTranslation();
            SR_MATH_NS::FVector2 offsetMin = transform.GetOffsetMin();
            SR_MATH_NS::FVector2 offsetMax = transform.GetOffsetMax();

            if (transform.AreAnchorsTogetherX()) {
                size.x = targetSize.x;
                translation.x = targetRect.Min().x + targetSize.x * pivot.x - anchorMin.x;
            }
            else {
                offsetMin.x = targetRect.Min().x - anchorMin.x;
                offsetMax.x = anchorMax.x - targetRect.Max().x;
            }

            if (transform.AreAnchorsTogetherY()) {
                size.y = targetSize.y;
                translation.y = targetRect.Min().y + targetSize.y * pivot.y - anchorMin.y;
            }
            else {
                offsetMin.y = targetRect.Min().y - anchorMin.y;
                offsetMax.y = anchorMax.y - targetRect.Max().y;
            }

            transform.SetSize(size);
            transform.SetTranslation(translation);
            transform.SetOffsetMin(offsetMin);
            transform.SetOffsetMax(offsetMax);
        }
    }

    void LayoutGroup::SetDirection(const LayoutGroupDirection value) { if (m_direction != value) { m_direction = value; SetLayoutDirty(); } }
    void LayoutGroup::SetChildAlignHorizontal(const LayoutGroupChildAlighHorizontal value) { if (m_childAlignHorizontal != value) { m_childAlignHorizontal = value; SetLayoutDirty(); } }
    void LayoutGroup::SetChildAlignVertical(const LayoutGroupChildAlighVertical value) { if (m_childAlignVertical != value) { m_childAlignVertical = value; SetLayoutDirty(); } }
    void LayoutGroup::SetSpacing(const float_t value) { if (m_spacing != value) { m_spacing = value; SetLayoutDirty(); } }
    void LayoutGroup::SetPaddingTop(const float_t value) { if (m_paddingTop != value) { m_paddingTop = value; SetLayoutDirty(); } }
    void LayoutGroup::SetPaddingBottom(const float_t value) { if (m_paddingBottom != value) { m_paddingBottom = value; SetLayoutDirty(); } }
    void LayoutGroup::SetPaddingLeft(const float_t value) { if (m_paddingLeft != value) { m_paddingLeft = value; SetLayoutDirty(); } }
    void LayoutGroup::SetPaddingRight(const float_t value) { if (m_paddingRight != value) { m_paddingRight = value; SetLayoutDirty(); } }
    void LayoutGroup::SetControlChildWidth(const bool value) { if (m_controlChildWidth != value) { m_controlChildWidth = value; SetLayoutDirty(); } }
    void LayoutGroup::SetControlChildHeight(const bool value) { if (m_controlChildHeight != value) { m_controlChildHeight = value; SetLayoutDirty(); } }
    void LayoutGroup::SetChildForceExpandWidth(const bool value) { if (m_childForceExpandWidth != value) { m_childForceExpandWidth = value; SetLayoutDirty(); } }
    void LayoutGroup::SetChildForceExpandHeight(const bool value) { if (m_childForceExpandHeight != value) { m_childForceExpandHeight = value; SetLayoutDirty(); } }

    void LayoutGroup::Update(float_t dt) {
        SR_TRACY_ZONE;
        auto&& pTransform = GetTransformAs<TransformRect>();
        auto&& pGameObject = GetGameObject();
        if (!pGameObject || !pTransform) { Super::Update(dt); return; }

        const SR_MATH_NS::FRect layoutRect = pTransform->GetLayoutRect();
        const SR_MATH_NS::FVector2 layoutSize(layoutRect.Width(), layoutRect.Height());
        const uint32_t childrenCount = static_cast<uint32_t>(pGameObject->GetChildrenRef().size());
        uint32_t activeChildrenCount = 0;
        uint64_t activeChildrenHash = 0;
        SR_MATH_NS::FVector2 childrenSize = SR_MATH_NS::FVector2::Zero();
        for (auto&& pChild : pGameObject->GetChildrenRef()) {
            if (!pChild->IsActive()) continue;
            ++activeChildrenCount;
            activeChildrenHash = activeChildrenHash * 31 + reinterpret_cast<uintptr_t>(pChild.Get());
            if (auto&& pChildTransform = ExtractTransformAs<TransformRect>(pChild.Get())) {
                childrenSize += pChildTransform->GetSize();
            }
        }
        if (layoutSize != m_lastLayoutSize || childrenSize != m_lastChildrenSize || childrenCount != m_lastChildrenCount
            || activeChildrenCount != m_lastActiveChildrenCount || activeChildrenHash != m_lastActiveChildrenHash) {
            m_layoutDirty = true;
        }
        if (!m_layoutDirty) { Super::Update(dt); return; }

        m_layoutDirty = false;
        m_lastLayoutSize = layoutSize;
        m_lastChildrenSize = childrenSize;
        m_lastChildrenCount = childrenCount;
        m_lastActiveChildrenCount = activeChildrenCount;
        m_lastActiveChildrenHash = activeChildrenHash;

        const bool horizontal = m_direction == LayoutGroupDirection::Horizontal;
        const float mainContentSize = horizontal ? layoutSize.x - m_paddingLeft - m_paddingRight : layoutSize.y - m_paddingTop - m_paddingBottom;
        const float crossContentSize = horizontal ? layoutSize.y - m_paddingTop - m_paddingBottom : layoutSize.x - m_paddingLeft - m_paddingRight;
        const bool controlMain = horizontal ? m_controlChildWidth : m_controlChildHeight;
        const bool controlCross = horizontal ? m_controlChildHeight : m_controlChildWidth;
        const bool forceExpandMain = horizontal ? m_childForceExpandWidth : m_childForceExpandHeight;
        const bool forceExpandCross = horizontal ? m_childForceExpandHeight : m_childForceExpandWidth;
        m_childrenCache.clear();
        float totalMin = 0.f;
        float totalPreferred = 0.f;
        float totalFlexible = 0.f;
        for (auto&& pChildSO : pGameObject->GetChildrenRef()) {
            if (!pChildSO->IsActive()) continue;
            if (!pChildSO->GetMeta()->IsSameOrInherited(GameObject::GetClassStaticName())) continue;

            auto&& pChild = pChildSO.StaticCast<GameObject>();

            TransformRect* pChildTransform = ExtractTransformAs<TransformRect>(pChild.Get());
            if (!pChildTransform || IsLayoutIgnored(*pChild)) continue;

            LayoutChild child;
            child.pTransform = pChildTransform;
            ResolveLayoutSize(*pChild, *pChildTransform, horizontal, controlMain, forceExpandMain, child.main);
            ResolveLayoutSize(*pChild, *pChildTransform, !horizontal, controlCross, forceExpandCross, child.cross);
            totalMin += child.main.min;
            totalPreferred += child.main.preferred;
            totalFlexible += child.main.flexible;
            m_childrenCache.emplace_back(child);
        }
        if (m_childrenCache.empty()) { Super::Update(dt); return; }

        const float spacing = m_spacing * static_cast<float>(m_childrenCache.size() - 1);
        const float available = SR_MAX(0.f, mainContentSize - spacing);
        if (available < totalPreferred && totalPreferred > totalMin) {
            const float factor = SR_MAX(0.f, (available - totalMin) / (totalPreferred - totalMin));
            for (LayoutChild& child : m_childrenCache) child.main.result = child.main.min + (child.main.preferred - child.main.min) * factor;
        }
        else if (available > totalPreferred && totalFlexible > 0.f) {
            const float extra = available - totalPreferred;
            for (LayoutChild& child : m_childrenCache) child.main.result = child.main.preferred + extra * child.main.flexible / totalFlexible;
        }

        float occupied = spacing;
        for (const LayoutChild& child : m_childrenCache) occupied += child.main.result;
        float cursor = horizontal ? m_paddingLeft : -m_paddingTop;
        if (horizontal) {
            if (m_childAlignHorizontal == LayoutGroupChildAlighHorizontal::Center) cursor += (mainContentSize - occupied) * 0.5f;
            else if (m_childAlignHorizontal == LayoutGroupChildAlighHorizontal::Right) cursor += mainContentSize - occupied;
        }
        else {
            if (m_childAlignVertical == LayoutGroupChildAlighVertical::Middle) cursor -= (mainContentSize - occupied) * 0.5f;
            else if (m_childAlignVertical == LayoutGroupChildAlighVertical::Lower) cursor -= mainContentSize - occupied;
        }

        for (LayoutChild& child : m_childrenCache) {
            if (controlCross) {
                if (crossContentSize < child.cross.preferred) {
                    child.cross.result = SR_MAX(child.cross.min, SR_MAX(0.f, crossContentSize));
                }
                else if (crossContentSize > child.cross.preferred && child.cross.flexible > 0.f) {
                    child.cross.result = SR_MAX(0.f, crossContentSize);
                }
            }

            const SR_MATH_NS::FRect& childLayoutRect = child.pTransform->GetLayoutRect();
            SR_MATH_NS::FVector2 childSize(childLayoutRect.Width(), childLayoutRect.Height());
            if (horizontal) {
                if (controlMain) childSize.x = SR_MAX(0.f, child.main.result);
                if (controlCross) childSize.y = SR_MAX(0.f, child.cross.result);
            }
            else {
                if (controlCross) childSize.x = SR_MAX(0.f, child.cross.result);
                if (controlMain) childSize.y = SR_MAX(0.f, child.main.result);
            }
            float cross = 0.f;
            SR_MATH_NS::FVector2 childMin;
            if (horizontal) {
                if (m_childAlignVertical == LayoutGroupChildAlighVertical::Upper) cross = -m_paddingTop;
                else if (m_childAlignVertical == LayoutGroupChildAlighVertical::Middle) cross = -(m_paddingTop + (layoutSize.y - m_paddingTop - m_paddingBottom - childSize.y) * 0.5f);
                else cross = -(layoutSize.y - m_paddingBottom - childSize.y);
                childMin = { layoutRect.Min().x + cursor, layoutRect.Max().y + cross - childSize.y };
                cursor += child.main.result + m_spacing;
            }
            else {
                if (m_childAlignHorizontal == LayoutGroupChildAlighHorizontal::Left) cross = m_paddingLeft;
                else if (m_childAlignHorizontal == LayoutGroupChildAlighHorizontal::Center) cross = m_paddingLeft + (layoutSize.x - m_paddingLeft - m_paddingRight - childSize.x) * 0.5f;
                else cross = layoutSize.x - m_paddingRight - childSize.x;
                childMin = { layoutRect.Min().x + cross, layoutRect.Max().y + cursor - childSize.y };
                cursor -= child.main.result + m_spacing;
            }
            ApplyLayoutRect(*child.pTransform, layoutRect, SR_MATH_NS::FRect(childMin, childSize));
        }

        m_lastChildrenSize = SR_MATH_NS::FVector2::Zero();
        for (auto&& pChild : pGameObject->GetChildrenRef()) {
            if (!pChild->IsActive()) continue;
            if (auto&& pChildTransform = ExtractTransformAs<TransformRect>(pChild.Get())) {
                m_lastChildrenSize += pChildTransform->GetSize();
            }
        }
        Super::Update(dt);
    }
}
