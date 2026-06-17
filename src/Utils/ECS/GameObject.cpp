 //
// Created by Nikita on 27.11.2020.
//

#include <Utils/ECS/GameObject.h>
#include <Utils/ECS/Transform3D.h>
#include <Utils/ECS/Component.h>
#include <Utils/ECS/LayerManager.h>

#include <Utils/World/Scene.h>

#include <Utils/Math/Vector3.h>
#include <Utils/Math/Quaternion.h>
#include <Utils/Math/Matrix4x4.h>
#include <Utils/Math/Mathematics.h>

#include <Codegen/GameObject.generated.hpp>

namespace SR_UTILS_NS {
    GameObject::GameObject(StringAtom name)
        : Super(name)
    { }

    GameObject::~GameObject() {
        m_transform.AutoFree();
    }

    GameObject::Ptr GameObject::GetOrCreateChild(StringAtom name) {
        if (auto&& pChild = DynamicPointerCast<GameObject>(Find(name))) {
            return pChild;
        }
        return CreateChild(name);
    }

    GameObject::Ptr GameObject::CreateChild(StringAtom name) {
        auto&& pChild = GetScene()->InstanceGameObject(name);
        if (!Super::AddChild(StaticPointerCast<SceneObject>(pChild))) {
            SRHalt("Something went wrong!");
        }
        return DynamicPointerCast<GameObject>(pChild);
    }

    const Transform::Ptr& GameObject::GetTransform() const noexcept {
        if (!m_transform) SR_UNLIKELY_ATTRIBUTE {
            const_cast<GameObject&>(*this).SetTransform(Transform3D::MakeShared<Transform3D, Transform>());
            SRAssert(m_transform);
        }
        return m_transform;
    }

    void GameObject::SetTransform(const SR_HTYPES_NS::SharedPtr<Transform>& pTransform) {
        if (m_transform == pTransform) {
            return;
        }

        if (!pTransform) {
            SRHalt("pTransform is nullptr!");
            return;
        }

        if (pTransform->GetMeasurement() == Measurement::Holder && GetParent()) {
            SRHalt("Incorrect HOLDER transform usage!");
            return;
        }

        m_transform.AutoFree();
        m_transform = pTransform;
        m_transform->SetGameObject(this);
        SetDirty(true);

        m_transform->UpdateTree();
    }

    void GameObject::OnHierarchyChanged() {
        Super::OnHierarchyChanged();
        if (auto&& pTransform = GetTransform()) {
            pTransform->OnHierarchyChanged();
        }
    }

    void GameObject::OnAttachedToParent() {
        if (GetParentTransform()) {
            GetTransform()->UpdateTree();
        }
        else {
            SR_WARN("GameObject::OnAttachedToParent() : GameObject doesn't have parent to get transform!");
        }
    }

    const Transform::Ptr& GameObject::GetParentTransform() const noexcept {
        auto&& parent = GetParent();
        if (parent && parent->GetSceneObjectType() == SceneObjectType::GameObject) {
            return static_cast<const GameObject*>(parent.Get())->GetTransform();
        }
        static const Transform::Ptr nullTransform;
        return nullTransform;
    }
}