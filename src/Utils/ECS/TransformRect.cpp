//
// Created by Monika on 01.08.2022.
//

#include <Utils/ECS/TransformRect.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/ECS/TransformUtils.h>

#include <Codegen/TransformRect.generated.hpp>

namespace SR_UTILS_NS {
    TransformRect::TransformRect()
        : Transform()
    { }

    bool TransformRect::Load(IDeserializer& deserializer) {
        SR_TRACY_ZONE;
        m_loadingLayout = true;
        const bool loaded = Super::Load(deserializer);
        m_loadingLayout = false;
        if (!loaded) {
            return false;
        }
        m_size = GetSizeDelta();
        const auto position = GetAnchoredPosition();
        m_translation.x = position.x;
        m_translation.y = position.y;
        UpdateTree();
        return true;
    }

    void TransformRect::SetAnchoredPosition(const SR_MATH_NS::FVector2& position) {
        SetTranslation(SR_MATH_NS::FVector3(position.x, position.y, m_translation.z));
    }

    void TransformRect::SetVisualOffset(const SR_MATH_NS::FVector2& offset) {
        if (m_visualOffset != offset) {
            m_visualOffset = offset;
            UpdateTreeRecursive();
        }
    }

    void TransformRect::SetTranslation(const SR_MATH_NS::FVector3& translation) {
        if (translation == m_translation) {
            return;
        }

        m_translation = translation;

        if (!m_loadingLayout) {
            const auto delta = GetSizeDelta();
            if (!AreAnchorsTogetherX()) {
                m_offsetMin.x = translation.x - delta.x * m_pivot.x;
                m_offsetMax.x = translation.x + delta.x * (1.f - m_pivot.x);
            }
            if (!AreAnchorsTogetherY()) {
                m_offsetMin.y = translation.y - delta.y * m_pivot.y;
                m_offsetMax.y = translation.y + delta.y * (1.f - m_pivot.y);
            }
        }

        UpdateTree();
    }

    void TransformRect::SetTranslationAndRotation(const SR_MATH_NS::FVector3& translation, const SR_MATH_NS::FVector3& euler) {
        SetTranslation(translation);
        m_quaternion = euler.Radians().ToQuat();
        m_rotation = m_quaternion.EulerAngle();

        UpdateTree();
    }

    void TransformRect::SetRotation(const SR_MATH_NS::FVector3& euler) {
        m_quaternion = euler.Radians().ToQuat();
        m_rotation = m_quaternion.EulerAngle();
        UpdateTree();
    }

    void TransformRect::SetRotation(const SR_MATH_NS::Quaternion& quaternion) {
        m_quaternion = quaternion;
        m_rotation = m_quaternion.EulerAngle();
        UpdateTree();
    }

    void TransformRect::SetSize(const SR_MATH_NS::FVector2& size) {
        if (!m_loadingLayout && size == GetSizeDelta()) {
            return;
        }
        const auto position = GetAnchoredPosition();
        m_size = size;
        if (!m_loadingLayout) {
            if (!AreAnchorsTogetherX()) {
                m_offsetMin.x = position.x - size.x * m_pivot.x;
                m_offsetMax.x = position.x + size.x * (1.f - m_pivot.x);
            }
            if (!AreAnchorsTogetherY()) {
                m_offsetMin.y = position.y - size.y * m_pivot.y;
                m_offsetMax.y = position.y + size.y * (1.f - m_pivot.y);
            }
        }
        UpdateTree();
    }

    void TransformRect::SetSizeWithCurrentAnchors(const bool horizontal, const float_t size) {
        const auto rect = GetLayoutRect();
        auto newSize = GetSize();
        auto offsetMin = GetOffsetMin();
        auto offsetMax = GetOffsetMax();
        if (horizontal) {
            if (AreAnchorsTogetherX()) {
                newSize.x = size;
                SetSize(newSize);
            }
            else {
                const float_t delta = size - rect.Width();
                offsetMin.x -= delta * m_pivot.x;
                offsetMax.x += delta * (1.f - m_pivot.x);
                SetOffsetMin(offsetMin);
                SetOffsetMax(offsetMax);
            }
        }
        else {
            if (AreAnchorsTogetherY()) {
                newSize.y = size;
                SetSize(newSize);
            }
            else {
                const float_t delta = size - rect.Height();
                offsetMin.y -= delta * m_pivot.y;
                offsetMax.y += delta * (1.f - m_pivot.y);
                SetOffsetMin(offsetMin);
                SetOffsetMax(offsetMax);
            }
        }
    }

