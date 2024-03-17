//
// Created by Nikita on 01.03.2021.
//

#ifndef GAMEENGINE_VECTOR3_H
#define GAMEENGINE_VECTOR3_H

#include <Utils/Math/Vector2.h>
#include <Utils/Math/Quaternion.h>

namespace SR_MATH_NS {
    template<typename T> struct SR_DLL_EXPORT Vector3 {
    public:
        union {
            struct {
                T x;
                T y;
                T z;
            };

            T coord[3] = { 0 };
        };
    public:
        constexpr SR_FORCE_INLINE Vector3() {
            x = 0;
            y = 0;
            z = 0;
        }
        template<typename U> constexpr SR_FORCE_INLINE explicit Vector3(const Vector3<U>& vec) {
            x = static_cast<T>(vec.x);
            y = static_cast<T>(vec.y);
            z = static_cast<T>(vec.z);
        }
        SR_FORCE_INLINE constexpr explicit Vector3(const float* vec) {
            x = (Unit)vec[0];
            y = (Unit)vec[1];
            z = (Unit)vec[2];
        }
        SR_FORCE_INLINE constexpr explicit Vector3(const uint8_t* axis) {
            x = (Unit)axis[0];
            y = (Unit)axis[1];
            z = (Unit)axis[2];
        }
        SR_FORCE_INLINE constexpr Vector3(T p_x, T p_y, T p_z) {
            x = p_x;
            y = p_y;
            z = p_z;
        }
        SR_FORCE_INLINE constexpr Vector3(T p) {
            x = p;
            y = p;
            z = p;
        }

        Vector3(const glm::vec3& v) {
            x = v.x;
            y = v.y;
            z = v.z;
        }

    public:
        template<typename U> static Vector3<T> XY(const Vector2<U>& v, U value) {
            return Vector3<T>(static_cast<T>(v.x), static_cast<T>(v.y), static_cast<T>(value));
        }
        template<typename U> static Vector3<T> XZ(const Vector2<U>& v, U value) {
            return Vector3<T>(static_cast<T>(v.x), static_cast<T>(value), static_cast<T>(v.y));
        }
        template<typename U> static Vector3<T> YZ(const Vector2<U>& v, U value) {
            return Vector3<T>(static_cast<T>(value), static_cast<T>(v.x), static_cast<T>(v.y));
        }

        static constexpr Vector3<T> Zero() { return Vector3(static_cast<T>(0)); }
        static constexpr Vector3<T> One() { return Vector3(static_cast<T>(1)); }

        static constexpr Vector3<T> UnitX() { return Vector3(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0)); }
        static constexpr Vector3<T> UnitY() { return Vector3(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0)); }
        static constexpr Vector3<T> UnitZ() { return Vector3(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1)); }

        static constexpr Vector3<T> Right() { return UnitX(); }
        static constexpr Vector3<T> Up() { return UnitY(); }
        static constexpr Vector3<T> Forward() { return UnitZ(); }

        static constexpr Vector3<T> AxisByIndex(uint8_t axis) {
            switch (axis) {
                case 0: return UnitX();
                case 1: return UnitY();
                case 2: return UnitZ();
                default:
                    return Zero();
            }
        }

        template<typename U> static Vector3<T> XY(const Vector2<U>& v) { return XY(v, 0); }
        template<typename U> static Vector3<T> XZ(const Vector2<U>& v) { return XZ(v, 0); }
        template<typename U> static Vector3<T> YZ(const Vector2<U>& v) { return YZ(v, 0); }

    public:
        template<typename U> SR_NODISCARD Vector3<U> SR_FASTCALL Cast() const noexcept { return Vector3<U>(
                    static_cast<U>(x),
                    static_cast<U>(y),
                    static_cast<U>(z)
            );
        }

        SR_NODISCARD T X() const noexcept { return x; }
        SR_NODISCARD T Y() const noexcept { return y; }
        SR_NODISCARD T Z() const noexcept { return z; }

        SR_NODISCARD Vector2<T> XY() const { return Vector2<T>(x, y); }
        SR_NODISCARD Vector2<T> XZ() const { return Vector2<T>(x, z); }
        SR_NODISCARD Vector2<T> YZ() const { return Vector2<T>(y, z); }

        SR_NODISCARD Vector3 Singular(const Vector3& segment) const { return Vector3(
                     x > 0 ? x + segment.x : x - segment.x,
                     y > 0 ? y + segment.y : y - segment.y,
                     z > 0 ? z + segment.z : z - segment.z
                );
        }
        SR_NODISCARD Vector3 DeSingular(const Vector3& segment) const { return Vector3(
                    x > 0 ? x - segment.x : x,
                    y > 0 ? y - segment.y : y,
                    z > 0 ? z - segment.z : z
            );
        }

        SR_NODISCARD T Max() const {
            return x > y && x > z ? x : y > x && y > z ? y : z;
        }

        SR_NODISCARD T Min() const {
            return x < y && x < z ? x : y < x && y < z ? y : z;
        }

        SR_NODISCARD Vector3 Max3() const { return Vector3(Max()); }
        SR_NODISCARD Vector3 Min3() const { return Vector3(Min()); }

        SR_NODISCARD bool Empty() const {
            return (x == 0 && y == 0 && z == 0);
        }

        SR_NODISCARD bool HasZero() const {
            return (x == 0 || y == 0 || z == 0);
        }

        SR_NODISCARD bool IsInfinity() const {
            return (x == UnitMAX && y == UnitMAX && z == UnitMAX); // TODO
        }

        SR_NODISCARD Vector3 Radians() const noexcept {
            return { static_cast<T>(SR_RAD(x)), static_cast<T>(SR_RAD(y)), static_cast<T>(SR_RAD(z)) };
        }
        SR_NODISCARD Vector3 Degrees() const noexcept {
            return { static_cast<T>(SR_DEG(x)), static_cast<T>(SR_DEG(y)), static_cast<T>(SR_DEG(z)) };
        }

        SR_NODISCARD T AngleCoefficientRadians(const Vector3& vector3) const {
            const T dot = Dot(vector3);
            const T length1 = Length();
            const T length2 = vector3.Length();

            if (length1 == static_cast<T>(0) || length2 == static_cast<T>(0)) {
                return 0.0;
            }

            /// Вычисление угла между векторами в радианах
            const T angle = std::acos(dot / (length1 * length2));
            return angle;
        }

        SR_NODISCARD T AngleCoefficientEuler(const Vector3& vector3) const {
            return SR_DEG(AngleCoefficientRadians(vector3));
        }

        SR_NODISCARD T AngleCoefficient(const Vector3& vector3) const {
            /// Преобразование угла в коэффициент от -1 до 1
            const T coefficient = std::cos(AngleCoefficientRadians(vector3));
            return coefficient;
        }

        SR_NODISCARD Quaternion LookAt(const Vector3& target) const {
            const auto&& direction = (target - *this).Normalized();
            const auto&& cosAngle = UnitZ().Dot(direction);

            if (cosAngle >= 1.0 - static_cast<T>(SR_EPSILON)) {
                return Quaternion::Identity();
            }

            else if (cosAngle <= -1.0 + static_cast<T>(SR_EPSILON)) {
                return Quaternion(UnitY(), SR_PI);
            }

            const auto&& axis = UnitZ().Cross(direction);
            const auto&& angle = std::acos(cosAngle);

            return Quaternion(axis, angle);
        }

        SR_NODISCARD T SqrMagnitude() const { return x * x + y * y + z * z; }

        SR_NODISCARD T Angle(const Vector3& to) const {
            /// sqrt(a) * sqrt(b) = sqrt(a * b) -- valid for real numbers

            const T denominator = static_cast<T>(sqrt(SqrMagnitude() * to.SqrMagnitude()));
            if (denominator < static_cast<T>(SR_EPSILON_NORMAL_SQRT)) {
                return static_cast<T>(0);
            }

            const T dot = SR_CLAMP(Dot(to) / denominator, static_cast<T>(-1), static_cast<T>(1));
            return static_cast<T>(std::acos(dot) * SR_RAD_2_DEG);
        }

        SR_NODISCARD Vector3 ProjectOnPlane(const Vector3& planeNormal) const {
            const T sqrMag = planeNormal.Dot(planeNormal);

            if (sqrMag < SR_EPSILON) {
                return *this;
            }

            auto&& dot = Dot(planeNormal);

            return Vector3(
                x - planeNormal.x * dot / sqrMag,
                y - planeNormal.y * dot / sqrMag,
                z - planeNormal.z * dot / sqrMag
           );
        }

        SR_NODISCARD T SignedAngle(const Vector3& to, const Vector3& axis) const {
            const T unsignedAngle = Angle(to);

            const T cross_x = y * to.z - z * to.y;
            const T cross_y = z * to.x - x * to.z;
            const T cross_z = x * to.y - y * to.x;
            const T sign = SR_MATH_NS::Sign(axis.x * cross_x + axis.y * cross_y + axis.z * cross_z);

            return unsignedAngle * sign;
        }

        SR_NODISCARD Quaternion AngleAxis(const Vector3& target, const Vector3& axis = Vector3::UnitZ()) const {
            auto&& direction = (target - *this).Normalized();

            auto&& crossAxis = axis.Cross(direction);
            const Unit angle = acos(axis.Dot(direction));

            return Quaternion(crossAxis, angle);
        }

        SR_NODISCARD Vector3 Angle(const Vector3& vector3) {
            Vector3 angle;

            angle.x = (T)(std::atan2(vector3.y, vector3.z) - atan2(y, z));
            angle.y = (T)(std::atan2(vector3.x, vector3.z) - atan2(x, z));
            angle.z = (T)(std::atan2(vector3.y, vector3.x) - atan2(y, x));

            Vector3 degrees = Vector3(T(180)) * angle / SR_PI;
            return (Vector3(T(360)) + degrees.Round()) % Vector3(T(360));
        }

        SR_NODISCARD std::string ToString() const {
            return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
        }

        SR_NODISCARD T SR_FASTCALL Distance(const Vector3& point) const {
            return sqrt(
                    pow(point.x - x, 2) +
                    pow(point.y - y, 2) +
                    pow(point.z - z, 2)
            );
        }

        SR_NODISCARD Vector3 Direction(const Vector3& point) const {
            if (point == *this)
                return Vector3();
            Vector3 heading = point - (*this);
            T distance = Magnitude(heading);
            return heading / distance;
        }

        SR_NODISCARD bool IsEquals(const Vector3& value, Unit tolerance) const noexcept {
            if (!SR_EQUALS_T(x, value.x, tolerance)) {
                return false;
            }

            if (!SR_EQUALS_T(y, value.y, tolerance)) {
                return false;
            }

            if (!SR_EQUALS_T(z, value.z, tolerance)) {
                return false;
            }

            SR_NOOP;

            return true;
        }

        SR_NODISCARD Vector3 Limits(int lim) const {
            int xi = (int)x / lim;
            int yi = (int)y / lim;
            int zi = (int)z / lim;

            T xd = x - lim * (T)xi;
            T yd = y - lim * (T)yi;
            T zd = z - lim * (T)zi;

            return Vector3(xd, yd, zd);
        }

        SR_NODISCARD bool IsFinite() const noexcept {
            /// если будет inf или nan, то вернет false
            return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
        }

        SR_NODISCARD bool ContainsNaN() const {
            return static_cast<float>(x) == SR_NAN || static_cast<float>(y) == SR_NAN || static_cast<float>(z) == SR_NAN;
        }

        SR_NODISCARD Vector3 Inverse() const {
            return Vector3(-x, -y, -z);
        }

        SR_NODISCARD Vector3 SR_FASTCALL InverseAxis(AxisFlag axis) const {
            Vector3 v = *this;

            switch (axis) {
                case Axis::X: v[0] = -v[0]; break;
                case Axis::Y: v[1] = -v[1]; break;
                case Axis::Z: v[2] = -v[2]; break;
                case Axis::XY: {
                    v[0] = -v[0];
                    v[1] = -v[1];
                    break;
                }
                case Axis::YZ: {
                    v[1] = -v[1];
                    v[2] = -v[2];
                    break;
                }
                case Axis::XZ: {
                    v[0] = -v[0];
                    v[2] = -v[2];
                    break;
                }
                case Axis::XYZ: {
                    v[0] = -v[0];
                    v[1] = -v[1];
                    v[2] = -v[2];
                    break;
                }
                default:
                    break;
            }

            return v;
        }

        SR_NODISCARD Vector3 SR_FASTCALL Swap(Axis axis) const {
            Vector3 v = *this;

            switch (axis) {
                case Axis::XY: std::swap(v.x, v.y); break;
                case Axis::YZ: std::swap(v.y, v.z); break;
                case Axis::XZ: std::swap(v.x, v.z); break;
                default:
                    break;
            }

            return v;
        }

        SR_NODISCARD Vector3 ZeroAxis(AxisFlag axis) const {
            Vector3 v = *this;

            switch (axis) {
                case Axis::X: v[0] = 0; break;
                case Axis::Y: v[1] = 0; break;
                case Axis::Z: v[2] = 0; break;
                case Axis::XY: v[0] = v[1] = 0; break;
                case Axis::XZ: v[0] = v[1] = 0; break;
                case Axis::YZ: v[1] = v[2] = 0; break;
                case Axis::XYZ: v[0] = v[1] = v[2] = 0; break;
                default:
                    break;
            }

            return v;
        }

        SR_NODISCARD SR_FORCE_INLINE Vector3 SR_FASTCALL Lerp(const Vector3& vector3, Unit t) const noexcept {
            return (Vector3)(*this + (vector3 - *this) * t);
        }

        SR_NODISCARD Vector3 Normalized() const {
            return Normalize();
        }

        SR_NODISCARD Vector3 Normalize() const {
            auto&& value = x * x + y * y + z * z;

            if (value > 0) {
                T len = static_cast<T>(std::sqrt(value));

                Vector3 vec3 = *this;

                if (len != static_cast<T>(0.)) {
                    vec3.x /= len;
                    vec3.y /= len;
                    vec3.z /= len;
                }

                return vec3;
            }

            return *this;
        }

        SR_NODISCARD T SquaredNorm() const noexcept {
            return x * x + y * y + z * z;
        }

        SR_NODISCARD Vector3<T> Clamp(const Vector3<T>& upper, const Vector3<T>& lover) const {
            return Vector3<T>(
                SR_CLAMP(x, lover.x, upper.x),
                SR_CLAMP(y, lover.y, upper.y),
                SR_CLAMP(z, lover.z, upper.z)
            );
        }

        SR_NODISCARD Quaternion ToQuat() const;

        SR_FORCE_INLINE constexpr const T &operator[](int p_axis) const {
            return coord[p_axis];
        }

        SR_FORCE_INLINE constexpr T &operator[](int p_axis) {
            return coord[p_axis];
        }

        SR_NODISCARD SR_FORCE_INLINE T Length() const {
            if constexpr (std::is_same_v<T, float_t> || std::is_same_v<T, float>) {
                return static_cast<T>(sqrtf(x * x + y * y + z * z));
            }
            else {
                return static_cast<T>(sqrt(x * x + y * y + z * z));
            }
        }

        SR_NODISCARD Vector3 Replace(int from, int to) const {
            return Vector3(
                    x == from ? to : x,
                    y == from ? to : y,
                    z == from ? to : z);
        }

        SR_NODISCARD Vector3 Abs() const {
            return Vector3(static_cast<T>(abs(x)), static_cast<T>(abs(y)), static_cast<T>(abs(z)));
        }

        SR_NODISCARD Vector3 Sin() const {
            return Vector3(static_cast<T>(sin(x)), static_cast<T>(sin(y)), static_cast<T>(sin(z)));
        }

        SR_NODISCARD Vector3 Cos() const {
            return Vector3(static_cast<T>(cos(x)), static_cast<T>(cos(y)), static_cast<T>(cos(z)));
        }

        SR_NODISCARD Vector3 Round() const {
            return Vector3(static_cast<T>(std::round(x)), static_cast<T>(std::round(y)), static_cast<T>(std::round(z)));
        }

        SR_NODISCARD Vector3 FixEulerAngles() const {
            return Vector3(FixAxis(x), FixAxis(y), FixAxis(z));
        }

        SR_NODISCARD T Dot(Vector3 p_b) const { return x * p_b.x + y * p_b.y + z * p_b.z; }

        SR_NODISCARD Vector3 Cross(const Vector3 &p_b) const {
            Vector3 ret(
                    (y * p_b.z) - (z * p_b.y),
                    (z * p_b.x) - (x * p_b.z),
                    (x * p_b.y) - (y * p_b.x));

            return ret;
        }

        SR_NODISCARD Vector3<T> Rotate(const Quaternion& q) const;

        template<typename U> SR_FORCE_INLINE Vector3 &operator+=(const Vector3<U> &p_v){
            x += p_v.x;
            y += p_v.y;
            z += p_v.z;
            return *this;
        }

        template<typename U> SR_FORCE_INLINE Vector3 SR_FASTCALL operator+(const Vector3<U> &p_v) const noexcept {
            return Vector3(x + p_v.x, y + p_v.y, z + p_v.z);
        }

        template<typename U> SR_FORCE_INLINE Vector3 operator%(const Vector3<U> &p_v) const {
            return Vector3(
                    static_cast<int32_t>(x) % static_cast<int32_t>(p_v.x),
                    static_cast<int32_t>(y) % static_cast<int32_t>(p_v.y),
                    static_cast<int32_t>(z) % static_cast<int32_t>(p_v.z)
                );
        }

        template<typename U> SR_FORCE_INLINE Vector3 &operator-=(const Vector3<U> &p_v) {
            x -= p_v.x;
            y -= p_v.y;
            z -= p_v.z;
            return *this;
        }
        template<typename U> SR_FORCE_INLINE Vector3 operator-(const Vector3<U> &p_v) const {
            return Vector3(x - p_v.x, y - p_v.y, z - p_v.z);
        }
        template<typename U> SR_FORCE_INLINE Vector3 &operator*=(const Vector3<U> &p_v) {
            x *= p_v.x;
            y *= p_v.y;
            z *= p_v.z;
            return *this;
        }
        template<typename U> SR_FORCE_INLINE Vector3 &operator%=(const Vector3<U> &p_v) {
            x %= p_v.x;
            y %= p_v.y;
            z %= p_v.z;
            return *this;
        }

        template<typename U> SR_FORCE_INLINE Vector3 operator*(const Vector3<U> &p_v) const {
            return Vector3(x * p_v.x, y * p_v.y, z * p_v.z);
        }

        template<typename U> SR_FORCE_INLINE Vector3 &operator/=(const Vector3<U> &p_v) {
            x /= p_v.x;
            y /= p_v.y;
            z /= p_v.z;
            return *this;
        }
        template<typename U> SR_FORCE_INLINE Vector3 operator/(const Vector3<U> &p_v) const {
            return Vector3(x / p_v.x, y / p_v.y, z / p_v.z);
        }

        template<typename U> SR_FORCE_INLINE Vector3 &operator*=(U p_scalar) {
            x *= p_scalar;
            y *= p_scalar;
            z *= p_scalar;
            return *this;
        }
        template<typename U> SR_FORCE_INLINE Vector3 operator*(U p_scalar) const {
            return Vector3(x * p_scalar, y * p_scalar, z * p_scalar);
        }
        template<typename U> SR_FORCE_INLINE Vector3 &operator/=(U p_scalar) {
            x /= p_scalar;
            y /= p_scalar;
            z /= p_scalar;
            return *this;
        }
        template<typename U> SR_FORCE_INLINE Vector3 operator/(U p_scalar) const {
            return Vector3(x / p_scalar, y / p_scalar, z / p_scalar);
        }

        template<typename U> SR_FORCE_INLINE bool operator>(U p_scalar) const { return *this > Vector3<U>(p_scalar); }
        template<typename U> SR_FORCE_INLINE bool operator<(U p_scalar) const { return *this < Vector3<U>(p_scalar); }
        template<typename U> SR_FORCE_INLINE bool operator>=(U p_scalar) const { return *this >= Vector3<U>(p_scalar); }
        template<typename U> SR_FORCE_INLINE bool operator<=(U p_scalar) const { return *this <= Vector3<U>(p_scalar); }
        template<typename U> SR_FORCE_INLINE bool operator==(U p_scalar) const { return *this == Vector3<U>(p_scalar); }
        template<typename U> SR_FORCE_INLINE bool operator!=(U p_scalar) const { return *this != Vector3<U>(p_scalar); }

        SR_FORCE_INLINE Vector3 operator-() const { return Vector3(-x, -y, -z); }
        SR_FORCE_INLINE Vector3 operator+() const { return *this; }

        SR_FORCE_INLINE bool operator==(const Vector3 &p_v) const { return SR_EQUALS(x, p_v.x) && SR_EQUALS(y, p_v.y) && SR_EQUALS(z, p_v.z); }
        SR_FORCE_INLINE bool operator!=(const Vector3 &p_v) const { return !SR_EQUALS(x, p_v.x) || !SR_EQUALS(y, p_v.y) || !SR_EQUALS(z, p_v.z); }

        SR_FORCE_INLINE bool operator<=(const Vector3 &p_v) const { return x <= p_v.x && y <= p_v.y && z <= p_v.z; }
        SR_FORCE_INLINE bool operator>=(const Vector3 &p_v) const { return x >= p_v.x && y >= p_v.y && z >= p_v.z; }
        SR_FORCE_INLINE bool operator<(const Vector3 &p_v) const { return x < p_v.x && y < p_v.y && z < p_v.z; }
        SR_FORCE_INLINE bool operator>(const Vector3 &p_v) const { return x > p_v.x && y > p_v.y && z > p_v.z; }

    public:
        SR_NODISCARD glm::vec3 ToGLM() const noexcept {
            return *reinterpret_cast<glm::vec3*>((void*)this);
        }
        static Unit Magnitude(Vector3 vec) {
            return sqrt(SR_SQUARE(vec.x) + SR_SQUARE(vec.y) + SR_SQUARE(vec.z));
        }

        static T Dot(Vector3 lhs, Vector3 rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }
        static Vector3 Cross(const Vector3 &p_a, const Vector3 &p_b) {
            Vector3 ret(
                    (p_a.y * p_b.z) - (p_a.z * p_b.y),
                    (p_a.z * p_b.x) - (p_a.x * p_b.z),
                    (p_a.x * p_b.y) - (p_a.y * p_b.x));
            return ret;
        }

        static T FixAxis(T axis) {
            if (axis == 0)
                return static_cast<T>(CMP_BIG_EPSILON);

            T absolute = std::abs(axis);
            if (SR_EQUALS(absolute, 90) || SR_EQUALS(absolute, 180) || SR_EQUALS(absolute, 270) || SR_EQUALS(absolute, 360))
                return axis - static_cast<T>(CMP_BIG_EPSILON);

            return axis;
        }
    };

    // bool inRads
    template<typename T>
    Quaternion Vector3<T>::ToQuat() const {
        return Quaternion(*this); //, inRads
    }

    template<typename T>
    Vector3<T> Vector3<T>::Rotate(const Quaternion &q) const  {
        // Extract the vector part of the quaternion
        Vector3 u(q.self.x, q.self.y, q.self.z);

        // Extract the scalar part of the quaternion
        auto s = q.self.w;

        Vector3 v = *this;

        // Do the math
        return u * 2.0f * Dot(u, v)
               + v * (s*s - Dot(u, u))
               + Cross(u, v) * 2.0f * s;
    }

    typedef Vector3<Unit> FVector3;
    typedef Vector3<int32_t> IVector3;
    typedef Vector3<uint32_t> UVector3;
    typedef Vector3<bool> BVector3;

    SR_INLINE static const FVector3 InfinityFV3 = FVector3 { UnitMAX, UnitMAX, UnitMAX };
    SR_INLINE static const FVector3 CmpEpsilonFV3 = FVector3 {
            static_cast<Unit>(CMP_EPSILON),
            static_cast<Unit>(CMP_EPSILON),
            static_cast<Unit>(CMP_EPSILON),
    };
}

namespace std {
    template <class T> static inline void hash_vector3_combine(std::size_t & s, const T & v) {
        std::hash<T> h;
        s^= h(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
    }

    template<typename U> struct hash<SR_MATH_NS::Vector3<U>> {
        size_t operator()(SR_MATH_NS::Vector3<U> const& vec) const {
            std::size_t res = 0;
            hash_vector3_combine<U>(res, vec.x);
            hash_vector3_combine<U>(res, vec.y);
            hash_vector3_combine<U>(res, vec.z);
            return res;
        }
    };
}

#endif //GAMEENGINE_VECTOR3_H
