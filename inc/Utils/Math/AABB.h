//
// Created by Monika on 28.06.2025.
//

#ifndef SR_ENGINE_COMMON_MATH_AABB_H
#define SR_ENGINE_COMMON_MATH_AABB_H

#include <Utils/Math/Vector3.h>

namespace SR_MATH_NS {
    struct SR_COMMON_DLL_API AABB {
        FVector3 min = FVector3(0.f, 0.f, 0.f);
        FVector3 max = FVector3(0.f, 0.f, 0.f);

        AABB();
        AABB(const FVector3& min, const FVector3& max);

        SR_NODISCARD bool operator==(const AABB& other) const noexcept;
        SR_NODISCARD bool operator!=(const AABB& other) const noexcept;

        SR_NODISCARD FVector3 Min() const noexcept { return min; }
        SR_NODISCARD FVector3 Max() const noexcept { return max; }

        void SetMin(const FVector3& _min) noexcept { min = _min; }
        void SetMax(const FVector3& _max) noexcept { max = _max; }

        SR_NODISCARD bool IsValid() const noexcept;
        SR_NODISCARD bool IsEmpty() const noexcept;
        SR_NODISCARD FVector3 GetCenter() const noexcept;
        SR_NODISCARD FVector3 GetSize() const noexcept;
        SR_NODISCARD FVector3 GetExtends() const noexcept;

        SR_NODISCARD static AABB UnitCube(float_t size = 1.f) noexcept;

    };
}

#endif //SR_ENGINE_COMMON_MATH_AABB_H