    void TransformRect::SetScale(const SR_MATH_NS::FVector3& rawScale) {
        SR_MATH_NS::FVector3 scale = rawScale;

        if (scale.ContainsNaN()) {
            SR_WARN("TransformRect::SetScale() : scale contains NaN! Reset...");
            scale = SR_MATH_NS::FVector3::One();
        }

        if (scale == m_scale) {
            return;
        }
        m_scale = scale;

        UpdateTree();
    }

    void TransformRect::Translate(const SR_MATH_NS::FVector3& translation) {
        SetTranslation(m_translation + GetQuaternion() * translation);
    }

    void TransformRect::Rotate(const SR_MATH_NS::Quaternion& quaternion) {
        SetRotation((m_quaternion * quaternion).EulerAngle());
    }

    void TransformRect::Scale(const SR_MATH_NS::FVector3& scale) {
        if (scale.HasZero()) {
            return;
        }
        SetScale(m_scale * scale);
    }

    void TransformRect::SetSkew(const SR_MATH_NS::FVector3& rawSkew) {
        SR_MATH_NS::FVector3 skew = rawSkew;

        if (skew.ContainsNaN()) {
            SR_WARN("TransformRect::SetSkew() : skew contains NaN! Reset...");
            skew = Math::FVector3::One();
        }

        m_skew = skew;

        UpdateTree();
    }

    const SR_MATH_NS::FRect& TransformRect::GetLayoutRect() const {
        SR_MAYBE_UNUSED_VAR GetMatrix();
        return m_layoutRect;
    }

    const SR_MATH_NS::FRect& TransformRect::GetScaledLayoutRect() const {
        SR_MAYBE_UNUSED_VAR GetMatrix();
        return m_scaledLayoutRect;
    }

    SR_MATH_NS::FRect TransformRect::GetTransformedLayoutRect() const {
        const SR_MATH_NS::FRect& layoutRect = GetLayoutRect();
        const SR_MATH_NS::Matrix4x4& matrix = GetMatrix();

        const SR_MATH_NS::FVector4 bottomLeft = matrix.TransformPoint({ layoutRect.Left(), layoutRect.Bottom(), 0.f });
        const SR_MATH_NS::FVector4 bottomRight = matrix.TransformPoint({ layoutRect.Right(), layoutRect.Bottom(), 0.f });
        const SR_MATH_NS::FVector4 topLeft = matrix.TransformPoint({ layoutRect.Left(), layoutRect.Top(), 0.f });
        const SR_MATH_NS::FVector4 topRight = matrix.TransformPoint({ layoutRect.Right(), layoutRect.Top(), 0.f });

        const float_t minX = std::min(std::min(bottomLeft.x, bottomRight.x), std::min(topLeft.x, topRight.x));
        const float_t minY = std::min(std::min(bottomLeft.y, bottomRight.y), std::min(topLeft.y, topRight.y));
        const float_t maxX = std::max(std::max(bottomLeft.x, bottomRight.x), std::max(topLeft.x, topRight.x));
        const float_t maxY = std::max(std::max(bottomLeft.y, bottomRight.y), std::max(topLeft.y, topRight.y));

        return SR_MATH_NS::FRect(minX, minY, maxX - minX, maxY - minY);
    }

