//
// Created by Monika on 1.10.2024.
//

#include <Utils/Math/Vector4.h>

#include <Utils/Common/StringAtomLiterals.h>

namespace SR_MATH_NS {
    std::unordered_map<SRHashType, FColor> SR_COLOR_PALETTE = {
        { "transparent"_atom_hash, FColor(0, 0, 0, 0) / 255.f },
        { "black"_atom_hash, FColor(0, 0, 0, 255) / 255.f },
        { "silver"_atom_hash, FColor(192, 192, 192, 255) / 255.f },
        { "gray"_atom_hash, FColor(128, 128, 128, 255) / 255.f },
        { "white"_atom_hash, FColor(255, 255, 255, 255) / 255.f },
        { "maroon"_atom_hash, FColor(128, 0, 0, 255) / 255.f },
        { "red"_atom_hash, FColor(255, 0, 0, 255) / 255.f },
        { "purple"_atom_hash, FColor(128, 0, 128, 255) / 255.f },
        { "fuchsia"_atom_hash, FColor(255, 0, 255, 255) / 255.f },
        { "green"_atom_hash, FColor(0, 128, 0, 255) / 255.f },
        { "lime"_atom_hash, FColor(0, 255, 0, 255) / 255.f },
        { "olive"_atom_hash, FColor(128, 128, 0, 255) / 255.f },
        { "yellow"_atom_hash, FColor(255, 255, 0, 255) / 255.f },
        { "navy"_atom_hash, FColor(0, 0, 128, 255) / 255.f },
        { "blue"_atom_hash, FColor(0, 0, 255, 255) / 255.f },
        { "teal"_atom_hash, FColor(0, 128, 128, 255) / 255.f },
        { "aqua"_atom_hash, FColor(0, 255, 255, 255) / 255.f },
        { "pink"_atom_hash, FColor(255, 192, 203, 255) / 255.f },
        { "brown"_atom_hash, FColor(165, 42, 42, 255) / 255.f },
        { "orange"_atom_hash, FColor(255, 165, 0, 255) / 255.f },
        { "coral"_atom_hash, FColor(255, 127, 80, 255) / 255.f },
        { "gold"_atom_hash, FColor(255, 215, 0, 255) / 255.f },
        { "beige"_atom_hash, FColor(245, 245, 220, 255) / 255.f },
        { "ivory"_atom_hash, FColor(255, 255, 240, 255) / 255.f },
        { "khaki"_atom_hash, FColor(240, 230, 140, 255) / 255.f },
        { "tan"_atom_hash, FColor(210, 180, 140, 255) / 255.f },
        { "wheat"_atom_hash, FColor(245, 222, 179, 255) / 255.f },
        { "azure"_atom_hash, FColor(240, 255, 255, 255) / 255.f },
        { "cyan"_atom_hash, FColor(0, 255, 255, 255) / 255.f },
        { "magenta"_atom_hash, FColor(255, 0, 255, 255) / 255.f },
        { "violet"_atom_hash, FColor(238, 130, 238, 255) / 255.f },
        { "indigo"_atom_hash, FColor(75, 0, 130, 255) / 255.f },
        { "turquoise"_atom_hash, FColor(64, 224, 208, 255) / 255.f },
        { "skyblue"_atom_hash, FColor(135, 206, 235, 255) / 255.f },
        { "royalblue"_atom_hash, FColor(65, 105, 225, 255) / 255.f },
        { "slateblue"_atom_hash, FColor(106, 90, 205, 255) / 255.f },
        { "steelblue"_atom_hash, FColor(70, 130, 180, 255) / 255.f },
        { "powderblue"_atom_hash, FColor(176, 224, 230, 255) / 255.f },
        { "aliceblue"_atom_hash, FColor(240, 248, 255, 255) / 255.f },
        { "ghostwhite"_atom_hash, FColor(248, 248, 255, 255) / 255.f },
        { "lavender"_atom_hash, FColor(230, 230, 250, 255) / 255.f },
        { "mintcream"_atom_hash, FColor(245, 255, 250, 255) / 255.f },
        { "honeydew"_atom_hash, FColor(240, 255, 240, 255) / 255.f },
        { "seashell"_atom_hash, FColor(255, 245, 238, 255) / 255.f },
        { "floralwhite"_atom_hash, FColor(255, 250, 240, 255) / 255.f },
        { "linen"_atom_hash, FColor(250, 240, 230, 255) / 255.f },
        { "oldlace"_atom_hash, FColor(253, 245, 230, 255) / 255.f },
        { "papayawhip"_atom_hash, FColor(255, 239, 213, 255) / 255.f },
        { "blanchedalmond"_atom_hash, FColor(255, 235, 205, 255) / 255.f },
        { "bisque"_atom_hash, FColor(255, 228, 196, 255) / 255.f },
        { "peachpuff"_atom_hash, FColor(255, 218, 185, 255) / 255.f },
        { "mistyrose"_atom_hash, FColor(255, 228, 225, 255) / 255.f },
        { "antiquewhite"_atom_hash, FColor(250, 235, 215, 255) / 255.f },
        { "beige"_atom_hash, FColor(245, 245, 220, 255) / 255.f },
        { "cornsilk"_atom_hash, FColor(255, 248, 220, 255) / 255.f },
        { "lemonchiffon"_atom_hash, FColor(255, 250, 205, 255) / 255.f },
        { "lightgoldenrodyellow"_atom_hash, FColor(250, 250, 210, 255) / 255.f },
        { "lightyellow"_atom_hash, FColor(255, 255, 224, 255) / 255.f },
        { "palegoldenrod"_atom_hash, FColor(238, 232, 170, 255) / 255.f },
        { "khaki"_atom_hash, FColor(240, 230, 140, 255) / 255.f },
    };

