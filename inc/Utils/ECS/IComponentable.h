//
// Created by Monika on 12.12.2022.
//

#ifndef SR_ENGINE_I_COMPONENTABLE_H
#define SR_ENGINE_I_COMPONENTABLE_H

#include <Utils/ECS/Entity.h>

namespace SR_WORLD_NS {
    class Scene;
}

namespace SR_UTILS_NS {
    class Component;

    class SR_COMMON_DLL_API IComponentable : public Entity {
        using Super = Entity;
        SR_CLASS()
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<IComponentable>;
        using ScenePtr = SR_WORLD_NS::Scene*;
        using OriginType = IComponentable;
        using ComponentPtr = SR_HTYPES_NS::SharedPtr<Component>;

    public:
        IComponentable() = default;
        ~IComponentable() override;

    public:
        SR_NODISCARD bool IsDirty() const noexcept;

    public:
        /// Вызывает OnAttached у компонентов загруженных через LoadComponent
        virtual bool PostLoad(bool force);

        virtual void CheckActivity(bool force) noexcept;

        virtual void Awake(bool force, bool isPaused) noexcept;
        virtual void Start(bool force) noexcept;

        SR_NODISCARD virtual bool IsActive() const noexcept { return true; }
        SR_NODISCARD virtual bool IsDestroyed() const noexcept { return false; }

        virtual bool SetDirty(bool dirty);

        virtual ComponentPtr GetOrCreateComponent(const std::string& name);
        virtual ComponentPtr GetOrCreateComponent(StringAtom name);
        virtual ComponentPtr GetComponent(const std::string& name);
        virtual ComponentPtr GetComponent(StringAtom name);
        virtual bool MoveComponent(const ComponentPtr& pComponent, int32_t offset);

        SR_NODISCARD bool HasComponent(const ComponentPtr& pComponent) const;
        SR_NODISCARD int32_t GetComponentIndex(const ComponentPtr& pComponent) const;
        SR_NODISCARD uint32_t GetComponentsCount() const noexcept;
        SR_NODISCARD const std::vector<ComponentPtr>& GetComponents() const noexcept;

        virtual bool AddComponent(const ComponentPtr& pComponent);

        template<typename T> SR_HTYPES_NS::SharedPtr<T> AddComponent() {
            ComponentPtr pComponent = CreateComponentByName(T::GetClassStaticName());
            if (!pComponent) {
                SR_ERROR("IComponentable::AddComponent() : failed to create component of type: {}", T::GetClassStaticName());
                return nullptr;
            }
            if (!AddComponent(pComponent)) {
                SR_ERROR("IComponentable::AddComponent() : failed to add component of type: {}", T::GetClassStaticName());
                return nullptr;
            }
            return DynamicPointerCast<T>(pComponent);
        }

        void RemoveComponents();
        virtual bool RemoveComponent(const ComponentPtr& pComponent);
        virtual bool ContainsComponent(const std::string& name);

        virtual void ForEachComponent(const std::function<bool(const ComponentPtr&)>& fun) const;
        virtual void ForEachComponent(const std::function<bool(ComponentPtr&)>& fun);

        template<typename T> SR_HTYPES_NS::SharedPtr<T> GetComponent() {
            return DynamicPointerCast<T>(GetComponent(T::GetClassStaticName()));
        }

        virtual void OnPriorityChanged();
        virtual void OnMatrixDirty();

        SR_NODISCARD virtual bool IsPrefab() const noexcept { return false; }

        SR_NODISCARD virtual ScenePtr GetScene() const;

        void OnPostLoad() override;

    private:
        static ComponentPtr CreateComponentByName(SR_UTILS_NS::StringAtom name);

    protected:
        void DestroyComponent(const ComponentPtr& pComponent);

    protected:
        /// @property @hidden @dontSaveTags(Inspector)
        /// @propertyCondition(!This.IsPrefab())
        std::vector<ComponentPtr> m_components;

    private:
        bool m_hasNotAttachedComponents = false;
        bool m_dirty = true;

    };
}

#endif //SR_ENGINE_I_COMPONENTABLE_H
