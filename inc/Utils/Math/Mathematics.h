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

#define SR_RAD(x) SR_MATH_NS::ToRad(x)
#define SR_DEG(x) SR_MATH_NS::ToDeg(x)

#define RAD(x) SR_RAD(x)
#define DEG(x) SR_DEG(x)

#define SR_DEG_2_RAD (SR_PI * 2.f / 360.f)
#define SR_RAD_2_DEG (1.f / SR_DEG_2_RAD)

/// !!!!!!!!!!!!!!!! Устаревшие макросы, использовать новые которые блоком ниже !!!!!!!!!!!!!!!!
#define CMP_EPSILON (0.00001)
#define CMP_BIG_EPSILON (0.001)
#define SR_EPSILON (0.00001)
#define SR_BIG_EPSILON (0.001)
#define SR_EPSILON_NORMAL_SQRT (1e-15)
#define SR_LARGE_EPSILON (0.00005)
#define SR_FLOAT_EPSILON (1.192092896e-07F)
#define CMP_EPSILON2 (CMP_EPSILON * CMP_EPSILON)
#define UNIT_EPSILON (0.00001)
#define CMP_NORMALIZE_TOLERANCE (0.000001)
#define CMP_POINT_IN_PLANE_EPSILON (0.00001)

#define SR_SCALAR_EPSILON (1e-8f) /// ТОЛЬКО для чистых скалярных сравнений
#define SR_SMALL_NUMBER_EPSILON (1e-6) /// Скалярное сравнение векторов, нулевой Cross, нулевой вектор
#define SR_KINDA_SMALL_NUMBER_EPSILON (1e-4) /// Векторное сравнение векторов, проверка противоположности, коллинеарность

#define Math_SQRT12 (0.7071067811865475244008443621048490)
#define Math_SQRT2 (1.4142135623730950488016887242)
#define Math_LN2 (0.6931471805599453094172321215)
#define Math_TAU (6.2831853071795864769252867666)
#define Math_E (2.7182818284590452353602874714)
#define Math_INF (INFINITY)

#define SR_EARTH_GRAVITY_CONST (9.81) // Gravitational Constant (m/s^2)

#define SR_SQUARE(value) (value * value)
#define SR_POW(x, n) (std::pow(x, n))
#define SR_SQRT(x) (SR_MATH_NS::Sqrt(x))
#define SR_SIN(x) (SR_MATH_NS::Sin(x))
#define SR_COS(x) (SR_MATH_NS::Cos(x))
#define SR_ACOS(x) (SR_MATH_NS::Acos(x))
#define SR_ASIN(x) (SR_MATH_NS::Asin(x))
#define SR_TAN(x) (SR_MATH_NS::Tan(x))
#define SR_ATAN(x) (SR_MATH_NS::Atan(x))
#define SR_ATAN2(y, x) (SR_MATH_NS::Atan2(y, x))

#define SR_ABS(x) (SR_MATH_NS::Abs(x))
#define SR_MAX(a, b) (a > b ? a : b)
#define SR_MIN(a, b) (a < b ? a : b)
#define SR_CLAMP(x, lower, upper) SR_MATH_NS::Clamp(x, lower, upper)
#define SR_CLAMP01(x) SR_MATH_NS::Clamp(x, static_cast<decltype(x)>(0), static_cast<decltype(x)>(1))

#define RAD3(v) glm::vec3(RAD(v.x), RAD(v.y), RAD(v.z))
#define DEG3(v) glm::vec3(DEG(v.x), DEG(v.y), DEG(v.z))

#ifndef SR_ENGINE_CODEGEN_CLANG_PARSE_MODE
    #include <glm/glm.hpp>
    #include <glm/gtc/type_ptr.hpp>
    #include <glm/gtx/string_cast.hpp>
    #include <glm/gtc/quaternion.hpp>
#endif

namespace SR_MATH_NS {
    template<typename T> SR_NODISCARD constexpr std::enable_if_t<std::is_arithmetic_v<T>, T> Min(T a, T b) noexcept { return a < b ? a : b; }
    template<typename T> SR_NODISCARD constexpr std::enable_if_t<std::is_arithmetic_v<T>, T> Max(T a, T b) noexcept { return a > b ? a : b; }

    SR_COMMON_DLL_API extern bool HasSSE41();

    constexpr float Deg2Rad = 0.01745329f;
    constexpr float Rad2Deg = 57.29578f;