    FVector4 BuildPlan(const FVector3 &point, const FVector3 &normal) {
        return SR_MATH_NS::FVector4(normal, normal.Normalize().Dot(point));
    }

    FVector4 BuildPlan(const FVector4 &point, const FVector4 &p_normal) {
        FVector4 normal, res;
        normal = p_normal.Normalize();
        res.w = normal.Dot(point);
        res.x = normal.x;
        res.y = normal.y;
        res.z = normal.z;
        return res;
    }

    uint32_t RGBToHEX(const IVector3 &color) {
        return ((color.x & 0xff) << 16) + ((color.y & 0xff) << 8) + (color.z & 0xff);
    }

    uint32_t BGRToHEX(const IVector3 &color) {
        return ((color.z & 0xff) << 16) + ((color.y & 0xff) << 8) + (color.x & 0xff);
    }

    IVector3 HEXToRGB(uint32_t hex) {
        IVector3 color;

        color.x = static_cast<Unit>(((hex >> 16) & 0xFF));
        color.y = static_cast<Unit>(((hex >> 8) & 0xFF));
        color.z = static_cast<Unit>(((hex) & 0xFF));

        return color;
    }

    IVector3 HEXToBGR(uint32_t hex) {
        IVector3 color;

        color.x = static_cast<Unit>(((hex) & 0xFF));
        color.y = static_cast<Unit>(((hex >> 8) & 0xFF));
        color.z = static_cast<Unit>(((hex >> 16) & 0xFF));

        return color;
    }

    FColor::FColor() { r = g = b = a = 0.f; }
    FColor::FColor (Unit scalar) { r = g = b = a = scalar; }
    FColor::FColor(const glm::vec4& vec4) {
        r = vec4.x;
        g = vec4.y;
        b = vec4.z;
        a = vec4.w;
    }

    FColor::FColor(const Vector4<Unit>& v) {
        r = v.x;
        g = v.y;
        b = v.z;
        a = v.w;
    }

    FColor::FColor(const FColor& color) = default;
    FColor::FColor(FColor&& color) = default;

    FColor::FColor(double_t _x, double_t _y, double_t _z, double_t _w) {
        r = static_cast<Unit>(_x);
        g = static_cast<Unit>(_y);
        b = static_cast<Unit>(_z);
        a = static_cast<Unit>(_w);
    }

    FColor::FColor(float_t _x, float_t _y, float_t _z, float_t _w) {
        r = static_cast<Unit>(_x);
        g = static_cast<Unit>(_y);
        b = static_cast<Unit>(_z);
        a = static_cast<Unit>(_w);
    }

