//
// Created by Monika on 26.01.2026.
//

#include <Utils/ECS/TransformUtils.h>
#include <Utils/ECS/GameObject.h>

namespace SR_UTILS_NS {
    Transform* ExtractTransform(const SceneObject* pSO) {
        if (!pSO) {
            return nullptr;
        }
        if (pSO->GetSceneObjectType() != SceneObjectType::GameObject) {
            return nullptr;
        }
        auto&& pGameObject = static_cast<const GameObject*>(pSO);
        return const_cast<Transform*>(pGameObject->GetTransform().Get());
    }

    SR_UTILS_NS::StringAtom GetTransformTypeName(const Transform* pTransform) {
        return pTransform ? pTransform->GetMeta()->GetFactoryName() : SR_UTILS_NS::StringAtom();
    }
}