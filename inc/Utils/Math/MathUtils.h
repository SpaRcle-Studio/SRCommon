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
        FVector2 MultiplyFVector2Float(const FVector2& a, float_t b) const;

        /// @method @evaluate
        FVector3 MakeFVector3(float_t x, float_t y, float_t z) const;
        /// @method @evaluate
        FVector3 Normalize(const FVector3& vector3) const;
        /// @method @evaluate
        FVector3 MultiplyFVector3(const FVector3& a, const FVector3& b) const;
        /// @method @evaluate
        FVector3 MultiplyFVector3Float(const FVector3& a, float_t b) const;
        /// @method @evaluate
        FVector3 SumFVector3(const FVector3& a, const FVector3& b) const;

        /// @method @evaluate
        float_t Length(const FVector3& vector3) const;
        /// @method @evaluate
        float_t ClampFloat(float_t value, float_t min, float_t max) const;
        /// @method @evaluate
        float_t SumFloat(float_t a, float_t b) const { return a + b; }
        /// @method @evaluate
        float_t SubtractFloat(float_t a, float_t b) const { return a - b; }
        /// @method @evaluate
        float_t BoolToFloat(bool value) const { return value ? 1.f : 0.f; }
        /// @method @evaluate
        float_t MultiplyFloat(float_t a, float_t b) const { return a * b; }
        /// @method @evaluate
        float_t GetFVector3X(const FVector3& v) const { return v.x; }
        /// @method @evaluate
        float_t GetFVector3Y(const FVector3& v) const { return v.y; }
        /// @method @evaluate
        float_t GetFVector3Z(const FVector3& v) const { return v.z; }
        /// @method @evaluate
        float_t GetFVector2X(const FVector2& v) const { return v.x; }
        /// @method @evaluate
        float_t GetFVector2Y(const FVector2& v) const { return v.y; }
        /// @method @evaluate
        float_t SwitchFloat(float_t trueValue, float_t falseValue, bool condition) const { return condition ? trueValue : falseValue; }

        /// @method @evaluate
        bool BoolAnd(bool a, bool b) const { return a && b; }
        /// @method @evaluate
        bool BoolOr(bool a, bool b) const { return a || b; }
        /// @method @evaluate
        bool BoolNot(bool value) const { return !value; }
        /// @method @evaluate
        bool BitAnd(uint64_t a, uint64_t b) const { return (a & b) != 0; }
        /// @method @evaluate
        bool BitOr(uint64_t a, uint64_t b) const { return (a | b) != 0; }
        /// @method @evaluate
        bool BitXor(uint64_t a, uint64_t b) const { return (a ^ b) != 0; }

        /// @method @evaluate
        bool IsFloatGreater(float_t a, float_t b) const { return a > b; }
        /// @method @evaluate
        bool IsFloatLess(float_t a, float_t b) const { return a < b; }

    };
}

#endif //SR_ENGINE_COMMON_MATH_UTILS_H
