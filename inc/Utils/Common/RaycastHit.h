//
// Created by innerviewer on 2/13/2023.
//

#ifndef SR_ENGINE_RAYCASTHIT_H
#define SR_ENGINE_RAYCASTHIT_H

#include <Utils/Math/Vector3.h>

namespace SR_UTILS_NS {
    struct RaycastHit {
        void* pHandler = nullptr;
        SR_MATH_NS::FVector3 position;
        SR_MATH_NS::FVector3 normal;
        float_t distance;

        SR_NODISCARD SR_MATH_NS::FVector3 GetRayEndPoint() const {
            return position + normal * distance;
        }
    };
}

#endif //SR_ENGINE_RAYCASTHIT_H
