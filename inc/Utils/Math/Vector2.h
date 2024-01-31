//
// Created by Nikita on 11.07.2021.
//

#ifndef GAMEENGINE_VECTOR2_H
#define GAMEENGINE_VECTOR2_H

#include <Utils/Math/Mathematics.h>

namespace SR_MATH_NS {
    template<typename T> struct SR_DLL_EXPORT Vector2 {
    public:
        union {
            struct {
                T x;
                T y;
            };

            struct {
                T top;
                T bottom;
            };

            T coord[2] = { 0 };
        };

        constexpr SR_FORCE_INLINE Vector2() {
            x = 0;
            y = 0;
        }

        constexpr SR_FORCE_INLINE Vector2(T p_x, T p_y) {
            x = p_x;
            y = p_y;
        }

        constexpr SR_FORCE_INLINE Vector2(T p) {
            x = p;
            y = p;
        }

        constexpr SR_FORCE_INLINE Vector2(const glm::vec2 &vec2) {
            x = static_cast<T>(vec2.x);
            y = static_cast<T>(vec2.y);
        }

        static constexpr Vector2<T> Zero() { return Vector2(static_cast<T>(0)); }
        static constexpr Vector2<T> One() { return Vector2(static_cast<T>(1)); }

        template<typename U> SR_NODISCARD Vector2<U> SR_FASTCALL Cast() const noexcept { return Vector2<U>(
                    static_cast<U>(x),
                    static_cast<U>(y)
            );
        }

        SR_NODISCARD Vector2 Singular(const Vector2& segment) const { return Vector2(
                    x > 0 ? x + segment.x : x - segment.x,
                    y > 0 ? y + segment.y : y - segment.y
            );
        }

        SR_NODISCARD Vector2 DeSingular(const Vector2& segment) const { return Vector2(
                    x > 0 ? x - segment.x : x,
                    y > 0 ? y - segment.y : y
            );
        }

        SR_NODISCARD T Distance(const Vector2& vec) const noexcept {
            if constexpr (std::is_same_v<T, float_t> || std::is_same_v<T, float>) {
                return static_cast<T>(sqrtf(powf(vec.x - x, 2) + powf(vec.y - y, 2)));
            }
            else {
                return static_cast<T>(sqrt(pow(vec.x - x, 2) + pow(vec.y - y, 2)));
            }
        }

        SR_NODISCARD T Length() const noexcept {
            return Distance(Vector2<T>());
        }

        SR_NODISCARD Vector2 Abs() const {
            return Vector2(static_cast<T>(abs(x)), static_cast<T>(abs(y)));
        }

        SR_NODISCARD T Sum() const {
            return x + y;
        }

        SR_NODISCARD Unit Aspect() const {
            if (HasZero()) {
                return static_cast<Unit>(0);
            }
            return static_cast<Unit>(x) / static_cast<Unit>(y);
        }

        SR_NODISCARD Unit AspectInv() const {
            if (HasZero()) {
                return static_cast<Unit>(0);
            }
            return static_cast<Unit>(y) / static_cast<Unit>(x);
        }

        SR_FORCE_INLINE Vector2 operator-() const { return Vector2(-x, -y); }
        SR_FORCE_INLINE Vector2 operator+() const { return *this; }

        SR_FORCE_INLINE bool operator<=(const Vector2 &p_v) const { return x <= p_v.x && y <= p_v.y; }
        SR_FORCE_INLINE bool operator>=(const Vector2 &p_v) const { return x >= p_v.x && y >= p_v.y; }
        SR_FORCE_INLINE bool operator<(const Vector2 &p_v) const { return x < p_v.x && y < p_v.y; }
        SR_FORCE_INLINE bool operator>(const Vector2 &p_v) const { return x > p_v.x && y > p_v.y; }

        SR_FORCE_INLINE bool operator>(T p_scalar) const { return *this > Vector2<T>(p_scalar); }
        SR_FORCE_INLINE bool operator<(T p_scalar) const { return *this < Vector2<T>(p_scalar); }
        SR_FORCE_INLINE bool operator>=(T p_scalar) const { return *this >= Vector2<T>(p_scalar); }
        SR_FORCE_INLINE bool operator<=(T p_scalar) const { return *this <= Vector2<T>(p_scalar); }
        SR_FORCE_INLINE bool operator==(T p_scalar) const { return *this == Vector2<T>(p_scalar); }
        SR_FORCE_INLINE bool operator!=(T p_scalar) const { return *this != Vector2<T>(p_scalar); }

        template<typename U> SR_FORCE_INLINE Vector2 operator*(const U &scalar) const noexcept {
            return Vector2(x * scalar, y * scalar);
        }

        template<typename U> SR_FORCE_INLINE Vector2 operator+(const Vector2<U> &p_v) const {
            return Vector2(x + p_v.x, y + p_v.y);
        }

        template<typename U> SR_FORCE_INLINE Vector2 operator-(const Vector2<U> &p_v) const {
            return Vector2(x - p_v.x, y - p_v.y);
        }

        template<typename U> SR_FORCE_INLINE Vector2 operator*=(const U& value) {
            this->x *= value;
            this->y *= value;
            return *this;
        }

        template<typename U> SR_FORCE_INLINE bool operator==(const Vector2<U> &p_v) const {
            return SR_EQUALS(x, p_v.x) && SR_EQUALS(y, p_v.y);
        }

        template<typename U> SR_FORCE_INLINE bool operator!=(const Vector2<U> &p_v) const {
            return !SR_EQUALS(x, p_v.x) || !SR_EQUALS(y, p_v.y);
        }

        template<typename U> SR_FORCE_INLINE Vector2 operator*(const Vector2<U> &p_v) const {
            return Vector2(x * p_v.x, y * p_v.y);
        }

        template<typename U> SR_FORCE_INLINE Vector2 operator/(const Vector2<U> &p_v) const {
            return Vector2(x / p_v.x, y / p_v.y);
        }

        template<typename U> SR_FORCE_INLINE Vector2 operator/(const U &v) const {
            return Vector2(x / v, y / v);
        }

        template<typename U> SR_FORCE_INLINE Vector2 &operator/=(U p_scalar) {
            x /= p_scalar;
            y /= p_scalar;
            return *this;
        }

        template<typename U> SR_FORCE_INLINE Vector2 &operator-=(const Vector2<U> &p_v) {
            x -= p_v.x;
            y -= p_v.y;
            return *this;
        }

        template<typename U> SR_FORCE_INLINE Vector2 &operator+=(Vector2<U> v) {
            x += v.x;
            y += v.y;
            return *this;
        }

        SR_NODISCARD SR_FORCE_INLINE bool Empty() const {
            return x == 0 && y == 0;
        }

        SR_NODISCARD bool HasZero() const {
            return (x == 0 || y == 0);
        }

        SR_NODISCARD bool IsNull() const {
            return (x == 0 && y == 0);
        }

        SR_NODISCARD bool HasNegative() const {
            return (x < 0 || y < 0);
        }

        SR_NODISCARD bool HasPositive() const {
            return (x > 0 || y > 0);
        }

        template<typename U> SR_NODISCARD bool Contains(U value) const {
            return x == static_cast<T>(value) || y == static_cast<T>(value);
        }

        SR_NODISCARD SR_FORCE_INLINE glm::vec2 ToGLM() const noexcept {
            return { static_cast<float_t>(x), static_cast<float_t>(y) };
        }

        SR_FORCE_INLINE const T &operator[](int p_axis) const {
            return coord[p_axis];
        }

        SR_FORCE_INLINE T &operator[](int p_axis) {
            return coord[p_axis];
        }

        SR_NODISCARD std::string ToString() const {
            return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
        }
    };

    typedef Vector2<Unit> FVector2;
    typedef Vector2<Unit> FPoint;
    typedef Vector2<int32_t> IVector2;
    typedef Vector2<int32_t> IPoint;
    typedef Vector2<uint32_t> UVector2;
    typedef Vector2<uint32_t> UPoint;
    typedef Vector2<bool> BVector2;

    SR_INLINE static const FVector2 InfinityFV2 = FVector2 { UnitMAX, UnitMAX };

    constexpr IVector2 IVector2MAX = IVector2(SR_INT32_MAX);
}

namespace std {
    template <class T> static inline void hash_vector2_combine(std::size_t & s, const T & v) {
        std::hash<T> h;
        s^= h(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
    }

    template<typename U> struct hash<SR_MATH_NS::Vector2<U>> {
        size_t operator()(SR_MATH_NS::Vector2<U> const& vec) const {
            std::size_t res = 0;
            hash_vector2_combine<U>(res, vec.x);
            hash_vector2_combine<U>(res, vec.y);
            return res;
        }
    };
}

#endif //GAMEENGINE_VECTOR2_H
