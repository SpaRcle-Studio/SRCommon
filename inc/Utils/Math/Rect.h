//
// Created by Monika on 18.11.2022.
//

#if !defined(SR_ENGINE_RECT_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_ENGINE_RECT_H

#include <Utils/Math/Vector2.h>
#include <Utils/Math/Vector4.h>

namespace SR_MATH_NS {
    template<typename T> struct Rect {
    public:
        union {
            struct {
                T x;
                T y;
                T w;
                T h;
            };
            struct {
                T left;
                T top;
                T right;
                T bottom;
            };
            struct {
                SR_MATH_NS::Vector2<T> xy;
                SR_MATH_NS::Vector2<T> wh;
            };
            struct {
                SR_MATH_NS::Vector2<T> position;
                SR_MATH_NS::Vector2<T> size;
            };
            struct {
                SR_MATH_NS::Vector4<T> vec4;
            };
        };

    public:
        /// left, top, right, bottom
        Rect(T _x, T _y, T _w, T _h);
        Rect(const Vector2<T>& position, const Vector2<T>& size);
        Rect(const Vector2<T>& position, T width, T height);
        Rect(const Vector4<T>& vec4);
        Rect(T x, T y, const Vector2<T>& size);
        Rect();

    public:
        void Shrink(const Rect<T>& margin) noexcept;
        SR_NODISCARD Rect<T> Shrink(const Rect<T>& margin) const noexcept;

        SR_NODISCARD T X() const noexcept;
        SR_NODISCARD T Y() const noexcept;

        SR_NODISCARD T Width() const noexcept;
        SR_NODISCARD T Height() const noexcept;

        SR_NODISCARD T Left() const noexcept;
        SR_NODISCARD T Right() const noexcept;

        SR_NODISCARD T Bottom() const noexcept;
        SR_NODISCARD T Top() const noexcept;

        SR_NODISCARD T Horizontal() const noexcept;
        SR_NODISCARD T Vertical() const noexcept;

        SR_NODISCARD SR_MATH_NS::Vector2<T> XY() const noexcept;
        SR_NODISCARD SR_MATH_NS::Vector2<T> WH() const noexcept;

        SR_NODISCARD SR_MATH_NS::Vector2<T> Min() const noexcept;
        SR_NODISCARD SR_MATH_NS::Vector2<T> Max() const noexcept;

        SR_NODISCARD SR_MATH_NS::Vector2<T> GetMin() const noexcept;
        SR_NODISCARD SR_MATH_NS::Vector2<T> GetMax() const noexcept;

        SR_NODISCARD SR_MATH_NS::Vector2<T> Center() const noexcept;
        SR_NODISCARD SR_MATH_NS::Vector2<T> Size() const noexcept;

        SR_NODISCARD SR_MATH_NS::Vector2<T> TopBottom() const noexcept;
        SR_NODISCARD SR_MATH_NS::Vector2<T> LeftRight() const noexcept;

        SR_NODISCARD SR_MATH_NS::Vector3<T> XY0() const noexcept;
        SR_NODISCARD SR_MATH_NS::Vector3<T> WH0() const noexcept;
        SR_NODISCARD SR_MATH_NS::Vector3<T> WH1() const noexcept;

        template<typename U> SR_NODISCARD bool IsInside(const SR_MATH_NS::Vector2<U>& point) const noexcept;
        SR_NODISCARD bool operator==(const Rect& other) const noexcept;
        SR_NODISCARD bool operator!=(const Rect& other) const noexcept;
        SR_NODISCARD Rect operator+(const SR_MATH_NS::Rect<T>& other) const noexcept;
        SR_NODISCARD Rect operator-(const SR_MATH_NS::Rect<T>& other) const noexcept;
        SR_NODISCARD Rect operator*(const SR_MATH_NS::Rect<T>& other) const noexcept;
        SR_NODISCARD Rect operator/(const SR_MATH_NS::Rect<T>& other) const noexcept;
        void operator+=(const SR_MATH_NS::Rect<T>& other) noexcept;
        void operator-=(const SR_MATH_NS::Rect<T>& other) noexcept;
        void operator*=(const SR_MATH_NS::Rect<T>& other) noexcept;
        void operator/=(const SR_MATH_NS::Rect<T>& other) noexcept;

        SR_NODISCARD Rect<int32_t> CastToInt() const noexcept;

    public:
        SR_NODISCARD static Rect<Unit> FromTranslationAndScale(const SR_MATH_NS::FVector2& translation, const SR_MATH_NS::FVector2& scale);

        SR_NODISCARD bool Contains(const SR_MATH_NS::Vector2<T>& point) const noexcept;

    public:
        void SetLeft(const T& value);
        void SetBottom(const T& value);
        void SetRight(const T& value);
        void SetTop(const T& value);

        SR_NODISCARD Rect IntersectInclusive(const Rect& other) const { // >=, границы считаются
            T newLeft = std::max(Left(), other.Left());
            T newBottom = std::max(Bottom(), other.Bottom());
            T newRight = std::min(Right(), other.Right());
            T newTop = std::min(Top(), other.Top());

            if (newLeft <= newRight && newBottom <= newTop) {
                return Rect(newLeft, newBottom, newRight - newLeft, newTop - newBottom);
            }

            return Rect(); // Пустой прямоугольник
        }

        SR_NODISCARD Rect IntersectExclusive(const Rect& other) const { // >
            T newLeft = std::max(Left(), other.Left());
            T newBottom = std::max(Bottom(), other.Bottom());
            T newRight = std::min(Right(), other.Right());
            T newTop = std::min(Top(), other.Top());

            if (newLeft < newRight && newBottom < newTop) {
                return Rect(newLeft, newBottom, newRight - newLeft, newTop - newBottom);
            }

            return Rect(); // Пустой прямоугольник
        }

        void SetMin(const SR_MATH_NS::Vector2<T>& min) {
            SetLeft(min.x);
            SetBottom(min.y);
        }

        void SetMax(const SR_MATH_NS::Vector2<T>& max) {
            SetRight(max.x);
            SetTop(max.y);
        }

        void SetMin(T minX, T minY) {
            SetLeft(minX);
            SetBottom(minY);
        }

        void SetMax(T maxX, T maxY) {
            SetRight(maxX);
            SetTop(maxY);
        }

    };

