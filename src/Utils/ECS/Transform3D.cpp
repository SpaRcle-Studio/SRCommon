//
// Created by Monika on 19.02.2022.
//

#include <Utils/ECS/Transform3D.h>
#include <Utils/ECS/GameObject.h>

namespace SR_UTILS_NS {
    void Transform3D::UpdateMatrix() const {
        if (m_skew.IsEqualsLikely(SR_MATH_NS::FVector3::One(), SR_EPSILON)) SR_LIKELY_ATTRIBUTE {
            m_localMatrix = SR_MATH_NS::Matrix4x4(
                m_translation,
                m_quaternion,
                m_scale
            );
        }
        else {
            m_localMatrix = SR_MATH_NS::Matrix4x4(
                m_translation,
                m_quaternion,
                m_scale,
                m_skew
            );
        }

        Super::UpdateMatrix();
    }

    const SR_MATH_NS::Matrix4x4& Transform3D::GetMatrix() const {
        if (IsDirty()) SR_UNLIKELY_ATTRIBUTE {
            UpdateMatrix();

            if (auto&& pTransform = m_gameObject->GetParentTransform()) SR_LIKELY_ATTRIBUTE {
                m_matrix = pTransform->GetMatrix() * m_localMatrix;
            }
            else {
                m_matrix = m_localMatrix;
            }
        }

        return m_matrix;
    }

    void Transform3D::Rotate(const SR_MATH_NS::FVector3& eulers) {
        Rotate(eulers.Radians().ToQuat());
    }

    void Transform3D::Rotate(const SR_MATH_NS::Quaternion &q) {
        SetRotation((m_quaternion * q).EulerAngle());
    }

    void Transform3D::Translate(const SR_MATH_NS::FVector3& translation) {
        const auto&& direction = TransformDirection(translation);
        SetTranslation(m_translation + direction);
    }

    SR_MATH_NS::FVector3 Transform3D::TransformDirection(const SR_MATH_NS::FVector3& direction) const {
        return m_quaternion * direction;
    }

    void Transform3D::SetMatrix(
        const std::optional<SR_MATH_NS::FVector3>& translation,
        const std::optional<SR_MATH_NS::Quaternion>& rotation,
        const std::optional<SR_MATH_NS::FVector3>& scale
    ) {
        bool changed = false;

        if (translation.has_value()) SR_LIKELY_ATTRIBUTE {
            m_translation = translation.value();
            changed = true;
        }

        if (rotation.has_value()) SR_LIKELY_ATTRIBUTE {
            m_quaternion = rotation.value();
            m_eulersDirty = true;
            changed = true;
        }

        if (scale.has_value()) SR_LIKELY_ATTRIBUTE {
            m_scale = scale.value();
            changed = true;
        }

        if (changed) {
            UpdateTree();
        }
    }

    void Transform3D::SetTranslation(const SR_MATH_NS::FVector3& translation) {
        const SR_MATH_NS::FVector3 delta = translation - m_translation;

        if (delta.Empty()) {
            return;
        }

        m_translation = translation;

    #ifdef SR_DEBUG
        if (!m_translation.IsFinite()) {
            SRHaltOnce("Translation is broke!");
            m_translation = SR_MATH_NS::FVector3::Zero();
        }
    #endif

        UpdateTree();
    }

    void Transform3D::SetRotation(const SR_MATH_NS::FVector3& euler) {
    #ifdef SR_DEBUG
        if (!euler.IsFinite()) {
            SRHaltOnce("Rotation is broke!");
            m_rotation = SR_MATH_NS::FVector3::Zero();
            m_quaternion = SR_MATH_NS::Quaternion::Identity();
        }
        else {
    #endif
            m_quaternion = euler.Radians().ToQuat();
            m_rotation = euler;
    #ifdef SR_DEBUG
        }
    #endif

        UpdateTree();
    }


    void Transform3D::SetRotation(const SR_MATH_NS::Quaternion& quaternion) {
        if (m_quaternion == quaternion) {
            return;
        }

    #ifdef SR_DEBUG
        if (!quaternion.IsFinite()) {
            SRHaltOnce("Rotation is broke!");
            m_eulersDirty = true;
            m_quaternion = SR_MATH_NS::Quaternion::Identity();
        }
        else {
    #endif
            m_eulersDirty = true;
            m_quaternion = quaternion;
    #ifdef SR_DEBUG
        }
    #endif

        UpdateTree();
    }

