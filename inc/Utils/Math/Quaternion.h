//
// Created by Nikita on 01.03.2021.
//

#ifndef GAMEENGINE_QUATERNION_H
#define GAMEENGINE_QUATERNION_H

#include <Utils/Math/Mathematics.h>

namespace SR_MATH_NS {
    template<typename T> struct Vector3;

    class Matrix4x4;

    class SR_DLL_EXPORT Quaternion {
        friend Vector3<Unit>;
    public:
        union {
            struct {
                float_t x;
                float_t y;
                float_t z;
                float_t w;
            };

            glm::quat self;
        };
    public:
        SR_NODISCARD SR_FORCE_INLINE glm::quat ToGLM() const noexcept { return self; }
        SR_NODISCARD Matrix4x4 ToMat4x4() const;
        SR_NODISCARD SR_FORCE_INLINE glm::mat4 ToMat4x4GLM() const noexcept { return mat4_cast(self); }
        SR_NODISCARD Vector3<Unit> EulerAngle() const;
        SR_NODISCARD Quaternion Rotate(const Vector3<Unit>& v) const;

        constexpr Quaternion(const Quaternion &p_q) {
            self = p_q.self;
        }

        constexpr Quaternion() {
            self = glm::quat();
        }

        //, bool inRads = false
        Quaternion(const Vector3<Unit> &p_euler);

        Quaternion(const glm::quat &q) {
            self = q;
        }

        constexpr explicit Quaternion(Unit x, Unit y, Unit z, Unit w) {
            self.x = x;
            self.y = y;
            self.z = z;
            self.w = w;
        }

        static Quaternion FromEuler(const Vector3<Unit>& euler);

        constexpr static Quaternion Identity() {
            return Quaternion(0.0, 0.0, 0.0, 1.0);
        }

        [[nodiscard]] Quaternion Inverse() const {
            return Quaternion(glm::inverse(self));
        }

        [[nodiscard]] std::string ToString() const {
            return "(" + std::to_string(self.x) + ", " + std::to_string(self.y) + ", " + std::to_string(self.z) + ", " + std::to_string(self.w) + ")";
        }

        SR_FORCE_INLINE void operator+=(const Quaternion &p_q) {
            self += p_q.self;
        }
        SR_FORCE_INLINE void operator-=(const Quaternion &p_q) {
            self -= p_q.self;
        }
        SR_FORCE_INLINE void operator*=(const double &s){
            self *= s;
        }
        SR_FORCE_INLINE void operator/=(const double &s) {
            self *= 1.0 / s;
        }
        SR_FORCE_INLINE Quaternion operator+(const Quaternion &q2) const {
            const Quaternion &q1 = *this;
            return Quaternion(q1.self + q2.self);
        }
        SR_FORCE_INLINE Quaternion operator-(const Quaternion &q2) const {
            const Quaternion &q1 = *this;
            return Quaternion(q1.self - q2.self);
        }
        SR_FORCE_INLINE Quaternion operator-() const {
            const Quaternion &q2 = *this;
            return Quaternion(-q2.self);
        }
        SR_FORCE_INLINE Quaternion operator*(const double &s) const {
            glm::quat q = self;
            q *= s;
            return Quaternion(q);
        }
        SR_FORCE_INLINE Quaternion operator/(const double &s) const {
            glm::quat q = self;
            q *= 1.0 / s;
            return Quaternion(q);
        }

        Vector3<Unit> operator*(const Vector3<Unit> &v) const;
        Vector3<Unit> operator/(const Vector3<Unit> &v) const;
        Quaternion operator*(const Quaternion& rhs) const {
            return Quaternion(self * rhs.self);
        }
    };
}

#endif //GAMEENGINE_QUATERNION_H
