//
// Created by Monika on 30.11.2022.
//

#include <Utils/ECS/EntityRefOld.h>
#include <Utils/ECS/GameObject.h>

#include <Codegen/EntityRefOld.generated.hpp>

namespace SR_UTILS_NS {
    EntityRefOld::EntityRefOld(EntityRefUtils::OwnerRef owner)
        : m_owner(std::move(owner))
    { }

    void EntityRefOld::OnPreSave() {
        UpdatePath();
        Serializable::OnPreSave();
    }

    GameObject::Ptr EntityRefOld::GetGameObject() const {
        if (m_path.empty() && m_target) {
            UpdatePath();
        }

        if (!m_target) {
            UpdateTarget();
        }

        return m_target.DynamicCast<GameObject>();
    }

    SR_HTYPES_NS::SharedPtr<SceneObject> EntityRefOld::GetSceneObject() const {
        if (m_path.empty() && m_target) {
            UpdatePath();
        }

        if (!m_target) {
            UpdateTarget();
        }

        return m_target.DynamicCast<SceneObject>();
    }

    Component::Ptr EntityRefOld::GetComponent() const {
        if (m_path.empty() && m_target) {
            UpdatePath();
        }

        if (!m_target) {
            UpdateTarget();
        }

        return m_target.DynamicCast<Component>();
    }

    void EntityRefOld::UpdatePath() const {
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

    void EntityRefOld::UpdateTarget() const {
        SRAssert(EntityRefUtils::IsOwnerValid(m_owner));

        if (IsRelative()) {
            m_target = EntityRefUtils::GetEntity(m_owner, m_path);
        }
        else {
            auto&& pScene = EntityRefUtils::GetSceneFromOwner(m_owner);
            m_target = EntityRefUtils::GetEntity(pScene, m_path);
        }
    }

    void EntityRefOld::SetRelative(bool relative) {
        m_relative = relative;
        UpdatePath();
    }

    EntityRefOld& EntityRefOld::SetPathTo(const Entity::Ptr& pEntity) {
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

    bool EntityRefOld::IsValid() const {
        return m_target && EntityRefUtils::IsOwnerValid(m_owner);
    }

    void EntityRefOld::SetOwner(const EntityRefUtils::OwnerRef& owner) {
        m_owner = owner;
        UpdatePath();
    }

    EntityRefOld EntityRefOld::Copy(const EntityRefUtils::OwnerRef& owner) const {
        EntityRefOld ref(owner);
        ref.m_relative = m_relative;
        ref.m_path = m_path;
        return ref;
    }
}