    /// ============================================== Rect<T> =========================================================

    template<typename T> Rect<int32_t> Rect<T>::CastToInt() const noexcept {
        return Rect<int32_t>(
            static_cast<int32_t>(x),
            static_cast<int32_t>(y),
            static_cast<int32_t>(w),
            static_cast<int32_t>(h)
        );
    }

    template<typename T> SR_MATH_NS::Vector2<T> Rect<T>::Min() const noexcept {
        return SR_MATH_NS::Vector2<T>(Left(), Bottom());
    }

    template<typename T> SR_MATH_NS::Vector2<T> Rect<T>::Max() const noexcept {
        return SR_MATH_NS::Vector2<T>(Right(), Top());
    }

    template<typename T> SR_MATH_NS::Vector2<T> Rect<T>::GetMin() const noexcept {
        return SR_MATH_NS::Vector2<T>(Left(), Bottom());
    }

    template<typename T> SR_MATH_NS::Vector2<T> Rect<T>::GetMax() const noexcept {
        return SR_MATH_NS::Vector2<T>(Right(), Top());
    }

    template<typename T> void Rect<T>::SetLeft(const T& value) {
        w -= value - x;
        x = value;
    }

    template<typename T> void Rect<T>::SetBottom(const T& value) {
        h -= value - y;
        y = value;
    }

    template<typename T> void Rect<T>::SetRight(const T& value) {
        w = value - x;
    }

    template<typename T> void Rect<T>::SetTop(const T& value) {
        h = value - y;
    }

