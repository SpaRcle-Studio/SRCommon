//
// Created by Monika on 28.06.2025.
//

#ifndef SR_ENGINE_COMMON_MATH_AABB_H
#define SR_ENGINE_COMMON_MATH_AABB_H

#include <Utils/Math/Vector3.h>

namespace SR_MATH_NS {
    struct SR_RENDERER_DLL_API AABB {
        FVector3 min = FVector3(0.f, 0.f, 0.f);
        FVector3 max = FVector3(0.f, 0.f, 0.f);

        AABB();
        AABB(const FVector3& min, const FVector3& max);

        SR_NODISCARD bool IsValid() const noexcept;
        SR_NODISCARD bool IsEmpty() const noexcept;
        SR_NODISCARD FVector3 GetCenter() const noexcept;

    };
}

#endif //SR_ENGINE_COMMON_MATH_AABB_H
