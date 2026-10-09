//
// Created by innerviewer on 2/13/2023.
//

#ifndef SR_ENGINE_RAYCASTHIT_H
#define SR_ENGINE_RAYCASTHIT_H

#include <Utils/Math/Vector3.h>
#include <Utils/Serialization/Serializable.h>

namespace SR_UTILS_NS {
    class Entity;
    class Component;
    class SceneObject;

    struct LayerMask : public Serializable {
        SR_STRUCT()

        LayerMask() = default;

        /// @property
        uint64_t mask = 0xFFFFFFFFFFFFFFFF;

        static LayerMask Any() {
            static LayerMask allMask = LayerMask();
            return allMask;
        }
    };

    struct RayCastHit : public Serializable {
        SR_STRUCT()

        SR_NODISCARD bool operator==(const RayCastHit& other) const {
            return position == other.position && normal == other.normal && distance == other.distance && pHandlerEntity == other.pHandlerEntity;
        }

        SR_NODISCARD bool operator!=(const RayCastHit& other) const {
            return !(*this == other);
        }

        void* pHandlerEntity = nullptr;

        /// @method @evaluate
        SR_NODISCARD SR_HTYPES_NS::SharedPtr<Entity> GetEntity() const;
        /// @method @evaluate
        SR_NODISCARD SR_HTYPES_NS::SharedPtr<Component> GetComponent() const;
        /// @method @evaluate
        SR_NODISCARD SR_HTYPES_NS::SharedPtr<SceneObject> GetSceneObject() const;

        /// @property
        SR_MATH_NS::FVector3 position;
        /// @property
        SR_MATH_NS::FVector3 normal;
        /// @property
        float_t distance;

        /// @method @evaluate
        SR_NODISCARD SR_MATH_NS::FVector3 GetRayEndPoint() const {
            return position + normal * distance;
        }
    };

    using RayCastHits = SR_UTILS_NS::Vector<SR_UTILS_NS::RayCastHit>;
}

#endif //SR_ENGINE_RAYCASTHIT_H
