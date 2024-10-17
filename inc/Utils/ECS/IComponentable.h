//
// Created by Monika on 12.12.2022.
//

#ifndef SR_ENGINE_I_COMPONENTABLE_H
#define SR_ENGINE_I_COMPONENTABLE_H

#include <Utils/Types/Marshal.h>
#include <Utils/Types/Vector.h>
#include <Utils/ECS/Entity.h>
#include <Utils/ECS/Component.h>

namespace SR_WORLD_NS {
    class Scene;
}

namespace SR_UTILS_NS {
    class IComponentable : public Entity {
        SR_CLASS()
    public:
        using ScenePtr = SR_WORLD_NS::Scene*;
        using OriginType = IComponentable;

    protected:
        IComponentable() = default;
        ~IComponentable() override;

    public:
        SR_NODISCARD SR_FORCE_INLINE const std::vector<Component::Ptr>& GetComponents() const noexcept { return m_components; }
        SR_NODISCARD bool IsDirty() const noexcept;

    public:
        SR_NODISCARD SR_HTYPES_NS::Marshal::Ptr SaveComponents(SavableContext data) const;

        /// Вызывает OnAttached у компонентов загруженных через LoadComponent
        virtual bool PostLoad(bool force);

        virtual void CheckActivity(bool force) noexcept;

        virtual void Awake(bool force, bool isPaused) noexcept;
        virtual void Start(bool force) noexcept;

        SR_NODISCARD virtual bool IsActive() const noexcept { return true; }
        SR_NODISCARD virtual bool IsDestroyed() const noexcept { return false; }

        virtual bool SetDirty(bool dirty);

        virtual Component::Ptr GetOrCreateComponent(const std::string& name);
        virtual Component::Ptr GetOrCreateComponent(StringAtom name);
        virtual Component::Ptr GetComponent(const std::string& name);
        virtual Component::Ptr GetComponent(StringAtom name);

        virtual bool AddComponent(const Component::Ptr& pComponent);

        virtual bool RemoveComponent(const Component::Ptr& pComponent);
        virtual bool ContainsComponent(const std::string& name);

        virtual void ForEachComponent(const std::function<bool(const Component::Ptr&)>& fun) const;
        virtual void ForEachComponent(const std::function<bool(Component::Ptr&)>& fun);

        virtual void DestroyComponents();

        template<typename T> SR_HTYPES_NS::SharedPtr<T> GetComponent() {
            return GetComponent(T::COMPONENT_NAME).template DynamicCast<T>();
        }

        virtual void OnPriorityChanged();
        virtual void OnMatrixDirty();

        SR_NODISCARD virtual ScenePtr GetScene() const;

    protected:
        void DestroyComponent(const Component::Ptr& pComponent);

    protected:
        /// @property
        std::vector<Component::Ptr> m_components;

    private:
        bool m_hasNotAttachedComponents = false;
        bool m_dirty = true;

    };
}

#endif //SR_ENGINE_I_COMPONENTABLE_H
