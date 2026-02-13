//
// Created by Monika on 26.01.2026.
//

#ifndef SR_ENGINE_COMMON_TRANSFORM_UTILS_H
#define SR_ENGINE_COMMON_TRANSFORM_UTILS_H

#include <Utils/TypeTraits/SRClassMeta.h>
#include <Utils/Types/StringAtom.h>

namespace SR_UTILS_NS {
    class SceneObject;
    class Transform3D;
    class TransformRect;
    class Transform;

    SR_NODISCARD Transform* ExtractTransform(const SceneObject* pSO);
    SR_NODISCARD SR_UTILS_NS::StringAtom GetTransformTypeName(const Transform* pTransform);

    template<typename T> SR_NODISCARD T* ExtractTransformAs(const SceneObject* pSO) {
        auto&& pTransform = ExtractTransform(pSO);
        if (pTransform && GetTransformTypeName(pTransform) != T::GetClassStaticName()) {
            return nullptr;
        }
        return static_cast<T*>(pTransform);
    }
}

#endif //SR_ENGINE_COMMON_TRANSFORM_UTILS_H
