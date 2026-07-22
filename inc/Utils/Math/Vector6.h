//
// Created by Monika on 08.07.2022.
//

#ifndef SR_ENGINE_VECTOR6_H
#define SR_ENGINE_VECTOR6_H

#include <Utils/Math/Mathematics.h>
#include <Utils/Math/Vector3.h>

namespace SR_MATH_NS {
    template<typename T> struct Vector6 {
    public:
        using ValueType = T;
        static inline constexpr SizeType Dimensions() { return 6; }

        union {
            struct {
                T x;
                T y;
                T z;
                T w;
                T v;
                T u;
            };

            T coord[6] = { 0 };

            struct {
                SR_MATH_NS::Vector3<T> vec3d1;
                SR_MATH_NS::Vector3<T> vec3d2;
            };

            struct {
                SR_MATH_NS::Vector2<T> vec2d1;
                SR_MATH_NS::Vector2<T> vec2d2;
                SR_MATH_NS::Vector2<T> vec2d3;
            };
        };
    public:
        Vector6();
        Vector6(T _x, T _y, T _z, T _w, T _v, T _u);
        Vector6(const Vector6& p_v);
        Vector6(SR_MATH_NS::Vector3<T> vec3d1, SR_MATH_NS::Vector3<T> vec3d2);
        Vector6(T scalar);

        bool operator==(const Vector6 &p_v) const;
        bool operator!=(const Vector6 &p_v) const;

        template<typename U> explicit Vector6(const Vector3<U>& vec);
        template<typename U> explicit Vector6(const Vector3<U>& vec1, const Vector3<U>& vec2);
    };

    template<typename T> Vector6<T>::Vector6() {
        x = y = z = w = v = u = 0;
    }

    template<typename T> Vector6<T>::Vector6(const Vector6<T>& p_v) {
        x = p_v.x;
        y = p_v.y;
        z = p_v.z;
        w = p_v.w;
        v = p_v.v;
        u = p_v.u;
    }

    template<typename T> Vector6<T>::Vector6(SR_MATH_NS::Vector3<T> vec3d1, SR_MATH_NS::Vector3<T> vec3d2)
        : vec3d1(vec3d1)
        , vec3d2(vec3d2)
    { }

    template<typename T> Vector6<T>::Vector6(T scalar) {
        x = y = z = w = v = u = scalar;
    }

    template<typename T> bool Vector6<T>::operator==(const Vector6<T> &p_v) const {
        return SR_EQUALS(x, p_v.x)
               && SR_EQUALS(y, p_v.y)
               && SR_EQUALS(z, p_v.z)
               && SR_EQUALS(w, p_v.w)
               && SR_EQUALS(v, p_v.v)
               && SR_EQUALS(u, p_v.u);
    }

    template<typename T> bool Vector6<T>::operator!=(const Vector6<T> &p_v) const {
        return !(*this == p_v);
    }

    template<typename T> template<typename U> Vector6<T>::Vector6(const Vector3<U>& vec) {
        x = static_cast<T>(vec.x);
        y = static_cast<T>(vec.y);
        z = static_cast<T>(vec.z);
        w = static_cast<T>(vec.w);
        v = static_cast<T>(vec.v);
        u = static_cast<T>(vec.u);
    }

    template<typename T> template<typename U> Vector6<T>::Vector6(const Vector3<U>& vec1, const Vector3<U>& vec2) {
        x = static_cast<T>(vec1.x);
        y = static_cast<T>(vec1.y);
        z = static_cast<T>(vec1.z);
        w = static_cast<T>(vec2.x);
        v = static_cast<T>(vec2.y);
        u = static_cast<T>(vec2.z);
    }

    template<typename T> Vector6<T>::Vector6(T _x, T _y, T _z, T _w, T _v, T _u) {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
        v = _v;
        u = _u;
    }

    typedef Vector6<Unit>     FVector6;
    typedef Vector6<int16_t>  SVector6;
    typedef Vector6<int32_t>  IVector6;
    typedef Vector6<uint32_t> UVector6;
    typedef Vector6<bool>     BVector6;
    typedef Vector6<uint16_t> USVector6;
}

#endif //SR_ENGINE_VECTOR6_H