    const SR_MATH_NS::Matrix4x4& TransformRect::GetMatrix() const {
        SR_TRACY_ZONE;

        if (IsDirty()) {
            /*==============================================================
                0. Parent layout rect (или Canvas root)
            ==============================================================*/
            const Transform* pParentTransform = GetParentTransform();
            const TransformRect* pParent =
                (pParentTransform && pParentTransform->GetMeta()->IsSameOrInherited(TransformRect::GetClassStaticName()))
                ? static_cast<const TransformRect*>(pParentTransform)
                : nullptr;

            const SR_MATH_NS::FRect parentRect = pParent
                ? pParent->GetLayoutRect()
                : SR_MATH_NS::FRect(
                    { 0.f, 0.f },
                    { m_canvasSize.x, m_canvasSize.y } // ЯВНО: корень = Canvas
                );

            const SR_MATH_NS::FVector2 parentMin = parentRect.Min();
            const SR_MATH_NS::FVector2 parentMax = parentRect.Max();

            /*==============================================================
                1. Anchor rect (в координатах родителя)
            ==============================================================*/

            const SR_MATH_NS::FVector2 anchorMinPos =
                SR_MATH_NS::FVector2::Lerp(parentMin, parentMax, m_anchors.min);

            const SR_MATH_NS::FVector2 anchorMaxPos =
                SR_MATH_NS::FVector2::Lerp(parentMin, parentMax, m_anchors.max);

            /*==============================================================
                2. Определение режима якорей и расчет layoutRect
                В Unity каждая ось обрабатывается независимо:
                - Если anchorMin.x == anchorMax.x → Position X + Width
                - Если anchorMin.y == anchorMax.y → Position Y + Height
                - Если anchorMin.x != anchorMax.x → Left + Right offsets
                - Если anchorMin.y != anchorMax.y → Top + Bottom offsets
            ==============================================================*/

            const bool anchorsTogetherX = AreAnchorsTogetherX();
            const bool anchorsTogetherY = AreAnchorsTogetherY();

            float_t left = 0.f, right = 0.f, bottom = 0.f, top = 0.f;

            float_t width = 0.f, height = 0.f;
            SR_MATH_NS::FVector2 pivotPos;

            // Обработка по оси X
            if (anchorsTogetherX) {
                // Якоря вместе по X: используем Position X и Width
                const float_t anchorX = anchorMinPos.x; // min.x == max.x
                width = m_size.x; // sizeDelta.x
                pivotPos.x = anchorX + m_translation.x; // anchorPos + anchoredPosition
            }
            else {
                // Якоря врозь по X: используем Left и Right offsets
                left = anchorMinPos.x + m_offsetMin.x;
                right = anchorMaxPos.x + m_offsetMax.x;
                width = right - left;
                pivotPos.x = left + width * m_pivot.x;
            }

            // Обработка по оси Y
            if (anchorsTogetherY) {
                // Якоря вместе по Y: используем Position Y и Height
                const float_t anchorY = anchorMinPos.y; // min.y == max.y
                height = m_size.y; // sizeDelta.y
                pivotPos.y = anchorY + m_translation.y; // anchorPos + anchoredPosition
            }
            else {
                // Якоря врозь по Y: используем Bottom и Top offsets
                bottom = anchorMinPos.y + m_offsetMin.y;
                top = anchorMaxPos.y + m_offsetMax.y;
                height = top - bottom;
                pivotPos.y = bottom + height * m_pivot.y;
            }

            const SR_MATH_NS::FVector2 finalSize = SR_MATH_NS::FVector2(width, height);

            /*==============================================================
                3. localRect (АНАЛОГ RectTransform.rect)
                ❗ БЕЗ позиции, относительно pivot
            ==============================================================*/

            m_localRect.SetMin(-finalSize.x * m_pivot.x, -finalSize.y * m_pivot.y);
            m_localRect.SetMax(m_localRect.Min().x + finalSize.x, m_localRect.Min().y + finalSize.y);

            /*==============================================================
                4. layoutRect (АНАЛОГ GetLocalCorners)
                ❗ с учётом позиции pivot в мировых координатах
            ==============================================================*/

            m_layoutRect.SetMin(
                pivotPos.x + m_localRect.Min().x,
                pivotPos.y + m_localRect.Min().y
            );

            m_layoutRect.SetMax(
                pivotPos.x + m_localRect.Max().x,
                pivotPos.y + m_localRect.Max().y
            );

            /*==============================================================
                5. Render matrix (ПОСЛЕ layout)
            ==============================================================*/

            m_localMatrix =
                SR_MATH_NS::Matrix4x4::FromTranslate({ pivotPos.x + m_visualOffset.x, pivotPos.y + m_visualOffset.y, 0.f }) *
                SR_MATH_NS::Matrix4x4::FromQuaternion(GetQuaternion()) *
                SR_MATH_NS::Matrix4x4::FromScale(GetScale()) *
                SR_MATH_NS::Matrix4x4::FromTranslate({
                   -pivotPos.x,
                   -pivotPos.y,
                   0.f
                });

            {
                SR_MATH_NS::FVector2 globalScale = GetGlobalScale().XY();
                m_scaledLayoutRect.SetMin(pivotPos + (m_layoutRect.Min() - pivotPos) * globalScale);
                m_scaledLayoutRect.SetMax(pivotPos + (m_layoutRect.Max() - pivotPos) * globalScale);
            }

            m_dirtyMatrix = false;

            if (auto&& pTransform = m_gameObject->GetParentTransform()) {
                m_matrix = pTransform->GetMatrix() * m_localMatrix;
            }
            else {
                m_matrix = m_localMatrix;
            }
        }

        return m_matrix;
    }

