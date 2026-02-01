//
// Created by Monika on 12.04.2023.
//

#include <Utils/ECS/Entity.h>
#include <Utils/ECS/EntityController.h>
#include <Utils/ECS/EntityRef.h>
#include <Utils/Reflection/SRClassUtils.h>

#include <Enum/EditorFlags.hpp>

#include <Codegen/Entity.generated.hpp>

namespace SR_UTILS_NS {
    /*EntityBranch::EntityBranch(EntityId entityId, std::list<EntityBranch> branches)
        : m_branches(std::move(branches))
        , m_id(entityId)
    { }

    void EntityBranch::Reserve() const {
        if (m_id != ENTITY_ID_MAX) {
            EntityManager::Instance().Reserve(m_id);
        }

        for (const auto& branch : m_branches) {
            branch.Reserve();
        }
    }

    void EntityBranch::UnReserve() const {
        if (m_id != ENTITY_ID_MAX) {
            EntityManager::Instance().TryUnReserve(m_id);
        }

        for (const auto& branch : m_branches) {
            branch.UnReserve();
        }
    }

    void EntityBranch::Clear() {
        m_id = ENTITY_ID_MAX;
        m_branches.clear();
    }

    ///---------------------------------------------------------------------------------------------------------------------

    EntityPath EntityPath::Concat(const EntityId &id) const {
        auto path = m_path;
        path.emplace_back(id);
        return EntityPath(std::move(path));
    }

    EntityPath EntityPath::ConcatBack(const EntityId &id) const {
        auto path = m_path;
        path.emplace_front(id);
        return EntityPath(std::move(path));
    }

    EntityPath& EntityPath::Concat(const EntityId &id) {
        m_path.emplace_back(id);
        return *this;
    }

    EntityPath& EntityPath::ConcatBack(const EntityId &id) {
        m_path.emplace_front(id);
        return *this;
    }

    EntityId EntityPath::Last() const {
        if (m_path.empty()) {
            return ENTITY_ID_MAX;
        }

        return m_path.back();
    }

    void EntityPath::Reserve() const {
        for (EntityId id : m_path) {
            if (id != ENTITY_ID_MAX)
                EntityManager::Instance().Reserve(id);
        }
    }

    void EntityPath::UnReserve() const {
        for (EntityId id : m_path) {
            if (id == ENTITY_ID_MAX) {
                continue;
            }
            EntityManager::Instance().TryUnReserve(id);
        }
    }

    void EntityPath::Clear() {
        for (EntityId& id : m_path) {
            id = ENTITY_ID_MAX;
        }
        m_path.clear();
    }*/

    ///---------------------------------------------------------------------------------------------------------------------

    Entity::Entity()
        : SR_HTYPES_NS::SharedPtr<Entity>(this, SharedPtrPolicy::Manually)
    { }

    Entity::~Entity() {
        UnregisterEntity();
    }

    void Entity::SetEntityController(EntityController* pEntityController) {
        SRAssert2(!pEntityController || !m_pEntityController, "Entity controller already set!");
        m_pEntityController = pEntityController;
    }

    void Entity::SetEntityId(const EntityId id) {
        SRAssert2(!IsEntityRegistered(), "Can't change entity id after registration!");
        m_entityId = id;
    }

    void Entity::UnregisterEntity() {
        if (m_pEntityController) {
            m_pEntityController->Unregister(m_entityId);
            m_pEntityController = nullptr;
        }
    }

    void Entity::OnPostLoad() {
        Super::OnPostLoad();
    }

    void Entity::OnEntityIdReplaced(const EntityReplaceMap& replaceMap) {
        if (replaceMap.empty()) {
            return;
        }

        SR_TRACY_ZONE;

        GetMeta()->ForEachSRClass(*this, [&replaceMap](SR_UTILS_NS::SRClass& srClass) {
            if (srClass.GetMeta()->GetFactoryName() == EntityRefBase::GetClassStaticName()) {
                static_cast<EntityRefBase&>(srClass).OnEntityIdReplaced(replaceMap);
            }
            else {
                srClass.GetMeta()->ForEachSRClass(srClass, [&replaceMap](SR_UTILS_NS::SRClass& innerSRClass) {
                    if (innerSRClass.GetMeta()->GetFactoryName() == EntityRefBase::GetClassStaticName()) {
                        static_cast<EntityRefBase&>(innerSRClass).OnEntityIdReplaced(replaceMap);
                    }
                });
            }
        });
    }

    bool Entity::HasEditorFlags(EditorFlags flags) const noexcept {
        SR_TRACY_ZONE;
        return SR_MATH_NS::IsMaskIncludedSubMask(m_editorFlags, flags);
    }

    void Entity::AddEditorFlags(EditorFlags flags) noexcept {
        m_editorFlags |= flags;
    }

    void Entity::RemoveEditorFlags(EditorFlags flags) noexcept {
        m_editorFlags &= ~flags;
    }

    Entity::Ptr Entity::GetEntity() const noexcept {
        return GetThis();
    }

    //void Entity::SetEntityPath(const EntityPath &path) {
    //    m_entityPath = path;
    //}

    ///---------------------------------------------------------------------------------------------------------------------
}