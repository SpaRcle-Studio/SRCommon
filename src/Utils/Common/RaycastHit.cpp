//
// Created by Monika on 01.10.2026.
//

#include <Utils/Common/RaycastHit.h>
#include <Utils/ECS/SceneObject.h>
#include <Utils/ECS/Component.h>

#include <Codegen/RaycastHit.generated.hpp>

namespace SR_UTILS_NS {
    SR_HTYPES_NS::SharedPtr<Entity> RayCastHit::GetEntity() const {
        return SR_HTYPES_NS::SharedPtr<Entity>(static_cast<Entity*>(pHandlerEntity));
    }

    SR_HTYPES_NS::SharedPtr<Component> RayCastHit::GetComponent() const {
        return GetEntity().DynamicCast<Component>();
    }

    SR_HTYPES_NS::SharedPtr<SceneObject> RayCastHit::GetSceneObject() const {
        if (auto&& pComponent = GetComponent()) {
            return pComponent->GetSceneObject();
        }
        return nullptr;
    }
}