    void TransformRect::SetGlobalTranslation(const SR_MATH_NS::FVector3& translation) {
        if (auto&& pParent = GetParentTransform()) {
            auto matrix = SR_MATH_NS::Matrix4x4::FromTranslate(translation);
            matrix *= SR_MATH_NS::Matrix4x4::FromScale(m_skew);
            matrix *= SR_MATH_NS::Matrix4x4::FromEulers(m_rotation.Inverse());
            matrix *= SR_MATH_NS::Matrix4x4::FromScale(m_scale);

            matrix = pParent->GetMatrix().Inverse() * matrix;

            SetTranslation(matrix.GetTranslate());

            UpdateTree();
        }
        else {
            SetTranslation(translation);
        }
    }

    void TransformRect::SetGlobalRotation(const SR_MATH_NS::Quaternion& quaternion) {
        if (auto&& pParent = GetParentTransform()) {
            auto&& matrix = SR_MATH_NS::Matrix4x4::FromScale(SR_MATH_NS::FVector3(1) / m_skew);
            matrix *= SR_MATH_NS::Matrix4x4::FromQuaternion(quaternion);
            matrix *= SR_MATH_NS::Matrix4x4::FromScale(m_scale);

            matrix = pParent->GetMatrix().Inverse() * matrix;

            SetRotation(matrix.GetEulers());
        }
        else {
            auto&& matrix = SR_MATH_NS::Matrix4x4::FromScale(SR_MATH_NS::FVector3(1) / m_skew);
            matrix *= SR_MATH_NS::Matrix4x4::FromQuaternion(quaternion);
            matrix *= SR_MATH_NS::Matrix4x4::FromScale(m_scale);

            SetRotation(matrix.GetEulers());
        }
    }

    int32_t TransformRect::GetPriority() const { /// NOLINT
        SR_TRACY_ZONE;

        if (!m_isDirtyPriority) {
            return m_priority;
        }

        if (m_relativePriority) {
            auto&& pParentTransform = GetParentTransform();
            if (pParentTransform && pParentTransform->GetMeta()->IsSameOrInherited(TransformRect::GetClassStaticName())) {
                m_priority = m_localPriority + static_cast<const TransformRect*>(pParentTransform)->GetPriority();
            }
            else {
                m_priority = m_localPriority;
            }
        }
        else {
            m_priority = m_localPriority;
        }

        m_isDirtyPriority = false;

        return m_priority;
    }

    void TransformRect::SetLocalPriority(int32_t priority) {
        if (m_localPriority == priority) {
            return;
        }
        m_localPriority = priority;
        UpdatePriorityTree();
    }

    void TransformRect::SetRelativePriority(bool relative) {
        if (m_relativePriority == relative) {
            return;
        }
        m_relativePriority = relative;
        UpdatePriorityTree();
    }

