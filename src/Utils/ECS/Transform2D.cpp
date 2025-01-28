//
// Created by Monika on 01.08.2022.
//

#include <Utils/ECS/Transform2D.h>

namespace SR_UTILS_NS {
    Transform2D::Transform2D()
        : Transform()
    {
        m_modifiers.reserve(8);
    }

    void Transform2D::SetTranslation(const SR_MATH_NS::FVector3& translation) {
        if (translation == m_translation) {
            return;
        }

        m_translation = translation;

        UpdateTree();
    }

    void Transform2D::SetTranslationAndRotation(const SR_MATH_NS::FVector3& translation, const SR_MATH_NS::FVector3& euler) {
        m_translation = translation;
        m_quaternion = euler.Radians().ToQuat();
        m_rotation = m_quaternion.EulerAngle();

        UpdateTree();
    }

    void Transform2D::SetRotation(const SR_MATH_NS::FVector3& euler) {
        m_quaternion = euler.Radians().ToQuat();
        m_rotation = m_quaternion.EulerAngle();
        UpdateTree();
    }

    void Transform2D::SetScale(const SR_MATH_NS::FVector3& rawScale) {
        SR_MATH_NS::FVector3 scale = rawScale;

        if (scale.ContainsNaN()) {
            SR_WARN("Transform2D::SetScale() : scale contains NaN! Reset...");
            scale = SR_MATH_NS::FVector3::One();
        }

        m_scale = scale;

        UpdateTree();
    }

    void Transform2D::Translate(const SR_MATH_NS::FVector3& translation) {
        SetTranslation(m_translation + GetQuaternion() * translation);
    }

    void Transform2D::Rotate(const SR_MATH_NS::Quaternion& quaternion) {
        SetRotation((m_quaternion * quaternion).EulerAngle());
    }

    void Transform2D::Scale(const SR_MATH_NS::FVector3& scale) {
        if (scale.HasZero()) {
            return;
        }
        SetScale(m_scale * scale);
    }

    void Transform2D::SetSkew(const SR_MATH_NS::FVector3& rawSkew) {
        SR_MATH_NS::FVector3 skew = rawSkew;

        if (skew.ContainsNaN()) {
            SR_WARN("Transform2D::SetSkew() : skew contains NaN! Reset...");
            skew = Math::FVector3::One();
        }

        m_skew = skew;

        UpdateTree();
    }

    const SR_MATH_NS::Matrix4x4& Transform2D::GetMatrix() const {
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

    void Transform2D::UpdateMatrix() const {
        const SR_MATH_NS::FVector2 size = GetSize();

        m_localMatrix = SR_MATH_NS::Matrix4x4(
            m_translation,
            SR_MATH_NS::Quaternion::Identity(),
            SR_MATH_NS::FVector3(size, 1.f),
            m_skew
        ) * SR_MATH_NS::Matrix4x4::FromQuaternion(m_quaternion);

        Transform::UpdateMatrix();
    }

    void Transform2D::BuildUITree() {
        SR_TRACY_ZONE;

        UI::UIModifierContext context;

        for (const UI::UIModifierComponent* pModifier : m_modifiers) {
            pModifier->Prepare(context);
        }

        if (m_gameObject) {
            for (auto&& pChild : m_gameObject->GetChildrenRef()) {
                if (pChild->GetSceneObjectType() != SceneObjectType::GameObject) {
                    continue;
                }

                auto&& pGameObject = pChild.StaticCast<GameObject>();
                if (pGameObject->GetTransform()->GetMeasurement() != Measurement::Space2D) {
                    continue;
                }

                auto&& pTransform2D = static_cast<Transform2D*>(pGameObject->GetTransform());

                pTransform2D->BuildUITree();

                context.childTranslation = pTransform2D->GetTranslation();
                context.childSize = pTransform2D->GetSize();

                for (const UI::UIModifierComponent* pModifier : m_modifiers) {
                    pModifier->ApplyChild(context);
                }
            }
        }

        for (const UI::UIModifierComponent* pModifier : m_modifiers) {
            pModifier->PostProcess(context);
        }

        m_translation = SR_MATH_NS::FVector3(context.position, context.zOrder);
        m_contentSize = context.contentSize;

        UpdateTree();
    }

    SR_MATH_NS::FVector2 Transform2D::GetSize() const {
        SR_TRACY_ZONE;

        if (m_contentSize.HasPercent()) {
            if (auto&& pParent = GetParentTransform()) {
                 if (pParent->GetMeasurement() == Measurement::Space2D) {
                     const Transform2D* pParentTransform = static_cast<Transform2D*>(pParent);
                     return m_contentSize.ToPixels(pParentTransform->GetSize());
                 }
            }
        }

        return m_contentSize.ToPixels();
    }

    void Transform2D::SetGlobalTranslation(const SR_MATH_NS::FVector3& translation) {
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

    void Transform2D::SetGlobalRotation(const SR_MATH_NS::Quaternion& quaternion) {
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

    Transform::Ptr Transform2D::Copy() const {
        auto&& pTransform = new Transform2D();

        pTransform->m_priority = m_priority;
        pTransform->m_localPriority = m_localPriority;
        pTransform->m_relativePriority = m_relativePriority;

        pTransform->m_translation = m_translation;
        pTransform->m_rotation = m_rotation;
        pTransform->m_quaternion = m_quaternion;
        pTransform->m_scale = m_scale;
        pTransform->m_skew = m_skew;

        return pTransform;
    }

    int32_t Transform2D::GetPriority() { /// NOLINT
        if (!m_isDirtyPriority) {
            return m_priority;
        }

        if (m_relativePriority) {
            if (auto&& pParentTransform = dynamic_cast<Transform2D*>(GetParentTransform())) {
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

    void Transform2D::SetLocalPriority(int32_t priority) {
        if (m_localPriority == priority) {
            return;
        }
        m_localPriority = priority;
        UpdatePriorityTree();
    }

    void Transform2D::SetRelativePriority(bool relative) {
        if (m_relativePriority == relative) {
            return;
        }
        m_relativePriority = relative;
        UpdatePriorityTree();
    }

    void Transform2D::UpdatePriorityTree() {
        m_isDirtyPriority = true;

        if (!m_gameObject) {
            return;
        }

        m_gameObject->OnPriorityChanged();

        for (auto&& pChild : m_gameObject->GetChildrenRef()) {
            if (auto&& pGameObject = pChild.DynamicCast<GameObject>()) {
                if (auto&& pTransform2D = dynamic_cast<Transform2D*>(pGameObject->GetTransform())) {
                    pTransform2D->UpdatePriorityTree();
                }
            }
        }
    }

    void Transform2D::OnHierarchyChanged() {
        UpdatePriorityTree();
        Transform::OnHierarchyChanged();
    }

    void Transform2D::RemoveModifier(UI::UIModifierComponent* pModifier) {
        /// erase element witout memory reallocation
        std::erase(m_modifiers, pModifier);
    }

    void Transform2D::OnUITreeChanged() {
        SR_TRACY_ZONE;

        if (auto&& pParent = GetParentTransform()) SR_LIKELY_ATTRIBUTE {
            if (pParent->GetMeasurement() == Measurement::Space2D) SR_LIKELY_ATTRIBUTE {
                static_cast<Transform2D*>(pParent)->OnUITreeChanged();
                return;
            }
        }

        BuildUITree();
    }
}
