//
// Created by Nikita on 27.11.2020.
//

#ifndef SR_ENGINE_GAME_OBJECT_H
#define SR_ENGINE_GAME_OBJECT_H

#include <Utils/ECS/SceneObject.h>
#include <Utils/ECS/TagManager.h>
#include <Utils/ECS/Prefab.h>
#include <Utils/ECS/Transform.h>

#include <Utils/Math/Vector3.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/SafeVariable.h>

namespace SR_UTILS_NS {
    class Transform3D;
    class Transform;
    class Component;

    class SR_DLL_EXPORT GameObject final : public SceneObject {
        SR_ENTITY_SET_VERSION(1010);
        SR_CLASS()
        using Super = SceneObject;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<GameObject>;

    public:
        ~GameObject() override;

    public:
        SR_NODISCARD SceneObjectType GetSceneObjectType() const noexcept override { return SceneObjectType::GameObject; }
        SR_NODISCARD SR_HTYPES_NS::SharedPtr<Transform> GetTransform() const noexcept;
        SR_NODISCARD SR_HTYPES_NS::SharedPtr<Transform> GetParentTransform() const noexcept;

        SR_NODISCARD GameObject::Ptr CreateChild(StringAtom name);
        SR_NODISCARD GameObject::Ptr GetOrCreateChild(StringAtom name);

        void OnAttached() override;
        void SetTransform(const SR_HTYPES_NS::SharedPtr<Transform>& pTransform);

    protected:
        void OnHierarchyChanged() override;

    private:
        /// @property @getter(GetTransform) @setter(SetTransform)
        SR_HTYPES_NS::SharedPtr<Transform> m_transform;

    };
}

#endif //SR_ENGINE_GAME_OBJECT_H
