//
// Created by Monika on 12.04.2023.
//

#ifndef SR_ENGINE_UTILS_ENTITY_H
#define SR_ENGINE_UTILS_ENTITY_H

#include <Utils/Serialization/Serializable.h>
#include <Utils/Types/SharedPtr.h>

namespace SR_UTILS_NS {
    class Entity;
    class EntityController;

    typedef uint64_t EntityId;
    typedef SR_HTYPES_NS::FlatHashMap<EntityId, EntityId> EntityReplaceMap;

    SR_ENUM_NS_STRUCT_T(EditorFlags, uint64_t,
        None       = 1 << 0,
        DontDelete = 1 << 1,
        ReadOnly   = 1 << 2,
        Hidden     = 1 << 4
    )

    class Entity : public Serializable, public SR_HTYPES_NS::SharedPtr<Entity> {
        SR_CLASS()
        using Super = Serializable;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<Entity>;
        using OriginType = Entity;

    public:
        Entity();
        ~Entity() override;

    public:
        SR_NODISCARD bool IsEntityRegistered() const noexcept { return m_pEntityController; }
        SR_NODISCARD EntityId GetEntityId() const { return m_entityId; }
        SR_NODISCARD Entity::Ptr GetEntity() const noexcept;

        void SetEntityController(EntityController* pEntityController);
        void SetEntityId(EntityId id);
        void UnregisterEntity();

        void OnEntityIdReplaced(const EntityReplaceMap& replaceMap);

        void OnPostLoad() override;

        void AddEditorFlags(EditorFlags flags) noexcept;
        void RemoveEditorFlags(EditorFlags flags) noexcept;

        SR_NODISCARD bool HasEditorFlags(EditorFlags flags) const noexcept;

        SR_NODISCARD virtual bool IsPrefabLoadingState() const noexcept { return false; }

    private:
        /// @property @hidden @dontSaveTags(Inspector)
        /// @loadCondition(!This.IsPrefabLoadingState())
        EntityId m_entityId = SR_ID_INVALID;

        EntityController* m_pEntityController = nullptr;
        EditorFlags m_editorFlags = EditorFlags::None;

    };
}

#endif //SR_ENGINE_UTILS_ENTITY_H
