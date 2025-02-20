//
// Created by Monika on 30.11.2022.
//

#include <Utils/ECS/EntityRef.h>

#include <Codegen/EntityRef.generated.hpp>

namespace SR_UTILS_NS {
    EntityRef::EntityRef(EntityRefUtils::OwnerRef owner)
        : m_owner(std::move(owner))
    { }

    void EntityRef::OnPreSave() {
        UpdatePath();
        Serializable::OnPreSave();
    }

    GameObject::Ptr EntityRef::GetGameObject() const {
        if (m_path.empty() && m_target) {
            UpdatePath();
        }

        if (!m_target) {
            UpdateTarget();
        }

        return m_target.DynamicCast<GameObject>();
    }

    SR_HTYPES_NS::SharedPtr<SceneObject> EntityRef::GetSceneObject() const {
        if (m_path.empty() && m_target) {
            UpdatePath();
        }

        if (!m_target) {
            UpdateTarget();
        }

        return m_target.DynamicCast<SceneObject>();
    }

    Component::Ptr EntityRef::GetComponent() const {
        if (m_path.empty() && m_target) {
            UpdatePath();
        }

        if (!m_target) {
            UpdateTarget();
        }

        return m_target.DynamicCast<Component>();
    }

    void EntityRef::UpdatePath() const {
        if (!EntityRefUtils::IsTargetInitialized(m_target)) {
            return;
        }

        if (!EntityRefUtils::IsOwnerValid(m_owner)) {
            SRHalt("Invalid owner!");
            return;
        }

        if (!m_target) {
            return;
        }

        if (IsRelative()) {
            m_path = EntityRefUtils::CalculateRelativePath(m_owner, m_target);
        }
        else {
            m_path = EntityRefUtils::CalculatePath(m_target);
        }
    }

    void EntityRef::UpdateTarget() const {
        SRAssert(EntityRefUtils::IsOwnerValid(m_owner));

        if (IsRelative()) {
            m_target = EntityRefUtils::GetEntity(m_owner, m_path);
        }
        else {
            auto&& pScene = EntityRefUtils::GetSceneFromOwner(m_owner);
            m_target = EntityRefUtils::GetEntity(pScene, m_path);
        }
    }

    void EntityRef::SetRelative(bool relative) {
        m_relative = relative;
        UpdatePath();
    }

    EntityRef& EntityRef::SetPathTo(const Entity::Ptr& pEntity) {
        if (!EntityRefUtils::IsOwnerValid(m_owner)) {
            SRHalt("Invalid owner!");
            return *this;
        }

        if (!pEntity) {
            m_target = pEntity;
            m_path.clear();
            return *this;
        }

        if (IsRelative()) {
            m_path = EntityRefUtils::CalculateRelativePath(m_owner, pEntity);
        }
        else {
            m_path = EntityRefUtils::CalculatePath(pEntity);
        }

        UpdateTarget();

        return *this;
    }

    bool EntityRef::IsValid() const {
        return m_target && EntityRefUtils::IsOwnerValid(m_owner);
    }

    void EntityRef::SetOwner(const EntityRefUtils::OwnerRef& owner) {
        m_owner = owner;
        UpdatePath();
    }

    EntityRef EntityRef::Copy(const EntityRefUtils::OwnerRef& owner) const {
        EntityRef ref(owner);
        ref.m_relative = m_relative;
        ref.m_path = m_path;
        return ref;
    }

    /*void EntityRefProperty::SaveProperty(MarshalRef marshal) const noexcept {
        if (auto&& pBlock = AllocatePropertyBlock()) {
            m_entityRef.Save(*pBlock);
            SavePropertyBase(marshal, std::move(pBlock));
        }
    }

    void EntityRefProperty::LoadProperty(MarshalRef marshal) noexcept {
        if (auto&& pBlock = LoadPropertyBase(marshal)) {
            m_entityRef.Load(*pBlock);
        }
    }*/
}