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
    GameObject::~GameObject() {
        m_transform.AutoFree();
    }

    GameObject::Ptr GameObject::GetOrCreateChild(StringAtom name) {
        if (auto&& pChild = Find(name).DynamicCast<GameObject>()) {
            return pChild;
        }
        return CreateChild(name);
    }

    GameObject::Ptr GameObject::CreateChild(StringAtom name) {
        auto&& pChild = GetScene()->InstanceGameObject(name);
        if (!Super::AddChild(pChild.StaticCast<SceneObject>())) {
            SRHalt("Something went wrong!");
        }
        return pChild.StaticCast<GameObject>();
    }

    Transform::Ptr GameObject::GetTransform() const noexcept {
        if (!m_transform) {
            const_cast<GameObject&>(*this).SetTransform(Transform3D::MakeShared<Transform3D, Transform>());
        }
        SRAssert(m_transform);
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

    void GameObject::OnAttached() {
        if (GetParentTransform()) {
            GetTransform()->UpdateTree();
        }
        else {
            SR_WARN("GameObject::OnAttached() : GameObject doesn't have parent to get transform!");
        }
    }

    Transform::Ptr GameObject::GetParentTransform() const noexcept {
        if (GetParent() && GetParent()->GetSceneObjectType() == SceneObjectType::GameObject) {
            return GetParent().StaticCast<GameObject>()->GetTransform();
        }
        return nullptr;
    }
}