    static SR_FORCE_INLINE float Ceiling(float value) {
        return std::ceil(value);
    }

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

    template<typename T> SR_NODISCARD SR_FORCE_INLINE static constexpr T Sin(T value) {
       return static_cast<T>(sin(static_cast<double_t>(value)));
    }

    template<typename T> SR_NODISCARD SR_FORCE_INLINE static constexpr T Cos(T value) {
        return static_cast<T>(cos(static_cast<double_t>(value)));
    }

    template<typename T> SR_NODISCARD SR_FORCE_INLINE static constexpr T Tan(T value) {
        return static_cast<T>(tan(static_cast<double_t>(value)));
    }

    template<typename T> SR_NODISCARD SR_FORCE_INLINE static constexpr T Acos(T value) {
       return static_cast<T>(acos(static_cast<double_t>(value)));
    }

    template<typename T> SR_NODISCARD SR_FORCE_INLINE static constexpr T Asin(T value) {
        return static_cast<T>(asin(static_cast<double_t>(value)));
    }

    template<typename T> SR_NODISCARD SR_FORCE_INLINE static constexpr T Atan(T value) {
        return static_cast<T>(atan(static_cast<double_t>(value)));
    }

    template<typename T> SR_NODISCARD SR_FORCE_INLINE static constexpr T Atan2(T y, T x) {
        return static_cast<T>(atan2(static_cast<double_t>(y), static_cast<double_t>(x)));
    }

    template<typename T> SR_NODISCARD SR_FORCE_INLINE static constexpr T Sqrt(T value) {
        return static_cast<T>(sqrt(static_cast<double_t>(value)));
    }

    template<typename T> SR_NODISCARD SR_FORCE_INLINE static constexpr T Round(T value) {
        return static_cast<T>(std::round(static_cast<double_t>(value)));
    }

    template<typename T> SR_FORCE_INLINE T Clamp(T value, T lower, T upper) {
        if (std::is_same_v<T, float_t> || std::is_same_v<T, float>) {
            if (static_cast<double_t>(value) < static_cast<double_t>(lower)) {
                return lower;
            }
            else if (static_cast<double_t>(value) > static_cast<double_t>(upper)) {
                return upper;
            }
            else {
                return value;
            }
        }
        else {
            if (value < lower) {
                return lower;
            }
            else if (value > upper) {
                return upper;
            }
            else {
                return value;
            }
        }
    }

    template<typename T> SR_FORCE_INLINE T Sign(T value) {
        return static_cast<T>(static_cast<double_t>(value) > 0.0 ? 1.f : -1.f);
    }

    template<typename T> SR_FORCE_INLINE T ToRad(T degrees) {
        return static_cast<T>(static_cast<double_t>(degrees) * static_cast<double_t>(SR_PI / 180.0));
    }

    template<typename T> SR_FORCE_INLINE T ToDeg(T radians) {
        return static_cast<T>(static_cast<double_t>(radians) * static_cast<double_t>(180.0 / SR_PI));
    }

    static SR_FORCE_INLINE float_t TriangleAngle(float_t aLen, float_t aLen1, float_t aLen2) {
        float_t c = SR_CLAMP((aLen1 * aLen1 + aLen2 * aLen2 - aLen * aLen) / (aLen1 * aLen2) / 2.0f, -1.0f, 1.0f);
        return SR_ACOS(c);
    }

    template<typename T> SR_NODISCARD SR_FORCE_INLINE static constexpr T Abs(T value) {
        if constexpr (std::is_same_v<T, bool>) {
            return value;
        }
        else if constexpr (std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>) {
            return value;
        }
        else {
            return value >= static_cast<T>(0) ? value : -value;
        }
    }

    static SR_FORCE_INLINE Unit Sign(Unit value) {
        return value >= static_cast<Unit>(0) ? static_cast<Unit>(1) : static_cast<Unit>(-1);
    }

    static SR_FORCE_INLINE double_t LerpDeprecated(double_t t, double_t a, double_t b) {
        return a + (b + -a) * t;
    }

    static SR_FORCE_INLINE double_t Lerp(double_t a, double_t b, double_t t) {
        return a + (b - a) * t;
    }

    static SR_FORCE_INLINE double_t Mix(double_t a, double_t b, double_t t) {
        return a + (b - a) * t;
    }

