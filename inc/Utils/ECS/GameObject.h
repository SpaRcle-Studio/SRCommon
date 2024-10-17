//
// Created by Nikita on 27.11.2020.
//

#ifndef SR_ENGINE_GAME_OBJECT_H
#define SR_ENGINE_GAME_OBJECT_H

#include <Utils/ECS/SceneObject.h>
#include <Utils/ECS/TagManager.h>
#include <Utils/ECS/Prefab.h>

#include <Utils/Math/Vector3.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/SafeVariable.h>

namespace SR_UTILS_NS {
    class Transform3D;
    class Transform;
    class Component;

    class SR_DLL_EXPORT GameObject final : public SceneObject {
        SR_ENTITY_SET_VERSION(1009);
        using Super = SceneObject;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<GameObject>;

    public:
        explicit GameObject(ObjectNameT name, Transform* pTransform = nullptr);
        ~GameObject() override;

        static GameObject::Ptr Load(SR_HTYPES_NS::Marshal& marshal, const ScenePtr& scene);

    public:
        SR_NODISCARD SceneObjectType GetSceneObjectType() const noexcept override { return SceneObjectType::GameObject; }
        SR_NODISCARD SceneObject::Ptr Copy(const ScenePtr& pScene, const SceneObject::Ptr& pObject) const override;
        SR_NODISCARD SR_HTYPES_NS::Marshal::Ptr SaveLegacy(SavableContext data) const override;
        SR_NODISCARD Transform* GetTransform() const noexcept { return m_transform; }
        SR_NODISCARD Transform* GetParentTransform() const noexcept;

        SR_NODISCARD GameObject::Ptr CreateChild(StringAtom name);
        SR_NODISCARD GameObject::Ptr GetOrCreateChild(StringAtom name);

        void OnAttached() override;
        void SetTransform(Transform* transform);

    protected:
        void OnHierarchyChanged() override;

    private:
        Transform* m_transform = nullptr;

    };
}

#endif //SR_ENGINE_GAME_OBJECT_H