    void TransformRect::UpdatePriorityTree() {
        m_isDirtyPriority = true;

        if (!m_gameObject) {
            return;
        }

        m_gameObject->OnPriorityChanged();

        for (auto&& pChild : m_gameObject->GetChildrenRef()) {
            if (auto&& pTransformRect = ExtractTransformAs<TransformRect>(pChild.Get())) {
                pTransformRect->UpdatePriorityTree();
            }
        }
    }

    void TransformRect::OnHierarchyChanged() {
        UpdatePriorityTree();
        Transform::OnHierarchyChanged();
    }

    void TransformRect::SetPivot(const SR_MATH_NS::FVector2& pivot) {
        if (pivot == m_pivot) {
            return;
        }
        const auto position = GetAnchoredPosition();
        const auto size = GetSizeDelta();
        m_pivot = pivot;
        if (!m_loadingLayout) {
            if (!AreAnchorsTogetherX()) {
                m_offsetMin.x = position.x - size.x * pivot.x;
                m_offsetMax.x = position.x + size.x * (1.f - pivot.x);
            }
            if (!AreAnchorsTogetherY()) {
                m_offsetMin.y = position.y - size.y * pivot.y;
                m_offsetMax.y = position.y + size.y * (1.f - pivot.y);
            }
        }
        UpdateTree();
    }

    void TransformRect::SetCanvasSize(const SR_MATH_NS::FVector2& size) {
        if (size == m_canvasSize) {
            return;
        }
        m_canvasSize = size;
        UpdateTree();
    }

    void TransformRect::SetAnchors(const RectAnchors& anchors) {
        if (anchors == m_anchors) {
            return;
        }
        const auto size = GetSizeDelta();
        const auto position = GetAnchoredPosition();
        m_anchors = anchors;
        if (!m_loadingLayout) {
            m_size = size;
            m_offsetMin = position - size * m_pivot;
            m_offsetMax = position + size * (SR_MATH_NS::FVector2::One() - m_pivot);
            m_translation.x = position.x;
            m_translation.y = position.y;
        }
        UpdateTree();
    }

    void TransformRect::SetOffsetMin(const SR_MATH_NS::FVector2& offset) {
        if (!m_loadingLayout && offset == GetOffsetMin()) {
            return;
        }
        if (!m_loadingLayout) {
            const auto delta = offset - GetOffsetMin();
            if (AreAnchorsTogetherX()) {
                m_size.x -= delta.x;
                m_translation.x += delta.x * (1.f - m_pivot.x);
            }
            if (AreAnchorsTogetherY()) {
                m_size.y -= delta.y;
                m_translation.y += delta.y * (1.f - m_pivot.y);
            }
        }
        m_offsetMin = offset;
        if (!m_loadingLayout) {
            const auto position = GetAnchoredPosition();
            m_translation.x = position.x;
            m_translation.y = position.y;
        }
        UpdateTree();
    }

    void TransformRect::SetOffsetMax(const SR_MATH_NS::FVector2& offset) {
        if (!m_loadingLayout && offset == GetOffsetMax()) {
            return;
        }
        if (!m_loadingLayout) {
            const auto delta = offset - GetOffsetMax();
            if (AreAnchorsTogetherX()) {
                m_size.x += delta.x;
                m_translation.x += delta.x * m_pivot.x;
            }
            if (AreAnchorsTogetherY()) {
                m_size.y += delta.y;
                m_translation.y += delta.y * m_pivot.y;
            }
        }
        m_offsetMax = offset;
        if (!m_loadingLayout) {
            const auto position = GetAnchoredPosition();
            m_translation.x = position.x;
            m_translation.y = position.y;
        }
        UpdateTree();
    }

    bool TransformRect::AreAnchorsTogetherX() const {
        return m_anchors.min.x == m_anchors.max.x;
    }

    bool TransformRect::AreAnchorsTogetherY() const {
        return m_anchors.min.y == m_anchors.max.y;
    }

    SR_MATH_NS::FVector2 TransformRect::GetAnchoredPosition() const {
        auto result = m_translation.XY();
        const auto size = GetSizeDelta();
        if (!AreAnchorsTogetherX()) {
            result.x = m_offsetMin.x + size.x * m_pivot.x;
        }
        if (!AreAnchorsTogetherY()) {
            result.y = m_offsetMin.y + size.y * m_pivot.y;
        }
        return result;
    }