    template<typename T> void Rect<T>::Shrink(const Rect<T>& margin) noexcept {
        x += margin.x;
        y += margin.y;
        w -= margin.x + margin.w;
        h -= margin.y + margin.h;
    }

    template<typename T> Rect<T> Rect<T>::Shrink(const Rect<T>& margin) const noexcept {
        return Rect<T>(x + margin.x, y + margin.y, w - margin.x - margin.w, h - margin.y - margin.h);
    }

    template<typename T> bool Rect<T>::Contains(const Vector2<T> &point) const noexcept {
        return point.x >= Left() && point.x <= Right() && point.y <= Top() && point.y >= Bottom();
    }

    template<typename T> Rect<T>::Rect(T _x, T _y, T _w, T _h)
        : x(_x)
        , y(_y)
        , w(_w)
        , h(_h)
    { }

    template<typename T> Rect<T>::Rect(const Vector2<T>& position, const Vector2<T>& size)
        : x(position.x)
        , y(position.y)
        , w(size.x)
        , h(size.y)
    { }

    template<typename T> Rect<T>::Rect(const Vector2<T>& position, T width, T height)
        : x(position.x)
        , y(position.y)
        , w(width)
        , h(height)
    { }

    template<typename T> Rect<T>::Rect(const Vector4<T>& vec4)
        : x(vec4.x)
        , y(vec4.y)
        , w(vec4.z)
        , h(vec4.w)
    { }

    template<typename T> Rect<T>::Rect(T x, T y, const Vector2<T>& size)
        : x(x)
        , y(y)
        , w(size.x)
        , h(size.y)
    { }

    template<typename T> Rect<T>::Rect()
        : Rect(T(), T(), T(), T())
    { }

    template<typename T> SR_NODISCARD T Rect<T>::X() const noexcept { return x; }
    template<typename T> SR_NODISCARD T Rect<T>::Y() const noexcept { return y; }

    template<typename T> SR_NODISCARD T Rect<T>::Width() const noexcept { return w; }
    template<typename T> SR_NODISCARD T Rect<T>::Height() const noexcept { return h; }

    template<typename T> SR_NODISCARD T Rect<T>::Left() const noexcept { return x; }
    template<typename T> SR_NODISCARD T Rect<T>::Right() const noexcept { return Left() + Width(); }

    template<typename T> SR_NODISCARD T Rect<T>::Bottom() const noexcept { return y;  }
    template<typename T> SR_NODISCARD T Rect<T>::Top() const noexcept { return Bottom() + Height(); }

    template<typename T> SR_NODISCARD T Rect<T>::Vertical() const noexcept { return top + bottom; }
    template<typename T> SR_NODISCARD T Rect<T>::Horizontal() const noexcept { return left + right; }

    template<typename T> SR_NODISCARD SR_MATH_NS::Vector2<T> Rect<T>::TopBottom() const noexcept { return SR_MATH_NS::Vector2<T>(Top(), Bottom()); }
    template<typename T> SR_NODISCARD SR_MATH_NS::Vector2<T> Rect<T>::LeftRight() const noexcept { return SR_MATH_NS::Vector2<T>(Left(), Right()); }

    template<typename T> SR_NODISCARD SR_MATH_NS::Vector2<T> Rect<T>::XY() const noexcept { return SR_MATH_NS::Vector2<T>(x, y); }
    template<typename T> SR_NODISCARD SR_MATH_NS::Vector2<T> Rect<T>::WH() const noexcept { return SR_MATH_NS::Vector2<T>(w, h); }

    template<typename T> SR_NODISCARD SR_MATH_NS::Vector2<T> Rect<T>::Center() const noexcept {
        return SR_MATH_NS::Vector2<T>(x + static_cast<T>(static_cast<double_t>(w) / 2.0),
                                      y + static_cast<T>(static_cast<double_t>(h) / 2.0));
    }

    template<typename T> SR_NODISCARD SR_MATH_NS::Vector2<T> Rect<T>::Size() const noexcept {
        return SR_MATH_NS::Vector2<T>(w, h);
    }

