//
// Created by Nikita on 27.11.2020.
//

#ifndef SR_ENGINE_UTILS_COMPONENT_H
#define SR_ENGINE_UTILS_COMPONENT_H

#include <Utils/ECS/Entity.h>
#include <Utils/ECS/TransformUtils.h>

/**
 * Awake -> OnEnabled -> Start -> Update -> FixedUpdate
 */

namespace SR_HTYPES_NS {
    class DataStorage;
}

namespace SR_WORLD_NS {
    class Scene;
}

namespace SR_UTILS_NS {
    struct CollisionData;

    class SceneObject;
    class ComponentManager;
    class Component;
    class IComponentable;
    class Transform3D;
    class Transform;
    class GameObject;

    class SR_COMMON_DLL_API Component : public Entity {
        SR_CLASS()
        friend class GameObject;
        friend class IComponentable;
        friend class ComponentManager;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<Component>;
        using OriginType = Component;
        using ScenePtr = SR_WORLD_NS::Scene*;
        using GameObjectPtr = SR_HTYPES_NS::SharedPtr<GameObject>;
        using SceneObjectPtr = SR_HTYPES_NS::SharedPtr<SceneObject>;
    public:
        Component();
        ~Component() override;

    public:
        virtual void OnMatrixDirty();
        virtual void OnMaskDirty();
        virtual void OnPriorityChanged();

        /// Вызывается после добавления компонента к игровому объекту
        virtual void OnAttached();
        /// Вызывается когда компонент убирается с объекта, но до OnDestroy и только если был OnAttached
        virtual void OnDetached();
        /// Вызывается когда компонент убирается с объекта, либо объект уничтожается. Может произойти отложенно.
        virtual void OnDestroy();

        virtual void OnEnable();
        virtual void OnDisable();

        virtual void Awake();
        virtual void Start();
        virtual void Update(float_t dt);
        virtual void FixedUpdate();
        virtual void LateUpdate();

        virtual void OnBeforeLayerChanged();
        virtual void OnLayerChanged();

        virtual void OnCollisionEnter(const CollisionData& data);
        virtual void OnCollisionStay(const CollisionData& data);
        virtual void OnCollisionExit(const CollisionData& data);

        virtual void OnTriggerEnter(const CollisionData& data);
        virtual void OnTriggerStay(const CollisionData& data);
        virtual void OnTriggerExit(const CollisionData& data);

    public:
        void CheckActivity();

        void Detach();

        void SetEnabled(bool value);
        void SetIndexIdSceneUpdater(int32_t index);

        SR_NODISCARD bool IsComponentValid() const noexcept;
        SR_NODISCARD bool IsAttached() const noexcept;

        /// Активен и компонент и его родительский объект
        SR_NODISCARD virtual bool IsUpdatable() const noexcept;
        /// Активен и компонент и его родительский объект
        SR_NODISCARD virtual bool IsActive() const noexcept;
        /// Активен сам компонент, независимо от объекта
        SR_NODISCARD virtual bool IsEnabled() const noexcept;

        SR_NODISCARD virtual bool IsAwake() const noexcept;
        SR_NODISCARD virtual bool IsStarted() const noexcept;

        /// Запущена ли сцена
        SR_NODISCARD bool IsPlayingMode() const;
        /// На паузе ли сцена (если запущена)
        SR_NODISCARD bool IsPausedMode() const;

        SR_NODISCARD virtual bool ExecuteInEditMode() const;
        SR_NODISCARD virtual SR_MATH_NS::FVector3 GetBarycenter() const;
        SR_NODISCARD Component* BaseComponent() noexcept;
        SR_NODISCARD IComponentable* GetParent() const;
        SR_NODISCARD IComponentable* TryGetParent() const;
        SR_NODISCARD ScenePtr GetScene() const;
        SR_NODISCARD bool HasScene() const;
        SR_NODISCARD GameObjectPtr GetGameObject() const;
        SR_NODISCARD const SceneObjectPtr& GetSceneObject() const;
        SR_NODISCARD ScenePtr TryGetScene() const;
        SR_NODISCARD SceneObjectPtr GetRoot() const;
        SR_NODISCARD Transform* GetTransform() const noexcept;
        SR_NODISCARD int32_t GetIndexInSceneUpdater() const noexcept;
        SR_NODISCARD bool HasParent() const;

        SR_NODISCARD Component::Ptr CloneComponent() const;

        template<typename T> SR_NODISCARD T* GetTransformAs() const {
            auto&& pTransform = GetTransform();
            if (!pTransform) {
                return nullptr;
            }

            if (T::GetClassStaticName() == GetTransformTypeName(pTransform)) {
                return static_cast<T*>(pTransform);
            }
            return nullptr;
        }

    protected:
        void SetParent(IComponentable* pParent);

    protected:
        bool m_isAttached = false;
        bool m_isActive = false;
        bool m_isAwake = false;
        bool m_isStarted = false;

        /// @property @hidden @setter(SetEnabled)
        bool m_isEnabled = true;

        int32_t m_indexInSceneUpdater = SR_ID_INVALID;

        SceneObjectPtr m_sceneObject;
        IComponentable* m_parent = nullptr;
        SR_WORLD_NS::Scene* m_scene = nullptr;

    };
}


#endif //SR_ENGINE_UTILS_COMPONENT_H