    void Transform3D::SetTranslationAndRotation(const SR_MATH_NS::FVector3 &translation, const SR_MATH_NS::FVector3 &euler) {
        SR_MATH_NS::FVector3 deltaTranslation = translation - m_translation;
        SR_MATH_NS::FVector3 deltaRotation = (SR_MATH_NS::Quaternion::FromEuler(euler) * GetQuaternion().Inverse()).EulerAngle();

        m_translation = translation;
        SetRotation(euler);

        for (auto&& pChild : m_gameObject->GetChildrenRef()) {
            if (auto&& pGameObject = pChild.DynamicCast<GameObject>()) {
                pGameObject->GetTransform()->GlobalTranslate(deltaTranslation);
                pGameObject->GetTransform()->RotateAroundParent(deltaRotation);
            }
        }
    }

    void Transform3D::SetScale(const SR_MATH_NS::FVector3& rawScale) {
        if (rawScale == m_scale) {
            return;
        }

        SR_MATH_NS::FVector3 scale = rawScale;

        if (!scale.IsFinite()) {
            SR_WARN("Transform3D::SetScale() : scale contains NaN! Reset...");
            scale = SR_MATH_NS::FVector3::One();
        }

        m_scale = scale;

        UpdateTree();
    }

    void Transform3D::SetSkew(const SR_MATH_NS::FVector3& rawSkew) {
        SR_MATH_NS::FVector3 skew = rawSkew;

        if (!skew.IsFinite()) {
            SR_WARN("Transform3D::GlobalSkew() : skew contains NaN! Reset...");
            skew = SR_MATH_NS::FVector3::One();
        }

        m_skew = skew;

        UpdateTree();
    }

    void Transform3D::RotateAround(const SR_MATH_NS::FVector3& point, const SR_MATH_NS::FVector3& eulers) {
        const SR_MATH_NS::Quaternion&& q = SR_MATH_NS::Quaternion::FromEuler(eulers);
        const SR_MATH_NS::Quaternion&& rotation = q * GetQuaternion();

        const SR_MATH_NS::FVector3&& worldPos = m_translation - point;
        const SR_MATH_NS::FVector3&& rotatedPos = q * worldPos;

        SetTranslationAndRotation(point + rotatedPos, rotation.EulerAngle());
    }

    void Transform3D::RotateAroundParent(const SR_MATH_NS::FVector3& eulers) {
        if (auto&& pTransform = GetParentTransform()) {
            RotateAround(pTransform->GetTranslation(), eulers);
        }
        else {
            Rotate(eulers);
        }
    }

    void Transform3D::Scale(const SR_MATH_NS::FVector3& scale) {
        if (scale.HasZero()) {
            return;
        }
        SetScale(m_scale * scale);
    }

    void Transform3D::SetGlobalTranslation(const SR_MATH_NS::FVector3 &translation) {
        if (auto&& pParent = GetParentTransform()) {
            auto matrix = SR_MATH_NS::Matrix4x4::FromTranslate(translation);
            matrix *= SR_MATH_NS::Matrix4x4::FromScale(m_skew);
            matrix *= SR_MATH_NS::Matrix4x4::FromEulers(GetRotation().Inverse());
            matrix *= SR_MATH_NS::Matrix4x4::FromScale(m_scale);

            matrix = pParent->GetMatrix().Inverse() * matrix;

            m_translation = matrix.GetTranslate();

            UpdateTree();
        }
        else {
            SetTranslation(translation);
        }
    }

    void Transform3D::SetGlobalRotation(const SR_MATH_NS::Quaternion& quaternion) {
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

    Transform *Transform3D::Copy() const {
        auto&& pTransform = new Transform3D();

        pTransform->m_quaternion = m_quaternion;

        pTransform->m_translation = m_translation;
        pTransform->m_scale = m_scale;
        pTransform->m_skew = m_skew;

        return pTransform;
    }

    void Transform3D::LookAt(const SR_MATH_NS::FVector3& position) {
        LookAt(position, LookAtAxis::AxisZ);
    }

    void Transform3D::LookAt(const SR_MATH_NS::FVector3& position, LookAtAxis axis) {
        /*SR_MATH_NS::FVector3 target;

        switch (axis) {
            case LookAtAxis::AxisX: target = Transform3D::RIGHT; break;
            case LookAtAxis::AxisY: target = Transform3D::UP; break;
            case LookAtAxis::AxisZ: target = Transform3D::FORWARD; break;
            case LookAtAxis::InvAxisX: target = -Transform3D::RIGHT; break;
            case LookAtAxis::InvAxisY: target = -Transform3D::UP; break;
            case LookAtAxis::InvAxisZ: target = -Transform3D::FORWARD; break;
            default:
                SRHalt0();
                break;
        }*/

        //auto&& source = GetMatrix().GetTranslate();

        //SetRotation(m_quaternion.LookAt(source - position));
    }
}



