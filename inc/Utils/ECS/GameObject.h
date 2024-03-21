//
// Created by Nikita on 27.11.2020.
//

#ifndef GAMEENGINE_GAMEOBJECT_H
#define GAMEENGINE_GAMEOBJECT_H

#include <Utils/ECS/EntityManager.h>
#include <Utils/ECS/IComponentable.h>
#include <Utils/ECS/TagManager.h>
#include <Utils/ECS/Prefab.h>

#include <Utils/Math/Vector3.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/SafeVariable.h>

namespace SR_UTILS_NS::World {
    class Scene;
}

namespace SR_UTILS_NS {
    class Transform3D;
    class Transform;
    class Component;

    class SR_DLL_EXPORT GameObject : public IComponentable, public Entity {
        SR_ENTITY_SET_VERSION(1009);
        friend class Component;
    public:
        using Name = std::string;
        using Ptr = SR_HTYPES_NS::SharedPtr<GameObject>;
        using GameObjects = std::vector<GameObject::Ptr>;
        using ScenePtr = SR_WORLD_NS::Scene*;
        using IdGetterFn = SR_HTYPES_NS::Function<uint64_t(const GameObject::Ptr&)>;

    public:
        GameObject(std::string name, Transform* pTransform);
        explicit GameObject(std::string name);

        ~GameObject() override;

        static GameObject::Ptr Load(SR_HTYPES_NS::Marshal& marshal, const ScenePtr& scene);

    public:
        SR_NODISCARD GameObject::Ptr Copy(const ScenePtr& scene) const;

        SR_NODISCARD ScenePtr GetScene() const override { return m_scene; }
        SR_NODISCARD StringAtom GetLayer() const noexcept { return m_cachedLayer; }
        SR_NODISCARD StringAtom GetLocalLayer() const noexcept { return m_layer; }
        SR_NODISCARD Prefab* GetPrefab() const noexcept { return m_prefab.first; }
        SR_NODISCARD bool IsPrefab() const noexcept { return m_prefab.first; }
        SR_NODISCARD bool IsPrefabOwner() const noexcept { return m_prefab.second; }
        SR_NODISCARD Transform* GetParentTransform() const noexcept { return m_parent ? m_parent->m_transform : nullptr; }
        SR_NODISCARD Transform* GetTransform() const noexcept { return m_transform; }
        SR_NODISCARD GameObject::Ptr GetParent() const noexcept { return m_parent; }
        SR_NODISCARD GameObject::Ptr GetRoot() const noexcept;
        SR_NODISCARD GameObject::Ptr Find(uint64_t hashName) const noexcept;
        SR_NODISCARD GameObject::Ptr Find(const std::string& name) const noexcept;
        SR_NODISCARD GameObject::Ptr Find(StringAtom name) const noexcept;
        SR_NODISCARD std::string GetName() const { return m_name; }
        SR_NODISCARD StringAtom GetTag() const;
        SR_NODISCARD std::string GetTagString() const;
        SR_NODISCARD bool IsActive() const noexcept override;
        SR_NODISCARD SR_FORCE_INLINE bool IsEnabled() const noexcept { return m_isEnabled; }
        SR_NODISCARD SR_FORCE_INLINE uint64_t GetHashName() const noexcept { return m_hashName; }
        SR_NODISCARD SR_FORCE_INLINE uint64_t GetIdInScene() const noexcept { return m_idInScene; }
        SR_NODISCARD SR_INLINE bool HasChildren() const { return !m_children.empty(); }
        SR_NODISCARD SR_INLINE GameObjects& GetChildrenRef() { return m_children; }
        SR_NODISCARD SR_INLINE const GameObjects& GetChildrenRef() const { return m_children; }
        SR_NODISCARD SR_INLINE GameObjects GetChildren() const { return m_children; }

        SR_NODISCARD std::string GetEntityInfo() const override;

        SR_NODISCARD SR_HTYPES_NS::Marshal::Ptr Save(SavableContext data) const override;
        SR_NODISCARD std::list<EntityBranch> GetEntityBranches() const override;

        SR_MATH_NS::FVector3 GetBarycenter();
        SR_MATH_NS::FVector3 GetHierarchyBarycenter();

        void SetIdInScene(uint64_t id);
        void SetScene(ScenePtr pScene);
        void SetPrefab(Prefab* pPrefab, bool owner);
        void UnlinkPrefab();

        void ForEachChild(const std::function<void(GameObject::Ptr&)>& fun);
        void ForEachChild(const std::function<void(const GameObject::Ptr&)>& fun) const;
        bool SetParent(const GameObject::Ptr& parent);
        void RemoveAllChildren();
        void SetName(std::string name);
        void SetTag(SR_UTILS_NS::StringAtom tag);

        bool Contains(const GameObject::Ptr& child);
        void SetEnabled(bool value);
        void SetLayer(StringAtom layer);
        void SetTransform(Transform* transform);

        bool MoveToTree(const GameObject::Ptr& destination);
        void RemoveChild(const GameObject::Ptr& child);
        bool AddChild(const GameObject::Ptr& child);

        SR_NODISCARD GameObject::Ptr AddChild(StringAtom name);
        SR_NODISCARD GameObject::Ptr GetOrAddChild(StringAtom name);

        /// Вызывает OnAttached у компонентов загруженных через LoadComponent
        bool PostLoad(bool force) override;

        void Awake(bool force, bool isPaused) noexcept override;
        void Start(bool force) noexcept override;

        void CheckActivity(bool force) noexcept override;

        bool SetDirty(bool value) override;

        /// ставит объект на очередь уничтожения, если есть сцена. Если сцены нет - сразу уничтожает
        void Destroy();

        /// освобождает память объекта
        void DestroyImpl();

    private:
        void OnAttached();
        void OnParentLayerChanged();

        bool UpdateEntityPath();

    private:
        bool m_isEnabled = true;
        bool m_isActive = false;
        bool m_isDestroyed = false;

        StringAtom m_layer;
        StringAtom m_cachedLayer;

        StringAtom m_tag;

        uint64_t m_hashName = 0;
        uint64_t m_idInScene = SR_ID_INVALID;

        GameObject::Ptr m_parent;

        GameObjects m_children;

        ScenePtr m_scene = nullptr;
        Transform* m_transform  = nullptr;
        std::pair<Prefab*, bool> m_prefab;

        Name m_name;

    };
}

#endif //GAMEENGINE_GAMEOBJECT_H