    static SR_FORCE_INLINE float_t Mix(float_t a, float_t b, float_t t) {
        return static_cast<float_t>(Mix(static_cast<double_t>(a), static_cast<double_t>(b), static_cast<double_t>(t)));
    }

    template<typename T, bool = std::is_enum_v<T>>
    struct UnderlyingOrSelfImpl {
        using type = T;
    };

    template<typename T>
    struct UnderlyingOrSelfImpl<T, true> {
        using type = std::underlying_type_t<T>;
    };

    template<typename T>
    using UnderlyingOrSelf = typename UnderlyingOrSelfImpl<T>::type;

    template<typename MaskLeft, typename MaskRight> static SR_FORCE_INLINE bool IsMaskIncludedSubMask(MaskLeft mask, MaskRight subMask) {
        using TypeLeft  = UnderlyingOrSelf<MaskLeft>;
        using TypeRight = UnderlyingOrSelf<MaskRight>;
        return (static_cast<TypeRight>(subMask) & static_cast<TypeLeft>(mask)) == static_cast<TypeRight>(subMask);
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

    template<typename T, typename Y> static SR_FORCE_INLINE bool IsEquals(const T& a, const Y& b) noexcept {
        // Check for exact equality first, required to handle "infinity" values.

        constexpr bool isNeedTolerance = std::is_floating_point_v<T> || std::is_floating_point_v<Y>;

        if constexpr (!isNeedTolerance) {
            return a == b;
        }
        else {
            if (a == b) {
                return true;
            }
            // Then check for approximate equality.
            double tolerance = CMP_EPSILON * Abs(a);
            if (tolerance < CMP_EPSILON) {
                tolerance = CMP_EPSILON;
            }
            return Abs(a - b) < tolerance;
        }
    }

    template<typename T, typename Y, typename Z> static SR_FORCE_INLINE bool IsEquals(const T& a, const Y& b, const Z& tolerance) noexcept {
        // Check for exact equality first, required to handle "infinity" values.
        if (a == b) {
            return true;
        }
        else {
            // Then check for approximate equality.
            return Abs(a - b) < tolerance;
        }
    }

    template<typename T> constexpr bool IsFloat() {
        using Type = RemoveQualifiersT<T>;
        return
            std::is_same_v<Type, float> ||
            std::is_same_v<Type, double> ||
            std::is_same_v<Type, float_t> ||
            std::is_same_v<Type, double_t> ||
            std::is_same_v<Type, Unit>;
    }

    template<typename T> constexpr bool IsBool() {
        using Type = RemoveQualifiersT<T>;
        return std::is_same_v<Type, bool>;
    }

    template<typename T> constexpr bool IsInt() {
        using Type = RemoveQualifiersT<T>;
        return
            std::is_same_v<Type, int> ||
            std::is_same_v<Type, unsigned short> ||
            std::is_same_v<Type, short> ||
            std::is_same_v<Type, unsigned int> ||
            std::is_same_v<Type, unsigned> ||
            std::is_same_v<Type, long> ||
            std::is_same_v<Type, long long> ||
            std::is_same_v<Type, unsigned long long> ||
            std::is_same_v<Type, unsigned long> ||
            std::is_same_v<Type, int64_t> ||
            std::is_same_v<Type, uint64_t> ||
            std::is_same_v<Type, int32_t> ||
            std::is_same_v<Type, uint32_t> ||
            std::is_same_v<Type, int8_t> ||
            std::is_same_v<Type, uint8_t> ||
            std::is_same_v<Type, int16_t> ||
            std::is_same_v<Type, uint16_t>;
    }

    template<typename T> constexpr bool IsNumber() {
        return IsInt<T>() || IsFloat<T>() || IsBool<T>();
    }

    template<typename T> constexpr bool Approximately(T a, T b) {
        return Abs(b - a) < Max(1E-06f * Max(Abs(a), Abs(b)), 1.401298E-45f * 8.f);
    }
}

#define SR_EQUALS(a, b) (SR_MATH_NS::IsEquals(a, b))
#define SR_EQUALS_T(a, b, tolerance) (SR_MATH_NS::IsEquals(a, b, tolerance))
#define Deg180InRad ((SR_MATH_NS::Unit)M_PI)
#define Deg90InRad  ((SR_MATH_NS::Unit)RAD(90.0))

#endif //SR_ENGINE_UTILS_MATHEMATICS_H
