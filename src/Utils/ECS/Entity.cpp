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
}