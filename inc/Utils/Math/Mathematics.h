//
// Created by Nikita on 17.11.2020.
//

#ifndef SR_ENGINE_UTILS_MATHEMATICS_H
#define SR_ENGINE_UTILS_MATHEMATICS_H

#include <Utils/stdInclude.h>

#ifndef M_PI
    #define M_PI (std::numbers::pi)
#endif

#ifndef SR_PI
    #define SR_PI M_PI
#endif

#define SR_RAD(x) (x * (SR_PI / 180.0))
#define SR_DEG(x) (x * (180.0 / SR_PI))

#define RAD(x) (x * (SR_PI / 180.0))
#define DEG(x) (x * (180.0 / SR_PI))

#define SR_DEG_2_RAD (SR_PI * 2.f / 360.f)
#define SR_RAD_2_DEG (1.f / SR_DEG_2_RAD)

#define DegreesToRad(angle) (angle * M_PI / 180)
#define RadToDegrees(angle) (angle * 180 / M_PI)

#define CMP_EPSILON (0.00001)
#define CMP_BIG_EPSILON (0.001)

#define SR_EPSILON (0.00001)
#define SR_BIG_EPSILON (0.001)
#define SR_LARGE_EPSILON (0.00005)
#define SR_EPSILON_NORMAL_SQRT (1e-15)

#define CMP_EPSILON2 (CMP_EPSILON * CMP_EPSILON)
#define UNIT_EPSILON (0.00001)

#define CMP_NORMALIZE_TOLERANCE (0.000001)
#define CMP_POINT_IN_PLANE_EPSILON (0.00001)

#define Math_SQRT12 (0.7071067811865475244008443621048490)
#define Math_SQRT2 (1.4142135623730950488016887242)
#define Math_LN2 (0.6931471805599453094172321215)
#define Math_TAU (6.2831853071795864769252867666)
#define Math_E (2.7182818284590452353602874714)
#define Math_INF (INFINITY)

#define SR_INF std::numeric_limits<double_t>::infinity()
#define SR_NAN NAN
#define SR_INT16_MAX std::numeric_limits<int16_t>::max()
#define SR_UINT16_MAX std::numeric_limits<uint16_t>::max()
#define SR_UINT8_MAX std::numeric_limits<uint8_t>::max()
#define SR_INT8_MAX std::numeric_limits<int8_t>::max()
#define SR_INT32_MAX std::numeric_limits<int32_t>::max()
#define SR_INT32_MIN std::numeric_limits<int32_t>::min()
#define SR_UINT32_MAX std::numeric_limits<uint32_t>::max()
#define SR_INT64_MAX std::numeric_limits<int64_t>::max()
#define SR_UINT64_MAX std::numeric_limits<uint64_t>::max()
#define SR_UINTPTR_MAX std::numeric_limits<uintptr_t>::max()
#define SR_DOUBLE_MAX std::numeric_limits<double_t>::max()
#define SR_FLOAT_MAX std::numeric_limits<float_t>::max()

#define SR_EARTH_GRAVITY_CONST (9.81) // Gravitational Constant (m/s^2)

#define SR_SQUARE(value) (value * value)
#define SR_POW(x, n) std::pow(x, n)
#define SR_SQRT(x) std::sqrt(x)
#define SR_SIN(x) std::sin(x)
#define SR_COS(x) std::cos(x)
#define SR_ARC_SIN(x) std::asin(x)
#define SR_ARC_COS(x) std::acos(x)

#define SR_ABS(x) (std::abs(x))
#define SR_MAX(a, b) (a > b ? a : b)
#define SR_MIN(a, b) (a < b ? a : b)
#define SR_CLAMP(x, lower, upper) (SR_MIN(upper, SR_MAX(x, lower)))

