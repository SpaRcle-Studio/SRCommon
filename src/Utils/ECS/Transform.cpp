//
// Created by Nikita on 27.11.2020.
//

#include <Utils/ECS/Transform.h>
#include <Utils/Profile/TracyContext.h>

#include <Codegen/Transform.generated.hpp>

namespace SR_UTILS_NS {
    Transform::Transform()
        : Ptr(this, SR_UTILS_NS::SharedPtrPolicy::Manually)
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
        if (!m_gameObject) {
            return nullptr;
        }

        if (auto&& pParent = m_gameObject->GetParent()) {
            if (auto&& pGameObject = pParent.DynamicCast<GameObject>()) {
                return pGameObject->GetTransform().Get();
            }
        }

        return nullptr;
    }

    SR_NODISCARD GameObject::Ptr Transform::GetGameObject() const {
        return m_gameObject->GetThis().DynamicCast<GameObject>();
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

    void Transform::Rotate(Math::Unit x, Math::Unit y, Math::Unit z) {
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

    void Transform::Translate(Math::Unit x, Math::Unit y, Math::Unit z) {
        Translate(Math::FVector3(x, y, z));
    }

    void Transform::Scale(Math::Unit x, Math::Unit y, Math::Unit z) {
        Scale(Math::FVector3(x, y, z));
    }

    SR_MATH_NS::FVector2 Transform::GetTranslation2D() const {
        return GetTranslation().XY();
    }

    SR_MATH_NS::FVector2 Transform::GetScale2D() const {
        return GetScale().XY();
    }

    const SR_MATH_NS::Matrix4x4& Transform::GetMatrix() const {
        static SR_MATH_NS::Matrix4x4 matrix4X4 = SR_MATH_NS::Matrix4x4::Identity();
        return matrix4X4;
    }

    void Transform::UpdateTree() {
        SR_TRACY_ZONE;

        if (!m_gameObject) SR_UNLIKELY_ATTRIBUTE {
            return;
        }

        /// Тут могут быть потенциальные баги с обновлением дерева, если добавили компонент,
        /// а дерево грязное и никто не может запросить актуальную матрицу.
        /// Такие баги надо править со стороны КОМПОНЕНТОВ,
        /// чтобы они при своей инициализации могли запросить перестроение дерева!
        if (m_dirtyMatrix) {
            return;
        }
        m_dirtyMatrix = true;

        m_gameObject->OnMatrixDirty();

        for (auto&& pChild : m_gameObject->GetChildrenRef()) {
            if (auto&& pGameObject = pChild.DynamicCast<GameObject>()) {
                pGameObject->GetTransform()->UpdateTree();
            }
        }
    }

    bool Transform::IsDirty() const noexcept {
        return m_dirtyMatrix;
    }

    void Transform::OnHierarchyChanged() {
        UpdateTree();
    }

    SR_MATH_NS::FVector3 Transform::Right() const {
        return GetMatrix().GetQuat() * SR_MATH_NS::FVector3::Right();
    }

    SR_MATH_NS::FVector3 Transform::Up() const {
        return GetMatrix().GetQuat() * SR_MATH_NS::FVector3::Up();
    }

    SR_MATH_NS::FVector3 Transform::Forward() const {
        return GetMatrix().GetQuat() * SR_MATH_NS::FVector3::Forward();
    }

    void Transform::GlobalRotate(const SR_MATH_NS::Quaternion& quaternion) {
        SetRotation(GetQuaternion() * quaternion);
    }
}