    FColor::FColor(int32_t _x, int32_t _y, int32_t _z, int32_t _w) {
        r = static_cast<Unit>(_x);
        g = static_cast<Unit>(_y);
        b = static_cast<Unit>(_z);
        a = static_cast<Unit>(_w);
    }

    FColor& FColor::operator=(const FColor& color) = default;
    FColor& FColor::operator=(FColor&& color) = default;

    bool FColor::operator==(const FColor& v) const {
        return SR_EQUALS(r, v.r) && SR_EQUALS(g, v.g) && SR_EQUALS(b, v.b) && SR_EQUALS(a, v.a);
    }
    bool FColor::operator!=(const FColor& v) const = default;

    FColor& FColor::operator*=(Unit p_scalar) {
        r *= p_scalar;
        g *= p_scalar;
        b *= p_scalar;
        a *= p_scalar;
        return *this;
    }

    FColor& FColor::operator/=(Unit p_scalar) {
        r /= p_scalar;
        g /= p_scalar;
        b /= p_scalar;
        a /= p_scalar;
        return *this;
    }

    FColor& FColor::operator*=(const FColor& p_v) {
        r *= p_v.r;
        g *= p_v.g;
        b *= p_v.b;
        a *= p_v.a;
        return *this;
    }

    FColor FColor::operator-(const FColor &p_v) const {
        return FColor(r - p_v.r, g - p_v.g, b - p_v.b, a - p_v.a);
    }

    FColor& FColor::operator/=(const FColor& p_v) {
        r /= p_v.r;
        g /= p_v.g;
        b /= p_v.b;
        a /= p_v.a;
        return *this;
    }

    FColor& FColor::operator%=(const FColor &p_v) {
        r = fmodf(r, p_v.r);
        g = fmodf(g, p_v.g);
        b = fmodf(b, p_v.b);
        a = fmodf(a, p_v.a);
        return *this;
    }

    FColor FColor::operator/(const Unit& v) const {
        return FColor(r / v, g / v, b / v, a / v);
    }
    FColor FColor::operator*(const FColor &p_v) const {
        return FColor(r * p_v.r, g * p_v.g, b * p_v.b, a * p_v.a);
    }
    FColor FColor::operator+(const FColor& v) const {
        return FColor(r + v.r, g + v.g, b + v.b, a + v.a);
    }

    FColor FColor::Red() { return FColor(255.f, 0.f, 0.f, 255.f); }
    FColor FColor::Green() { return FColor(0.f, 255.f, 0.f, 255.f); }
    FColor FColor::Blue() { return FColor(0.f, 0.f, 255.f, 255.f); }
    FColor FColor::White() { return FColor(255.f, 255.f, 255.f, 255.f); }
    FColor FColor::Black() { return FColor(0.f, 0.f, 0.f, 255.f); }
    FColor FColor::Yellow() { return FColor(255.f, 255.f, 0.f, 255.f); }
    FColor FColor::Cyan() { return FColor(0.f, 255.f, 255.f, 255.f); }
    FColor FColor::Magenta() { return FColor(255.f, 0.f, 255.f, 255.f); }
    FColor FColor::Alpha() { return FColor(0.f, 0.f, 0.f, 0.f); }

    SR_NODISCARD Unit Ray::IntersectPlaneDistance(const SR_MATH_NS::FVector4& plane) const {
        const Unit numer = plane.Dot3(origin) - plane.w;
        const Unit denom = plane.Dot3(direction);

        /// normal is orthogonal to vector, cant intersect
        if (fabsf(denom) < SR_FLT_EPSILON) {
            return -1.0f;
        }

        return -(numer / denom);
    }

    SR_NODISCARD SR_MATH_NS::FVector3 Ray::RotationVector(const SR_MATH_NS::FVector4& plan, const SR_MATH_NS::FVector3& position) const noexcept {
        const Unit len = IntersectPlaneDistance(plan);
        auto&& localPos = origin + direction * len - position;
        auto&& rotationVectorSource = localPos.Normalize();
        return rotationVectorSource;
    }


