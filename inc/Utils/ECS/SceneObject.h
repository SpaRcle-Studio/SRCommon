//
// Created by Monika on 04.10.2024.
//

#ifndef SR_ENGINE_UTILS_SCENE_OBJECT_H
#define SR_ENGINE_UTILS_SCENE_OBJECT_H

#include <Utils/ECS/EntityManager.h>
#include <Utils/ECS/IComponentable.h>

namespace SR_UTILS_NS::World {
    class Scene;
}

namespace SR_UTILS_NS {
    class Prefab;

    SR_ENUM_NS_CLASS_T(SceneObjectType, int16_t,
        Invalid = -1,
        GameObject = 0
    )

    class SceneObject : public IComponentable {
        friend class Component;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<SceneObject>;
        using ScenePtr = SR_WORLD_NS::Scene*;
        using ObjectNameT = SR_UTILS_NS::StringAtom;
        using ChildrenType = std::vector<SceneObject::Ptr>;

    public:
        explicit SceneObject(ObjectNameT name);
        ~SceneObject() override;

    public:
        SR_NODISCARD SR_HTYPES_NS::Marshal::Ptr SaveLegacy(SavableContext data) const override;
        SR_NODISCARD virtual SceneObject::Ptr Copy(const ScenePtr& pScene, const SceneObject::Ptr& pObject) const;

        SR_NODISCARD SR_FORCE_INLINE const ObjectNameT& GetName() const { return m_name; }
        SR_NODISCARD SR_FORCE_INLINE bool IsEnabled() const noexcept { return m_isEnabled; }
        SR_NODISCARD SR_FORCE_INLINE uint64_t GetIdInScene() const noexcept { return m_idInScene; }

        SR_NODISCARD SR_FORCE_INLINE ScenePtr GetScene() const override { return m_scene; }
        SR_NODISCARD SR_FORCE_INLINE StringAtom GetLayer() const noexcept { return m_cachedLayer; }
        SR_NODISCARD SR_FORCE_INLINE StringAtom GetLocalLayer() const noexcept { return m_layer; }

        SR_NODISCARD SR_FORCE_INLINE Prefab* GetPrefab() const noexcept { return m_prefabInfo.pPrefab; }
        SR_NODISCARD SR_FORCE_INLINE bool IsPrefab() const noexcept { return m_prefabInfo.pPrefab; }
        SR_NODISCARD SR_FORCE_INLINE bool IsPrefabOwner() const noexcept { return m_prefabInfo.isOwner; }

        SR_NODISCARD SR_FORCE_INLINE SceneObject::Ptr GetParent() const noexcept { return m_parent; }
        SR_NODISCARD SR_FORCE_INLINE SceneObject::Ptr GetRoot() const noexcept { return m_root; }

        SR_NODISCARD SR_FORCE_INLINE bool HasChildren() const { return !m_children.empty(); }
        SR_NODISCARD SR_FORCE_INLINE ChildrenType& GetChildrenRef() { return m_children; }
        SR_NODISCARD SR_FORCE_INLINE const ChildrenType& GetChildrenRef() const { return m_children; }
        SR_NODISCARD SR_FORCE_INLINE ChildrenType GetChildren() const { return m_children; }
        SR_NODISCARD SR_FORCE_INLINE bool IsDestroyed() const noexcept override { return m_isDestroyed; }
        SR_NODISCARD SR_FORCE_INLINE bool IsActive() const noexcept override { return m_isActive && !m_isDestroyed; }

        SR_NODISCARD SceneObject::Ptr Find(uint64_t hashName) const noexcept;
        SR_NODISCARD SceneObject::Ptr Find(const std::string& name) const noexcept;
        SR_NODISCARD SceneObject::Ptr Find(const std::string_view& name) const noexcept;
        SR_NODISCARD SceneObject::Ptr Find(StringAtom name) const noexcept;

        SR_NODISCARD std::string GetEntityInfo() const override;
        SR_NODISCARD StringAtom GetTag() const;
        SR_NODISCARD std::list<EntityBranch> GetEntityBranches() const override;

        SR_NODISCARD virtual SceneObjectType GetSceneObjectType() const noexcept = 0;

        bool MoveToTree(const SceneObject::Ptr& destination);
        void RemoveChild(const SceneObject::Ptr& pChild);
        bool AddChild(const SceneObject::Ptr& pChild);
        void ForEachChild(const std::function<void(SceneObject::Ptr&)>& fn);
        void ForEachChild(const std::function<void(const SceneObject::Ptr&)>& fn) const;
        bool SetParent(const SceneObject::Ptr& parent);
        void SetName(ObjectNameT name);
        void SetTag(SR_UTILS_NS::StringAtom tag);
        bool Contains(const SceneObject::Ptr& pChild);
        void SetEnabled(bool value);
        void SetLayer(StringAtom layer);
        void SetIdInScene(uint64_t id);
        void SetScene(ScenePtr pScene);
        void SetPrefab(Prefab* pPrefab, bool isOwner);

        void UnlinkPrefab();
        void RemoveAllChildren();

        /// Вызывает OnAttached у компонентов загруженных через LoadComponent
        bool PostLoad(bool force) override;
        void Awake(bool force, bool isPaused) noexcept override;
        void Start(bool force) noexcept override;
        void CheckActivity(bool force) noexcept override;
        bool SetDirty(bool value) override;

        /// Ставит объект на очередь уничтожения, если есть сцена. Если сцены нет - сразу уничтожает.
        void Destroy();

        /// Освобождает память объекта
        void DestroyImpl();

    protected:
        virtual void OnHierarchyChanged() { }
        void UpdateRoot();

    private:
        virtual void OnAttached() { }
        void OnParentLayerChanged();

        bool UpdateEntityPath();

    private:
        struct PrefabInfo {
            Prefab* pPrefab = nullptr;
            bool isOwner = false;
        } m_prefabInfo;

        SceneObject::Ptr m_root;
        SceneObject::Ptr m_parent;
        ChildrenType m_children;

        bool m_isEnabled = true;
        bool m_isActive = false;
        bool m_isDestroyed = false;

        StringAtom m_layer;
        StringAtom m_cachedLayer;

        StringAtom m_tag;

        ObjectNameT m_name;

        ScenePtr m_scene = nullptr;
        SRHashType m_idInScene = SR_ID_INVALID;

    };
}

#endif //SR_ENGINE_UTILS_SCENE_OBJECT_H
