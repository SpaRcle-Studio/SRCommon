//
// Created by Monika on 26.08.2026.
//

#include <Utils/Math/MathUtils.h>

#include <Codegen/MathUtils.generated.hpp>

namespace SR_MATH_NS {
    SR_MAYBE_UNUSED auto&& mathUtilsInstance = MathUtils::Instance();

    FVector3 MathUtils::MakeFVector3(float_t x, float_t y, float_t z) const {
        return FVector3(x, y, z);
    }

    FVector3 MathUtils::MultiplyFVector3(const FVector3& a, const FVector3& b) const {
        return a * b;
    }

    FVector3 MathUtils::SumFVector3(const FVector3& a, const FVector3& b) const {
        return a + b;
    }
}