    SR_NODISCARD Unit Ray::ComputeAngleOnPlan(const SR_MATH_NS::FVector4& plan, const SR_MATH_NS::FVector3& position, const SR_MATH_NS::FVector3& sourceRotationVector) const noexcept {
        const Unit len = IntersectPlaneDistance(plan);
        auto&& localPos = (origin + direction * len - position).Normalize();

        auto&& perpendicularVector = sourceRotationVector.Cross(plan.XYZ()).Normalize();

        const Unit acosAngle = SR_CLAMP(localPos.Dot(sourceRotationVector), -1.f, 1.f);
        const Unit angle = acosf(acosAngle) * ((localPos.Dot(perpendicularVector) < 0.f) ? 1.f : -1.f);

        return angle;
    }

    SR_NODISCARD SR_MATH_NS::FVector3 Ray::IntersectPlane(const SR_MATH_NS::FVector4& plan) const noexcept {
        const float_t signedLength = IntersectPlaneDistance(plan);
        const float_t len = fabsf(signedLength);
        return origin + direction * len;
    }

    SR_NODISCARD SR_MATH_NS::FVector3 Ray::Origin3D() const { return origin; }
    SR_NODISCARD SR_MATH_NS::FVector3 Ray::Direction3D() const { return direction; }

    Ray::Ray() = default;
    Ray::~Ray() = default;

#define SR_MAKE_TEMPLATE_VECTOR4_IMPL(T) \
    template<> Vector4<T>::Vector4() {\
        x = y = z = w = 0;\
    }\
\
    template<> Vector4<T>::Vector4(T scalar) {\
        x = y = z = w = scalar;\
    }\
\
    template<> Vector4<T>::Vector4(const glm::vec4& vec4) {\
        x = vec4.x;\
        y = vec4.y;\
        z = vec4.z;\
        w = vec4.w;\
    }\
\
    template<> Vector4<T>::Vector4(T _x, T _y, T _z, T _w)\
        : x(_x)\
        , y(_y)\
        , z(_z)\
        , w(_w)\
    { }\
\
    template<> Vector4<T>::Vector4(const Vector3<T>& v, T w)\
        : x(v.x)\
        , y(v.y)\
        , z(v.z)\
        , w(w)\
    { }\
\
    template<> SR_NODISCARD Vector4<T> Vector4<T>::Normalize() const {\
        const T length = Length();\
        return (*this) * (static_cast<T>(1) / (length > SR_FLT_EPSILON ? length : SR_FLT_EPSILON));\
    }\
\
    template<> const T& Vector4<T>::operator[](int32_t axis) const {\
        return coord[axis];\
    }\
\
    template<> T& Vector4<T>::operator[](int32_t axis) {\
        return coord[axis];\
    }\
\
    template<> bool Vector4<T>::operator==(const Vector4<T>& v) const {\
        return SR_EQUALS(x, v.x) && SR_EQUALS(y, v.y) && SR_EQUALS(z, v.z) && SR_EQUALS(w, v.w);\
    }\
\
    template<> bool Vector4<T>::operator!=(const Vector4<T>& v) const {\
        return !(*this == v);\
    }\
\
    template<> template<typename U> Vector4<T>& Vector4<T>::operator*=(U p_scalar) {\
        x *= p_scalar;\
        y *= p_scalar;\
        z *= p_scalar;\
        w *= p_scalar;\
        return *this;\
    }\
\
    template<> template<typename U> Vector4<T>& Vector4<T>::operator/=(U p_scalar) {\
        x /= p_scalar;\
        y /= p_scalar;\
        z /= p_scalar;\
        w /= p_scalar;\
        return *this;\
    }\
\
    template<> template<typename U> Vector4<T>& Vector4<T>::operator*=(const Vector4<U>& p_v) {\
        x *= p_v.x;\
        y *= p_v.y;\
        z *= p_v.z;\
        w *= p_v.w;\
        return *this;\
    }\
\
    template<> template<typename U> Vector4<T> Vector4<T>::operator-(const Vector4<U> &p_v) const {\
        return Vector4(x - p_v.x, y - p_v.y, z - p_v.z, w - p_v.w);\
    }\
\
    template<> template<typename U> Vector4<T>& Vector4<T>::operator/=(const Vector4<U>& p_v) {\
        x /= p_v.x;\
        y /= p_v.y;\
        z /= p_v.z;\
        w /= p_v.w;\
        return *this;\
    }\
\
    template<> template<typename U> Vector4<T>& Vector4<T>::operator%=(const Vector4<U>& p_v) {\
        x %= p_v.x;\
        y %= p_v.y;\
        z %= p_v.z;\
        w %= p_v.w;\
        return *this;\
    }\
\
    template<> template<typename U> Vector4<T> Vector4<T>::operator/(const U& v) const {\
        return Vector4(x / v, y / v, z / v, w / v);\
    }\
\
    template<> template<typename U> Vector4<T> Vector4<T>::operator*(const Vector4<U> &p_v) const {\
        return Vector4(x * p_v.x, y * p_v.y, z * p_v.z, w * p_v.w);\
    }\
\
    template<> template<typename U> Vector4<T> Vector4<T>::operator+(const Vector4<U>& v) const {\
        return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);\
    }\
