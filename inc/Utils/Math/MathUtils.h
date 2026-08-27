//
// Created by Monika on 26.08.2026.
//

#ifndef SR_ENGINE_COMMON_MATH_UTILS_H
#define SR_ENGINE_COMMON_MATH_UTILS_H

#include <Utils/Common/Singleton.h>
#include <Utils/TypeTraits/SRClass.h>

namespace SR_MATH_NS {
    /// @noCopyable @noMovable
    class SR_COMMON_DLL_API MathUtils : public SR_UTILS_NS::Singleton<MathUtils>, public SRClass {
        SR_REGISTER_SINGLETON(MathUtils);
        SR_CLASS()
    public:
        /// @method @evaluate
        FVector3 MakeFVector3(float_t x, float_t y, float_t z) const;
        /// @method @evaluate
        FVector3 MultiplyFVector3(const FVector3& a, const FVector3& b) const;
        /// @method @evaluate
        FVector3 SumFVector3(const FVector3& a, const FVector3& b) const;
        /// @method @evaluate
        float_t BoolToFloat(bool value) const { return value ? 1.f : 0.f; }
        /// @method @evaluate
        float_t MultiplyFloat(float_t a, float_t b) const { return a * b; }

    };
}

#endif //SR_ENGINE_COMMON_MATH_UTILS_H