    template<typename T> SR_NODISCARD SR_MATH_NS::Vector3<T> Rect<T>::XY0() const noexcept { return SR_MATH_NS::Vector3<T>(x, y, 0); }
    template<typename T> SR_NODISCARD SR_MATH_NS::Vector3<T> Rect<T>::WH0() const noexcept { return SR_MATH_NS::Vector3<T>(w, h, 0); }
    template<typename T> SR_NODISCARD SR_MATH_NS::Vector3<T> Rect<T>::WH1() const noexcept { return SR_MATH_NS::Vector3<T>(w, h, 1); }

    template<typename T> template<typename U> SR_NODISCARD bool Rect<T>::IsInside(const SR_MATH_NS::Vector2<U>& point) const noexcept {
        return point.x >= static_cast<U>(x) && point.x <= static_cast<U>(x + w) &&
               point.y >= static_cast<U>(y) && point.y <= static_cast<U>(y + h);
    }

    template<typename T> SR_NODISCARD bool Rect<T>::operator==(const Rect& other) const noexcept {
        return SR_MATH_NS::IsEquals(x, other.x) && SR_MATH_NS::IsEquals(y, other.y) &&
               SR_MATH_NS::IsEquals(w, other.w) && SR_MATH_NS::IsEquals(h, other.h);
    }

    template<typename T> SR_NODISCARD bool Rect<T>::operator!=(const Rect& other) const noexcept {
        return !(*this == other);
    }

    template<typename T> SR_NODISCARD Rect<T> Rect<T>::operator+(const SR_MATH_NS::Rect<T>& other) const noexcept {
        return Rect(x + other.x, y + other.y, w + other.w, h + other.h);
    }

    template<typename T> SR_NODISCARD Rect<T> Rect<T>::operator-(const SR_MATH_NS::Rect<T>& other) const noexcept {
        return Rect(x - other.x, y - other.y, w - other.w, h - other.h);
    }

    template<typename T> SR_NODISCARD Rect<T> Rect<T>::operator*(const SR_MATH_NS::Rect<T>& other) const noexcept {
        return Rect(x * other.x, y * other.y, w * other.w, h * other.h);
    }

    template<typename T> SR_NODISCARD Rect<T> Rect<T>::operator/(const SR_MATH_NS::Rect<T>& other) const noexcept {
        return Rect(x / other.x, y / other.y, w / other.w, h / other.h);
    }

    template<typename T> void Rect<T>::operator+=(const SR_MATH_NS::Rect<T>& other) noexcept {
        x += other.x;
        y += other.y;
        w += other.w;
        h += other.h;
    }

    template<typename T> void Rect<T>::operator-=(const SR_MATH_NS::Rect<T>& other) noexcept {
        x -= other.x;
        y -= other.y;
        w -= other.w;
        h -= other.h;
    }

    template<typename T> void Rect<T>::operator*=(const SR_MATH_NS::Rect<T>& other) noexcept {
        x *= other.x;
        y *= other.y;
        w *= other.w;
        h *= other.h;
    }

    template<typename T> void Rect<T>::operator/=(const SR_MATH_NS::Rect<T>& other) noexcept {
        x /= other.x;
        y /= other.y;
        w /= other.w;
        h /= other.h;
    }

    template<typename T> Rect<Unit> Rect<T>::FromTranslationAndScale(const FVector2& translation, const FVector2& scale) {
        return Rect<Unit>(translation.x, translation.y, scale.x * 2, scale.y * 2);
    }

    typedef Rect<Unit> FRect;
    typedef Rect<int32_t> IRect;
    typedef Rect<uint32_t> URect;
    typedef Rect<uint16_t> USRect;
}

template<typename T> struct fmt::formatter<SR_MATH_NS::Rect<T>> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    auto format(const SR_MATH_NS::Rect<T>& val, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "Rect(x: {}, y: {}, w: {}, h: {})", val.x, val.y, val.w, val.h);
    }
};

#endif //SR_ENGINE_RECT_H