#define RAD3(v) glm::vec3(RAD(v.x), RAD(v.y), RAD(v.z))
#define DEG3(v) glm::vec3(DEG(v.x), DEG(v.y), DEG(v.z))

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SR_MATH_NS {
    static SR_FORCE_INLINE bool IsNumber(std::string_view str) {
        bool hasDot = false;
        bool hasNumber = false;

        for (const char* c = str.data(); *c; c++) {
            if (*c == '-' && c == str.data()) {
                continue;
            }

            if (*c == '.' || *c == ',') {
                if (hasDot) {
                    return false;
                }
                hasDot = true;
                continue;
            }

            if (*c < '0' || *c > '9') {
                return false;
            }
            hasNumber = true;
        }

        return hasNumber;
    }

    static SR_FORCE_INLINE bool IsIntegerNumber(std::string_view str) {
        bool hasNumber = false;
        for (const char* c = str.data(); *c; c++) {
            if (*c == '-' && c == str.data()) {
                continue;
            }
            if (*c < '0' || *c > '9') {
                return false;
            }
            hasNumber = true;
        }

        return hasNumber;
    }

    typedef float Unit; //! can broke render

    constexpr double_t DoubleMAX = std::numeric_limits<double_t>::max();
    constexpr float_t  FloatMAX  = std::numeric_limits<float_t>::max();
    constexpr int32_t  Int32MAX  = std::numeric_limits<int32_t>::max();

    constexpr Unit UnitMAX = FloatMAX;

    static SR_FORCE_INLINE Unit Sign(Unit value) {
        return value >= static_cast<Unit>(0) ? static_cast<Unit>(1) : static_cast<Unit>(-1);
    }

    static SR_FORCE_INLINE double_t Lerp(double_t t, double_t a, double_t b) {
        return a + (b + -a) * t;
    }

    template<typename MaskLeft, typename MakeRight> static SR_FORCE_INLINE bool IsMaskIncludedSubMask(MaskLeft mask, MakeRight subMask) {
        return (subMask & mask) == subMask;
    }

    template<typename MaskLeft, typename MakeRight> static SR_FORCE_INLINE bool IsAnyMaskIncludedSubMask(MaskLeft mask, std::initializer_list<MakeRight> subMasks) {
        bool included = false;

        for (auto&& subMask : subMasks) {
            included |= IsMaskIncludedSubMask(mask, subMask);
        }

        return included;
    }

    static SR_FORCE_INLINE double_t SR_FASTCALL FastSqrt(double_t value) {
        return std::sqrt(value);
    }

    static SR_FORCE_INLINE bool is_equal_approx(const Unit a, const Unit b) noexcept {
        // Check for exact equality first, required to handle "infinity" values.
        if (a == b) {
            return true;
        }
        // Then check for approximate equality.
        double tolerance = CMP_EPSILON * abs(a);
        if (tolerance < CMP_EPSILON) {
            tolerance = CMP_EPSILON;
        }
        return abs(a - b) < tolerance;
    }
    static SR_FORCE_INLINE bool is_equal_approx(const Unit a, const Unit b, const Unit tolerance) {
        // Check for exact equality first, required to handle "infinity" values.
        if (a == b) {
            return true;
        }
        // Then check for approximate equality.
        return abs(a - b) < tolerance;
    }

    template<typename T> constexpr bool IsFloat() {
        if (!IsVolatile<T>()) {
            return IsFloat<volatile T>();
        }

        return
            std::is_same_v<T, volatile float> ||
            std::is_same_v<T, volatile double> ||
            std::is_same_v<T, volatile float_t> ||
            std::is_same_v<T, volatile double_t> ||
            std::is_same_v<T, volatile Unit>;
    }

    template<typename T> constexpr bool IsInt() {
        if (!IsVolatile<T>()) {
            return IsInt<volatile T>();
        }

        return
            std::is_same_v<T, volatile int> ||
            std::is_same_v<T, volatile unsigned short> ||
            std::is_same_v<T, volatile short> ||
            std::is_same_v<T, volatile unsigned int> ||
            std::is_same_v<T, volatile unsigned> ||
            std::is_same_v<T, volatile long> ||
            std::is_same_v<T, volatile long long> ||
            std::is_same_v<T, volatile unsigned long long> ||
            std::is_same_v<T, volatile unsigned long> ||
            std::is_same_v<T, volatile int64_t> ||
            std::is_same_v<T, volatile uint64_t> ||
            std::is_same_v<T, volatile int32_t> ||
            std::is_same_v<T, volatile uint32_t> ||
            std::is_same_v<T, volatile int8_t> ||
            std::is_same_v<T, volatile uint8_t> ||
            std::is_same_v<T, volatile int16_t> ||
            std::is_same_v<T, volatile uint16_t>;
    }

    template<typename T> constexpr bool IsNumber() {
        if (!IsVolatile<T>()) {
            return IsNumber<volatile T>();
        }

        return IsInt<T>() || IsFloat<T>() || std::is_same_v<T, volatile bool>;
    }
}

#define SR_EQUALS(a, b) (SR_MATH_NS::is_equal_approx(a, b))
#define SR_EQUALS_T(a, b, tolerance) (SR_MATH_NS::is_equal_approx(a, b, tolerance))
#define Deg180InRad ((SR_MATH_NS::Unit)M_PI)
#define Deg90InRad  ((SR_MATH_NS::Unit)RAD(90.0))

#endif //SR_ENGINE_UTILS_MATHEMATICS_H