    SR_MATH_NS::FVector2 TransformRect::GetSizeDelta() const {
        auto result = m_size;
        if (!AreAnchorsTogetherX()) {
            result.x = m_offsetMax.x - m_offsetMin.x;
        }
        if (!AreAnchorsTogetherY()) {
            result.y = m_offsetMax.y - m_offsetMin.y;
        }
        return result;
    }

    SR_MATH_NS::FVector2 TransformRect::GetOffsetMin() const {
        return GetAnchoredPosition() - GetSizeDelta() * m_pivot;
    }

    SR_MATH_NS::FVector2 TransformRect::GetOffsetMax() const {
        return GetAnchoredPosition() + GetSizeDelta() * (SR_MATH_NS::FVector2::One() - m_pivot);
    }

    SR_MATH_NS::Quaternion TransformRect::GetGlobalRotation() const {
        if (m_dirtyRotation) SR_UNLIKELY_ATTRIBUTE {
            if (auto&& pParent = GetParentTransform()) SR_LIKELY_ATTRIBUTE {
                m_globalRotation = pParent->GetGlobalRotation() * m_quaternion;
            }
            else {
                m_globalRotation = m_quaternion;
            }

            m_dirtyRotation = false;
        }
        return m_globalRotation;
    }

    SR_MATH_NS::FVector3 TransformRect::GetGlobalScale() const {
        if (m_dirtyScale) SR_UNLIKELY_ATTRIBUTE {
            if (auto&& pParent = GetParentTransform()) SR_LIKELY_ATTRIBUTE {
                m_globalScale = pParent->GetGlobalScale() * m_scale;
            }
            else {
                m_globalScale = m_scale;
            }

            m_dirtyScale = false;
        }
        return m_globalScale;
    }

    void TransformRect::SetMaskInfo(const UI::MaskInfo& maskInfo) {
        if (maskInfo == m_localMaskInfo) {
            return;
        }
        m_localMaskInfo = maskInfo;
        UpdateMaskInfoTree();
    }

    void TransformRect::UpdateMaskInfoTree() {
        if (!m_gameObject) {
            return;
        }
        m_maskInfoDirty = true;
        m_gameObject->OnMaskDirty();

        for (auto&& pChild : m_gameObject->GetChildrenRef()) {
            if (auto&& pChildTransform = ExtractTransformAs<TransformRect>(pChild.Get())) {
                pChildTransform->UpdateMaskInfoTree();
            }
        }
    }

    const UI::MaskInfo& TransformRect::GetMaskInfo() const {
        if (m_maskInfoDirty) SR_UNLIKELY_ATTRIBUTE {
            const Transform* pParentTransform = GetParentTransform();
            const TransformRect* pParent =
                (pParentTransform && pParentTransform->GetMeta()->IsSameOrInherited(TransformRect::GetClassStaticName()))
                ? static_cast<const TransformRect*>(pParentTransform)
                : nullptr;

            if (pParent) SR_LIKELY_ATTRIBUTE {
                const auto& parentMaskInfo = pParent->GetMaskInfo();
                m_globalMaskInfo = UI::MaskInfo::Combine(parentMaskInfo, m_localMaskInfo);
            }
            else {
                m_globalMaskInfo = m_localMaskInfo;
            }

            m_maskInfoDirty = false;
        }
        return m_globalMaskInfo;
    }

    std::span<RectAnchors> GetRectAnchorsPresets() {
        static std::array<RectAnchors, 10> presets = {
            RectAnchors::Fill(),
            RectAnchors::TopLeft(),
            RectAnchors::TopCenter(),
            RectAnchors::TopRight(),
            RectAnchors::MiddleLeft(),
            RectAnchors::MiddleCenter(),
            RectAnchors::MiddleRight(),
            RectAnchors::BottomLeft(),
            RectAnchors::BottomCenter(),
            RectAnchors::BottomRight(),
        };
        return presets;
    }
}
