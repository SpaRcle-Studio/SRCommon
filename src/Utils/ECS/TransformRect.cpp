//
// Created by Monika on 01.08.2022.
//

#include <Utils/ECS/TransformRect.h>
#include <Utils/ECS/GameObject.h>

#include <Codegen/TransformRect.generated.hpp>

namespace SR_UTILS_NS {
    TransformRect::TransformRect()
        : Transform()
    { }

    void TransformRect::SetTranslation(const SR_MATH_NS::FVector3& translation) {
        if (translation == m_translation) {
            return;
        }

        m_translation = translation;

        UpdateTree();
    }

    void TransformRect::SetTranslationAndRotation(const SR_MATH_NS::FVector3& translation, const SR_MATH_NS::FVector3& euler) {
        m_translation = translation;
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
        m_size = size;
        UpdateTree();
    }

    void TransformRect::SetScale(const SR_MATH_NS::FVector3& rawScale) {
        SR_MATH_NS::FVector3 scale = rawScale;

        if (scale.ContainsNaN()) {
            SR_WARN("TransformRect::SetScale() : scale contains NaN! Reset...");
            scale = SR_MATH_NS::FVector3::One();
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

    const SR_MATH_NS::Matrix4x4& TransformRect::GetMatrix() const {
        if (IsDirty()) {
            UpdateMatrix();

            if (auto&& pTransform = m_gameObject->GetParentTransform()) {
                m_matrix = pTransform->GetMatrix() * m_localMatrix;
            }
            else {
                m_matrix = m_localMatrix;
            }
        }

        return m_matrix;
    }

    void TransformRect::UpdateMatrix() const {
        SR_TRACY_ZONE;

        /*==============================================================
            0. Parent layout rect (или Canvas root)
        ==============================================================*/

        const TransformRect* pParent =
            (GetParentTransform() && GetParentTransform()->GetMeasurement() == Measurement::Space2D)
            ? static_cast<const TransformRect*>(GetParentTransform())
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

        const SR_MATH_NS::FVector2 anchorSize = anchorMaxPos - anchorMinPos;

        /*==============================================================
            2. Финальный размер (stretch + sizeDelta)
        ==============================================================*/

        const SR_MATH_NS::FVector2 finalSize = anchorSize + m_size;

        /*==============================================================
            3. localRect (АНАЛОГ RectTransform.rect)
            ❗ БЕЗ позиции
        ==============================================================*/

        //const SR_MATH_NS::FVector2 finalSize = m_size;

        m_localRect.SetMin(-finalSize.x * m_pivot.x, -finalSize.y * m_pivot.y);
        m_localRect.SetMax(m_localRect.Min().x + finalSize.x, m_localRect.Min().y + finalSize.y);

        /*==============================================================
            4. layoutRect (АНАЛОГ GetLocalCorners)
            ❗ с учётом anchoredPosition
        ==============================================================*/

        //const SR_MATH_NS::FVector2 pivotPos = anchorMinPos + m_translation.XY();
        //const SR_MATH_NS::FVector2 pivotPos = anchorMinPos + anchorSize * m_pivot + m_translation.XY();
        const SR_MATH_NS::FVector2 pivotPos = anchorMinPos + anchorSize * m_pivot + m_translation.XY();

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

        //const SR_MATH_NS::FVector2 pivotWorldPos =
        //        m_layoutRect.Min() +
        //        m_layoutRect.Size() * m_pivot;

        m_localMatrix =
            SR_MATH_NS::Matrix4x4::FromTranslate({ pivotPos.x, pivotPos.y, 0.f }) *
            SR_MATH_NS::Matrix4x4::FromQuaternion(GetQuaternion()) *
            SR_MATH_NS::Matrix4x4::FromScale(GetScale()) *
            SR_MATH_NS::Matrix4x4::FromTranslate({
               -pivotPos.x,
               -pivotPos.y,
               0.f
            });

        Super::UpdateMatrix();
    }

    void TransformRect::SetGlobalTranslation(const SR_MATH_NS::FVector3& translation) {
        if (auto&& pParent = GetParentTransform()) {
            auto matrix = SR_MATH_NS::Matrix4x4::FromTranslate(translation);
            matrix *= SR_MATH_NS::Matrix4x4::FromScale(m_skew);
            matrix *= SR_MATH_NS::Matrix4x4::FromEulers(m_rotation.Inverse());
            matrix *= SR_MATH_NS::Matrix4x4::FromScale(m_scale);

            matrix = pParent->GetMatrix().Inverse() * matrix;

            m_translation = matrix.GetTranslate();

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

    int32_t TransformRect::GetPriority() { /// NOLINT
        if (!m_isDirtyPriority) {
            return m_priority;
        }

        if (m_relativePriority) {
            if (auto&& pParentTransform = dynamic_cast<TransformRect*>(GetParentTransform())) {
                m_priority = m_localPriority + pParentTransform->GetPriority();
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
            if (auto&& pGameObject = pChild.DynamicCast<GameObject>()) {
                if (auto&& pTransformRect = const_cast<TransformRect*>(dynamic_cast<const TransformRect*>(pGameObject->GetTransform().Get()))) {
                    pTransformRect->UpdatePriorityTree();
                }
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
        m_pivot = pivot;
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
        m_anchors = anchors;
        UpdateTree();
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
}
