//
// Created by Monika on 26.08.2026.
//

#include <Utils/Math/MathUtils.h>

#include <Codegen/MathUtils.generated.hpp>

namespace SR_MATH_NS {
    SR_MAYBE_UNUSED auto&& mathUtilsInstance = MathUtils::Instance();

    FVector2 MathUtils::MultiplyFVector2Float(const FVector2& a, float_t b) const {
        return a * b;
    }

    FVector3 MathUtils::MakeFVector3(float_t x, float_t y, float_t z) const {
        return FVector3(x, y, z);
    }

    FVector3 MathUtils::MultiplyFVector3(const FVector3& a, const FVector3& b) const {
        return a * b;
    }

    FVector3 MathUtils::SumFVector3(const FVector3& a, const FVector3& b) const {
        return a + b;
    }

    FVector3 MathUtils::Normalize(const FVector3& vector3) const {
        return vector3.Normalize();
    }

    float_t MathUtils::Length(const FVector3& vector3) const {
        return vector3.Length();
    }

    FVector3 MathUtils::MultiplyFVector3Float(const FVector3& a, float_t b) const {
        return a * b;
    }

    float_t MathUtils::ClampFloat(float_t value, float_t min, float_t max) const {
        return Clamp(value, min, max);
    }
}