\
    template<> SR_NODISCARD Vector4<T> Vector4<T>::Cross(const Vector4& v) const {\
        Vector4 res;\
        res.x = y * v.z - z * v.y;\
        res.y = z * v.x - x * v.z;\
        res.z = x * v.y - y * v.x;\
        res.w = 0.f;\
        return res;\
    }\
\
    template<> SR_NODISCARD T Vector4<T>::SqrMagnitude() const { return x * x + y * y + z * z + w * w; }\
\
    template<> Vector4<T> Vector4<T>::UnitX() { return Vector4(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0)); }\
    template<> Vector4<T> Vector4<T>::UnitY() { return Vector4(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0)); }\
    template<> Vector4<T> Vector4<T>::UnitZ() { return Vector4(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0)); }\
    template<> Vector4<T> Vector4<T>::UnitW() { return Vector4(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)); }\
\
    template<> Vector4<T> Vector4<T>::operator-() const { return Vector4(-x, -y, -z, -w); }\
\
    template<> template<typename U> Vector4<T> Vector4<T>::operator*(U p_scalar) const {\
        return Vector4(x * p_scalar, y * p_scalar, z * p_scalar, w * p_scalar);\
    }\
\
    template<> SR_NODISCARD Vector3<T> Vector4<T>::XYZ() const noexcept { return Vector3<T>(x, y, z); }\
    template<> SR_NODISCARD Vector2<T> Vector4<T>::XY() const noexcept { return Vector2<T>(x, y); }\
\
    template<> SR_NODISCARD T Vector4<T>::Dot3(const Vector3<T>& v) const {\
        return (x * v.x) + (y * v.y) + (z * v.z);\
    }\
\
    template<> template<typename U> SR_NODISCARD Vector4<U> SR_FASTCALL Vector4<T>::Cast() const noexcept { return Vector4<U>(\
        static_cast<U>(x),\
            static_cast<U>(y),\
            static_cast<U>(z),\
            static_cast<U>(w)\
        );\
    }\
\
    template<> Unit Vector4<T>::DistanceToPlane(const SR_MATH_NS::FVector3& point) const {\
        return Dot3(point) + w;\
    }\
\
    template<> SR_NODISCARD bool Vector4<T>::IsFinite() const noexcept {\
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);\
    }\
\
    template<> SR_NODISCARD T Vector4<T>::Dot(const Vector4<T>& v) const\
    {\
        return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w);\
    }\
\
    template<> SR_NODISCARD T Vector4<T>::Length() const {\
        return static_cast<T>(sqrt(x * x + y * y + z * z + w * w));\
    }\
\
    template<> template<typename U, typename Y> SR_NODISCARD Vector4<T> Vector4<T>::Clamp(U _max, Y _min) const {\
        return Vector4(\
            SR_CLAMP(x, static_cast<T>(_min), static_cast<T>(_max)),\
            SR_CLAMP(y, static_cast<T>(_min), static_cast<T>(_max)),\
            SR_CLAMP(z, static_cast<T>(_min), static_cast<T>(_max)),\
            SR_CLAMP(w, static_cast<T>(_min), static_cast<T>(_max))\
        );\
    }\

    //SR_MAKE_TEMPLATE_VECTOR4_IMPL(float_t)
}