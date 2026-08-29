//
// Created by Nikita on 27.11.2020.
//

#include <Utils/ECS/Transform.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/Profile/TracyContext.h>

#include <Codegen/Transform.generated.hpp>

namespace SR_UTILS_NS {
    Transform::Transform()
        : Ptr(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    Transform::~Transform() {
        m_gameObject = nullptr;
    }

    void Transform::SetGameObject(GameObject* pGameObject) {
        if ((m_gameObject = pGameObject)) {
            OnHierarchyChanged();
        }
    }

    Transform* Transform::GetParentTransform() const {
        if (m_gameObject) {
            if (auto&& pParent = m_gameObject->GetParent()) {
                if (pParent->GetSceneObjectType() == SceneObjectType::GameObject) {
                    return const_cast<Transform*>(static_cast<const GameObject*>(pParent.Get())->GetTransform().Get());
                }
            }
        }
        return nullptr;
    }

    SR_NODISCARD GameObject::Ptr Transform::GetGameObject() const {
        return SR_UTILS_NS::DynamicPointerCast<GameObject>(m_gameObject->GetThis());
    }

    void Transform::GlobalTranslate(const Math::FVector3& translation) {
        SetTranslation(GetTranslation() + translation);
    }

    void Transform::GlobalRotate(const Math::FVector3& eulers) {
        SetRotation(GetRotation() + eulers);
    }

    void Transform::GlobalScale(const Math::FVector3 &scale) {
        SetScale(GetScale() * scale);
    }

    void Transform::GlobalSkew(const Math::FVector3 &skew) {
        SetSkew(GetSkew() * skew);
    }

    void Transform::RotateAround(const SR_MATH_NS::FVector3& point, const SR_MATH_NS::FVector3& eulers) { }
    void Transform::RotateAroundParent(const SR_MATH_NS::FVector3& eulers) { }

    void Transform::RotateXYZ(Math::Unit x, Math::Unit y, Math::Unit z) {
        Rotate(Math::FVector3(x, y, z));
    }

    void Transform::GlobalRotate(Math::Unit x, Math::Unit y, Math::Unit z) {
        GlobalRotate(Math::FVector3(x, y, z));
    }

    void Transform::SetTranslation(Math::Unit x, Math::Unit y, Math::Unit z) {
        SetTranslation(Math::FVector3(x, y, z));
    }

    void Transform::SetRotation(Math::Unit yaw, Math::Unit pitch, Math::Unit roll) {
        SetRotation(Math::FVector3(yaw, pitch, roll));
    }

    void Transform::SetScale(Math::Unit x, Math::Unit y, Math::Unit z) {
        SetScale(Math::FVector3(x, y, z));
    }

    void Transform::SetSkew(Math::Unit x, Math::Unit y, Math::Unit z) {
        SetSkew(Math::FVector3(x, y, z));
    }

    void Transform::TranslateXYZ(Math::Unit x, Math::Unit y, Math::Unit z) {
        Translate(Math::FVector3(x, y, z));
    }

    void Transform::Translate(const SR_MATH_NS::FVector3& translation) { }
    void Transform::Rotate(const SR_MATH_NS::FVector3& eulers) { }
    void Transform::Rotate(const SR_MATH_NS::Quaternion& q) { }
    void Transform::Scale(const SR_MATH_NS::FVector3& scale) { }

    void Transform::ScaleXYZ(Math::Unit x, Math::Unit y, Math::Unit z) {
        Scale(Math::FVector3(x, y, z));
    }

    SR_MATH_NS::FVector2 Transform::GetTranslation2D() const {
        return GetTranslation().XY();
    }

    SR_MATH_NS::FVector3 Transform::InverseTransformPoint(const SR_MATH_NS::FVector3& point) const {
        return GetMatrix().Inverse().TransformPoint(point).XYZ();
    }

    SR_MATH_NS::FVector3 Transform::InverseTransformDirection(const SR_MATH_NS::FVector3& direction) const {
        return GetGlobalRotation().Inverse() * direction;
    }

    SR_MATH_NS::FVector3 Transform::TransformDirection(const SR_MATH_NS::FVector3& direction) const {
        SR_TRACY_ZONE;
        return GetGlobalRotation() * direction;
    }

    SR_MATH_NS::FVector3 Transform::GetGlobalTranslation() const {
        return GetMatrix().GetTranslate();
    }

    SR_MATH_NS::Quaternion Transform::GetGlobalRotation() const {
        SR_TRACY_ZONE;
        SRHaltOnce("Transform::GetGlobalRotation() is not implemented!");
        return SR_MATH_NS::Quaternion::Identity();
    }

    SR_MATH_NS::FVector3 Transform::GetGlobalScale() const {
        SR_TRACY_ZONE;
        SRHaltOnce("Transform::GetGlobalScale() is not implemented!");
        return SR_MATH_NS::FVector3::One();
    }

    SR_MATH_NS::FVector3 Transform::GetTranslation() const {
        return SR_MATH_NS::FVector3();
    }

    SR_MATH_NS::FVector3 Transform::GetRotation() const {
        return SR_MATH_NS::FVector3();
    }

    SR_MATH_NS::FVector3 Transform::GetScale() const {
        return SR_MATH_NS::FVector3();
    }

    SR_MATH_NS::FVector3 Transform::GetSkew() const {
        return SR_MATH_NS::FVector3();
    }


    SR_MATH_NS::FVector2 Transform::GetScale2D() const {
        return GetScale().XY();
    }

    void Transform::SetMatrix(const SR_UTILS_NS::Optional<SR_MATH_NS::FVector3>& translation,
                   const SR_UTILS_NS::Optional<SR_MATH_NS::Quaternion>& rotation,
                   const SR_UTILS_NS::Optional<SR_MATH_NS::FVector3>& scale) { }

    void Transform::SetGlobalTranslation(const SR_MATH_NS::FVector3& translation) { }

    void Transform::SetGlobalRotation(const SR_MATH_NS::Quaternion& quaternion) { }

    void Transform::SetTranslation(const SR_MATH_NS::FVector3& translation) { }
    void Transform::SetTranslationAndRotation(const SR_MATH_NS::FVector3& translation, const SR_MATH_NS::FVector3& euler) { }
    void Transform::SetRotation(const SR_MATH_NS::FVector3& euler) { }
    void Transform::SetRotation(const SR_MATH_NS::Quaternion& quaternion) { }
    void Transform::SetScale(const SR_MATH_NS::FVector3& scale) { }
    void Transform::SetSkew(const SR_MATH_NS::FVector3& skew) { }

    void Transform::LookAt(const SR_MATH_NS::FVector3& position) { }
    void Transform::LookAt(const SR_MATH_NS::FVector3& position, const SR_MATH_NS::FVector3& up) { }
    void Transform::LookAt(const SR_MATH_NS::FVector3& position, LookAtAxis axis) { }

    const SR_MATH_NS::Matrix4x4& Transform::GetMatrix() const {
        static SR_MATH_NS::Matrix4x4 matrix4X4 = SR_MATH_NS::Matrix4x4::Identity();
        return matrix4X4;
    }

    const SR_MATH_NS::AABB& Transform::GetAABB() const {
        static SR_MATH_NS::AABB aabb;
        return aabb;
    }

    void Transform::UpdateTree() {
        if (!m_gameObject) SR_UNLIKELY_ATTRIBUTE {
            return;
        }

        /// Тут могут быть потенциальные баги с обновлением дерева, если добавили компонент,
        /// а дерево грязное и никто не может запросить актуальную матрицу.
        /// Такие баги надо править со стороны КОМПОНЕНТОВ,
        /// чтобы они при своей инициализации могли запросить перестроение дерева!
        if (m_dirtyMatrix && m_dirtyRotation && m_dirtyScale) {
            return;
        }
        m_dirtyMatrix = true;
        m_dirtyRotation = true;
        m_dirtyScale = true;
        m_aabbDirty = true;

        m_gameObject->OnMatrixDirty();

        for (auto&& pChild : m_gameObject->GetChildrenRef()) {
            if (pChild->GetSceneObjectType() != SceneObjectType::GameObject) SR_UNLIKELY_ATTRIBUTE {
                continue;
            }

            static_cast<GameObject*>(pChild.Get())->GetTransform()->UpdateTree();
        }
    }

    void Transform::OnHierarchyChanged() {
        UpdateTree();
    }

    SR_MATH_NS::FVector3 Transform::Right() const {
        return GetGlobalRotation() * SR_MATH_NS::FVector3::Right();
    }

    SR_MATH_NS::FVector3 Transform::Up() const {
        return GetGlobalRotation() * SR_MATH_NS::FVector3::Up();
    }

    SR_MATH_NS::FVector3 Transform::Forward() const {
        return GetGlobalRotation() * SR_MATH_NS::FVector3::Forward();
    }

    void Transform::GlobalRotate(const SR_MATH_NS::Quaternion& quaternion) {
        SetRotation(GetQuaternion() * quaternion);
    }

    SR_MATH_NS::Quaternion Transform::GetQuaternion() const {
        return SR_MATH_NS::Quaternion::Identity();
    }

    void Transform::SetGlobalRotation(const SR_MATH_NS::FVector3& eulers) {
        SetGlobalRotation(eulers.Radians().ToQuat());
    }

    Measurement Transform::GetMeasurement() const {
        SRHalt("Abstract method called!");
        return Measurement::MeasurementMAX;
    }

    SR_MATH_NS::Matrix4x4 Transform::GetLocalMatrix() const {
        return SR_MATH_NS::Matrix4x4(
            GetTranslation(),
            GetRotation(),
            GetScale()
        );
    }

    void Transform::SetRotationXYZ(SR_MATH_NS::Unit x, SR_MATH_NS::Unit y, SR_MATH_NS::Unit z) {
        SetRotation(SR_MATH_NS::FVector3(x, y, z));
    }
}