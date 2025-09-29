//
// Created by Monika on 28.06.2025.
//

#include <Utils/Math/AABB.h>

namespace SR_MATH_NS {
    AABB::AABB() = default;

    AABB::AABB(const FVector3& min, const FVector3& max)
        : min(min)
        , max(max)
    { }

    bool AABB::IsValid() const noexcept {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }

    bool AABB::IsEmpty() const noexcept {
        return !IsValid();
    }

    FVector3 AABB::GetCenter() const noexcept {
        return (min + max) * 0.5f;
    }

    FVector3 AABB::GetSize() const noexcept {
        return max - min;
    }

    AABB AABB::UnitCube(const float_t size) noexcept {
        return AABB(FVector3(-size), FVector3(size));
    }

    FVector3 AABB::GetExtends() const noexcept {
        return GetSize() * 0.5f;
    }
}