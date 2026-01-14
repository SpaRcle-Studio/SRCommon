//
// Created by Nikita on 01.03.2021.
//

#include <Utils/Math/Quaternion.h>
#include <Utils/Math/Vector3.h>
#include <Utils/Math/Matrix3x3.h>
#include <Utils/Math/Matrix4x4.h>

namespace SR_MATH_NS {
    Vector3<Unit> Quaternion::EulerAngle() const {
        return Vector3<Unit>(glm::eulerAngles(glm::normalize(self))).Degrees();
    }

    /*Quaternion::Quaternion(const Vector3<Unit>& axis, Unit angle) {
        const Unit halfAngle = angle * static_cast<Unit>(.5f);
        const Unit s = (Unit)SR_SIN(halfAngle);

        auto&& normalized = axis.Normalized();

        x = normalized.x * s;
        y = normalized.y * s;
        z = normalized.z * s;
        w = (Unit)SR_COS(halfAngle);
    }*/

    Quaternion::Quaternion(const Vector3<Unit>& eulerAngle) {
        Vector3<T> c = (eulerAngle * T(0.5)).Cos();
        Vector3<T> s = (eulerAngle * T(0.5)).Sin();

        this->w = c.x * c.y * c.z + s.x * s.y * s.z;
        this->x = s.x * c.y * c.z - c.x * s.y * s.z;
        this->y = c.x * s.y * c.z + s.x * c.y * s.z;
        this->z = c.x * c.y * s.z - s.x * s.y * c.z;

        ///self = p_euler.ToGLM();
    }

    Vector3<Unit> Quaternion::operator*(const Vector3<Unit> &v) const noexcept {
        Vector3<Unit> const QuatVector(x, y, z);
        Vector3<Unit> const uv = QuatVector.Cross(v);
        Vector3<Unit> const uuv = QuatVector.Cross(uv);

        return v + ((uv * w) + uuv) * static_cast<Unit>(2);
    }

    Quaternion Quaternion::Rotate(const Vector3<Unit> &v) const {
        if (v.Empty())
            return *this;

        glm::quat q = glm::rotate(self, 1.f, glm::radians(glm::vec3(v.x, v.y, v.z)));
        return Quaternion(q);
    }

    Matrix4x4 Quaternion::ToMat4x4() const {
        return Matrix4x4(mat4_cast(self));
    }

    Vector3<Unit> Quaternion::operator/(const Vector3<Unit> &v) const {
        glm::vec3 rot = EulerAngle().ToGLM();

        /// TODO: здесь должна быть инвертирована ось z?
        glm::fquat q = glm::vec3(1) / glm::vec3(
                rot.x,
                rot.y,
                -rot.z
        );

        return Vector3<Unit>(q * v.ToGLM());
    }

    bool Quaternion::IsFromToRotationValid(const Vector3<Unit> &from, const Vector3<Unit> &to) {
        const FVector3 f = from.NormalizeSafe();
        const FVector3 t = to.NormalizeSafe();

        const Unit cosTheta = f.Dot(t);
        const Unit kEps = SR_KINDA_SMALL_NUMBER_EPSILON;

        // почти совпадают
        if (cosTheta > (1.0f - kEps)) {
            return false;
        }

        // почти противоположны
        if (cosTheta < (-1.0f + kEps)) {
            return false;
        }

        // общий случай
        FVector3 axis = f.Cross(t);
        const Unit axisLenSq = axis.SqrMagnitude();
        if (axisLenSq < kEps) {
            return false;
        }

        return true;
    }

    Quaternion Quaternion::FromToRotation(const Vector3<Unit>& from, const Vector3<Unit>& to) {
        ///  const Vector3<Unit>& fallbackAxis, const Quaternion* pPrevious

        /**FVector3 f = from.Normalized();
        FVector3 t = to.Normalized();

        float cosTheta = f.Dot(t);

        if (cosTheta > 0.999999f) {
            // Векторы почти совпадают → Identity
            return Quaternion::Identity();
        }

        if (cosTheta < -0.999999f) {
            // Векторы почти противоположны → выбираем fallback ось
            FVector3 axis = f.Cross(FVector3(1,0,0));
            if (axis.SqrMagnitude() < 1e-6f) {
                axis = f.Cross(FVector3(0,1,0));
            }
            axis = axis.Normalize();
            return Quaternion::AngleAxis(180.0f, axis);
        }

        // Основной случай
        FVector3 cross = f.Cross(t);
        if (cross.SqrMagnitude() < 1e-6f) {
            // Векторы почти коллинеарны → оставляем identity
            return Quaternion::Identity();
        }
        cross = cross.Normalize();

        float angleDeg = SR_ACOS(cosTheta) * SR_RAD_2_DEG;
        return Quaternion::AngleAxis(angleDeg, cross);*/

        const Unit theta = SR_MATH_NS::FVector3::Dot(from.Normalize(), to.Normalize());
        if (theta >= Unit(1.0)) {
            return Quaternion::Identity();
        }

        if (theta <= Unit(-1.0)) {
            FVector3 axis = from.Cross(FVector3::Right());
            if (axis.SqrMagnitude() == Unit(0.0)) {
                axis = from.Cross(FVector3::Up());
            }

            return Quaternion::AngleAxis(Unit(180.0), axis);
        }

        return Quaternion::AngleAxis(SR_ACOS(theta) * static_cast<Unit>(SR_MATH_NS::Rad2Deg), SR_MATH_NS::FVector3::Cross(from, to).Normalize());

        /*double_t theta = from.CastToDouble().Normalize().Dot(to.CastToDouble().Normalize());
        if (theta >= 1.0) {
            return Quaternion::Identity();
        }

        if (theta <= -1.0) {
            DVector3 axis = from.CastToDouble().Cross(DVector3::Right());
            if (axis.SqrMagnitude() == 0.0) {
                axis = from.CastToDouble().Cross(DVector3::Up());
            }

            return Quaternion::AngleAxis(180.f, axis);
        }

        return Quaternion::AngleAxis(SR_ACOS(theta) * static_cast<double_t>(SR_RAD_2_DEG), from.CastToDouble().Cross(to.CastToDouble()).Normalize());*/


        /****const FVector3 f = from.NormalizeSafe();
        const FVector3 t = to.NormalizeSafe();

        const Unit cosTheta = f.Dot(t);
        const Unit kEps = SR_KINDA_SMALL_NUMBER_EPSILON;

        // почти совпадают
        if (cosTheta > (1.0f - kEps)) {
            return Quaternion::Identity();
        }

        // почти противоположны
        if (cosTheta < (-1.0f + kEps)) {
            FVector3 axis = fallbackAxis;

            if (axis.SqrMagnitude() < kEps) {
                // стабильная ось через previousBendAxis
                if (pPrevious) {
                    axis = FVector3::Right().Rotate(*pPrevious);
                    if (axis.SqrMagnitude() < kEps) {
                        axis = FVector3::Up().Rotate(*pPrevious);
                    }
                }
                else {
                    axis = f.Cross(FVector3(1, 0, 0));
                    if (axis.SqrMagnitude() < kEps) {
                        axis = f.Cross(FVector3(0, 1, 0));
                    }
                }
            }

            axis = axis.NormalizeSafe();

            // Срезаем угол чуть меньше 180°, чтобы не было резкого флипа
            return Quaternion::AngleAxis(179.9f, axis).NormalizeSafe();
        }

        // общий случай
        FVector3 axis = f.Cross(t);
        const Unit axisLenSq = axis.SqrMagnitude();
        if (axisLenSq < kEps) {
            return Quaternion::Identity();
        }

        axis /= SR_SQRT(axisLenSq);
        const Unit angle = SR_ACOS(glm::clamp(cosTheta, -1.0f, 1.0f));
        return Quaternion::AngleAxis(SR_DEG(angle), axis).NormalizeSafe();*/







        /*FVector3 f = from.Normalize();
        FVector3 t = to.Normalize();

        if (f.SqrMagnitude() < SR_KINDA_SMALL_NUMBER_EPSILON || t.SqrMagnitude() < SR_KINDA_SMALL_NUMBER_EPSILON) {
            // слишком маленький вектор → не вращаем
            return Quaternion::Identity();
        }

        // 2. Косинус угла между векторами
        float cosTheta = f.Dot(t);
        cosTheta = SR_CLAMP(cosTheta, -1.f, 1.f);

        // 3. Векторы почти совпадают → Identity
        if (cosTheta > 0.999999f) {
            return Quaternion::Identity();
        }

        // 4. Векторы почти противоположны
        if (cosTheta < -0.999999f) {
            FVector3 axis = fallbackAxis;

            // если fallbackAxis нулевой — тогда обеспечь ось
            if (axis.SqrMagnitude() < SR_KINDA_SMALL_NUMBER_EPSILON) {
                axis = f.Cross(FVector3(1, 0, 0));
                if (axis.SqrMagnitude() < SR_KINDA_SMALL_NUMBER_EPSILON) {
                    axis = f.Cross(FVector3(0, 1, 0));
                }
            }

            axis = axis.NormalizeSafe();
            return Quaternion::AngleAxis(180.0f, axis).NormalizeSafe();
        }

        // 5. Общий случай
        FVector3 rotationAxis = f.Cross(t);
        float axisLenSq = rotationAxis.LengthSqr();

        if (axisLenSq < SR_KINDA_SMALL_NUMBER_EPSILON) {
            // почти коллинеарны → Identity
            return Quaternion::Identity();
        }

        rotationAxis = rotationAxis / sqrt(axisLenSq); // нормализация
        float angle = acosf(cosTheta); // угол

        return Quaternion::AngleAxis(SR_DEG(angle), rotationAxis);*/
    }

    //Quaternion Quaternion::FromToRotation(const Vector3<Unit>& from, const Vector3<Unit>& to) {
       /*float theta = from.Normalize().Dot(to.Normalize());
       if (theta >= 1.f) {
           return Quaternion::Identity();
       }

       if (theta <= -1.f) {
           FVector3 axis = from.Cross(FVector3::Right());
           if (axis.SqrMagnitude() == 0.f) {
               axis = from.Cross(FVector3::Up());
           }

           return Quaternion::AngleAxis(180.f, axis);
       }

       return Quaternion::AngleAxis(SR_ACOS(theta) * SR_RAD_2_DEG, from.Cross(to).Normalize());*/




        /*FVector3 f = from.NormalizeSafe();
        FVector3 t = to.NormalizeSafe();

        if (f.SqrMagnitude() < 1e-8f || t.SqrMagnitude() < 1e-8f) {
            return Quaternion::Identity(); // защита degenerate case
        }

        float theta = f.Dot(t);
        theta = SR_CLAMP(theta, -1.f, 1.f);

        if (theta >= 1.f) return Quaternion::Identity();
        if (theta <= -1.f) {
            FVector3 axis = f.Cross(FVector3::Right());
            if (axis.SqrMagnitude() < 1e-8f) axis = f.Cross(FVector3::Up());
            return Quaternion::AngleAxis(180.f, axis.NormalizeSafe());
        }

        return Quaternion::AngleAxis(SR_ACOS(theta) * SR_RAD_2_DEG, f.Cross(t).NormalizeSafe());*/

       /*
        БОЛЕЕ МЕНЕЕ РАБОЧИЙ ВАРИАНТ

        const FVector3 f = from.NormalizeSafe();
       const FVector3 t = to.NormalizeSafe();

       const Unit cosTheta = f.Dot(t);

       // почти совпадают
       const Unit kEps = 1e-6f;
       if (cosTheta > (1.0f - kEps)) {
           return Quaternion::Identity();
       }

       // почти противоположны
       if (cosTheta < (-1.0f + kEps)) {
           // выберем ось, перпендикулярную f
           FVector3 ortho = FVector3(1,0,0).Cross(f);
           if (ortho.LengthSqr() < kEps) ortho = FVector3(0,1,0).Cross(f);
           ortho = ortho.NormalizeSafe();
           Quaternion q = Quaternion::AngleAxis(SR_DEG(SR_PI), ortho); // 180°
           return q.NormalizeSafe();
       }

       // общий случай
       FVector3 axis = f.Cross(t);
       const Unit axisLenSq = axis.LengthSqr();
       if (axisLenSq < 1e-12f) {
           // численно плохо — вернуть Identity (или небольшой поворот)
           return Quaternion::Identity();
       }

       axis = axis / SR_SQRT(axisLenSq);
       const Unit angle = SR_ACOS(glm::clamp(cosTheta, -1.0f, 1.0f));
       Quaternion q = Quaternion::AngleAxis(SR_DEG(angle), axis);
       return q.NormalizeSafe();

*/




      /*// 1. Нормализуем входные векторы безопасно
        FVector3 f = from.Normalize();
        FVector3 t = to.Normalize();

        if (f.SqrMagnitude() < 1e-8f || t.SqrMagnitude() < 1e-8f) {
            // слишком маленький вектор → не вращаем
            return Quaternion::Identity();
        }

        // 2. Косинус угла между векторами
        float cosTheta = f.Dot(t);
        cosTheta = SR_CLAMP(cosTheta, -1.f, 1.f);

        // 3. Векторы почти совпадают → Identity
        if (cosTheta > 0.999999f) {
            return Quaternion::Identity();
        }

        // 4. Векторы почти противоположны
        if (cosTheta < -0.999999f) {
            // эвристика Unity: сначала X, если коллинеарно → Y
            FVector3 axis = f.Cross(FVector3(1,0,0));
            if (axis.SqrMagnitude() < 1e-6f) {
                axis = f.Cross(FVector3(0,1,0));
            }
            axis = axis.NormalizeSafe();
            return Quaternion::AngleAxis(SR_DEG(SR_PI), axis);
        }

        // 5. Общий случай
        FVector3 rotationAxis = f.Cross(t);
        float axisLenSq = rotationAxis.LengthSqr();

        if (axisLenSq < 1e-8f) {
            // почти коллинеарны → Identity
            return Quaternion::Identity();
        }

        rotationAxis = rotationAxis / sqrt(axisLenSq); // нормализация
        float angle = acosf(cosTheta); // угол

        return Quaternion::AngleAxis(SR_DEG(angle), rotationAxis);*/




       /// нет дрожи но криво вращает
     /* // 1. Нормализуем входные векторы
       const FVector3 f = from.Normalized();
       const FVector3 t = to.Normalized();

       // 2. Считаем косинус угла
       const Unit cosTheta = f.Dot(t);

       // 3. Векторы почти совпадают
       if (cosTheta > 0.999999f) {
           return Quaternion::Identity();
       }

       // 4. Векторы почти противоположны
       if (cosTheta < -0.999999f) {
           // нужно выбрать любую ортонормальную ось
           FVector3 ortho = FVector3(1, 0, 0).Cross(f);
           if (ortho.LengthSqr() < 1e-6f) {
               ortho = FVector3(0, 1, 0).Cross(f);
           }
           ortho = ortho.Normalized();
           return Quaternion::AngleAxis(SR_PI, ortho); // поворот на 180° вокруг оси
       }

       // 5. Общий случай
       FVector3 rotationAxis = f.Cross(t);
       const Unit axisLenSq = rotationAxis.LengthSqr();

       if (axisLenSq < 1e-8f) {
           // векторы почти коллинеарны → возвращаем Identity
           return Quaternion::Identity();
       }

       rotationAxis = rotationAxis / sqrt(axisLenSq); // нормализация

       // угол между векторами
       const Unit angle = acosf(cosTheta);

       return Quaternion::AngleAxis(angle, rotationAxis);*/




      /* /// Кривая реализация
       const FVector3 f = from.Normalized();
       const FVector3 t = to.Normalized();

       const Unit cosTheta = f.Dot(t);
       FVector3 rotationAxis;

       if (cosTheta >= 1.0f - 1e-6f) {
           // векторы почти совпадают
           return Quaternion::Identity();
       }

       if (cosTheta < -1.0f + 1e-6f) {
           // векторы противоположны
           // выбираем любой перпендикулярный вектор
           rotationAxis = FVector3(1,0,0).Cross(f);
           if (rotationAxis.Length() < 1e-6f) {
               rotationAxis = FVector3(0, 1, 0).Cross(f);
           }
           rotationAxis = rotationAxis.Normalize();
           return Quaternion::AngleAxis(SR_DEG(SR_PI), rotationAxis);
       }

       rotationAxis = f.Cross(t);
       const Unit s = sqrt((1 + cosTheta) * 2);
       const Unit invs = 1.0f / s;

       return Quaternion(
               s * 0.5f,
               rotationAxis.x * invs,
               rotationAxis.y * invs,
               rotationAxis.z * invs
       );*/
    //}

    Quaternion Quaternion::AngleAxis(float_t angle, const Vector3<float_t>& axis) {
        /*const Unit halfAngle = SR_RAD(angle) * static_cast<Unit>(.5f);
        const Unit s = (Unit)SR_SIN(halfAngle);

        auto&& normalized = axis.Normalized();

        Quaternion q;
        q.x = normalized.x * s;
        q.y = normalized.y * s;
        q.z = normalized.z * s;
        q.w = (Unit)SR_COS(halfAngle);

        return q;*/

        const float_t lenSq = axis.SqrMagnitude();
        if (lenSq == 0.f) {
            return Quaternion::Identity();
        }

        const float_t half = SR_RAD(angle) * float_t(0.5);
        const float_t s = SR_SIN(half);

        const Vector3<Unit> n = axis / SR_SQRT(lenSq);

        return Quaternion(n.x * s, n.y * s, n.z * s, SR_COS(half));
    }

    Quaternion Quaternion::AngleAxis(double_t angle, const Vector3<double_t>& axis) {
        const double_t lenSq = axis.SqrMagnitude();
        if (lenSq == 0.0) {
            return Quaternion::Identity();
        }

        const double_t half = SR_RAD(angle) * double_t(0.5);
        const double_t s = SR_SIN(half);

        const Vector3<double_t> n = axis / SR_SQRT(lenSq);

        return Quaternion(
            static_cast<Unit>(n.x * s),
            static_cast<Unit>(n.y * s),
            static_cast<Unit>(n.z * s),
            static_cast<Unit>(SR_COS(half))
        );
    }

    Quaternion Quaternion::FromEuler(const Vector3<Unit>& euler) {
        return euler.Radians().ToQuat();
    }

    Quaternion Quaternion::FromEulerAngles(const Vector3<Unit>& euler) {
        return FromEuler(euler);
    }

    Unit Quaternion::Pitch() const noexcept {
       const Unit value_y = static_cast<Unit>(2) * (y * z + w * x);
       const Unit value_x = w * w - x * x - y * y + z * z;

       /// avoid atan2(0,0) - handle singularity - Matiis
       if (Vector2<Unit>(value_x, value_y) == Vector2<Unit>(Unit(0), Unit(0))) {
           return static_cast<Unit>(static_cast<Unit>(2) * atan2(x, w));
       }

       return static_cast<Unit>(atan2(value_y, value_x));
    }

    Quaternion Quaternion::RotateX(Unit angle) const {
       if (angle == static_cast<Unit>(0)) {
           return *this;
       }

       glm::quat q = glm::rotate(self, static_cast<float_t>(SR_RAD(angle)), glm::vec3(1, 0, 0));
       return Quaternion(q);
    }

    Quaternion Quaternion::RotateY(Unit angle) const {
       if (angle == static_cast<Unit>(0)) {
           return *this;
       }

       glm::quat q = glm::rotate(self, static_cast<float_t>(SR_RAD(angle)), glm::vec3(0, 1, 0));
       return Quaternion(q);
    }

    Quaternion Quaternion::RotateZ(Unit angle) const {
       if (angle == static_cast<Unit>(0)) {
           return *this;
       }

       glm::quat q = glm::rotate(self, static_cast<float_t>(SR_RAD(angle)), glm::vec3(0, 0, 1));
       return Quaternion(q);
    }

    Quaternion Quaternion::Conjugate() const {
         return Quaternion(-x, -y, -z, w);
    }

    SR_NODISCARD Quaternion::T Quaternion::X() const noexcept { return static_cast<T>(self.x); }
    SR_NODISCARD Quaternion::T Quaternion::Y() const noexcept { return static_cast<T>(self.y); }
    SR_NODISCARD Quaternion::T Quaternion::Z() const noexcept { return static_cast<T>(self.z); }
    SR_NODISCARD Quaternion::T Quaternion::W() const noexcept { return static_cast<T>(self.w); }

    Quaternion Quaternion::LookAt(const Vector3<Unit>& direction) {
        static Vector3<Unit> up = Vector3<Unit>(0, 1, 0);
        return Quaternion::LookAt(direction, up);
    }

    Quaternion Quaternion::LookRotation(Vector3<Unit> forward, Vector3<Unit> up) {
        // forward должен быть нормализован
        if (forward.SqrMagnitude() < 1e-12f)
            return Quaternion::Identity();

        forward = forward.Normalize();

        // up не должен быть параллелен forward
        FVector3 right = up.Cross(forward);
        if (right.SqrMagnitude() < 1e-12f) {
            // up слишком параллелен — выбери fallback
            // любое ортогональное направление
            up = std::abs(forward.y) < 0.999f ?
                 FVector3(0,1,0) :
                 FVector3(1,0,0);

            right = up.Cross(forward);
        }

        right = right.Normalize();
        FVector3 newUp = forward.Cross(right);

        return FromBasis(right, newUp, forward);
    }

    Quaternion Quaternion::LookAt(const Vector3<Unit>& direction, const Vector3<Unit>& up) {
        /*const Vector3 z = direction.Normalized();
        const Vector3 x = up.Cross(direction).Normalized();
        const Vector3 y = direction.Cross(x).Normalized();

        FVector4 rows[] = {
            FVector4(x, 0.0f),
            FVector4(y, 0.0f),
            FVector4(z, 0.0f),
            FVector4::UnitW()
        };

        return Quaternion(Matrix4x4(rows));*/

        /*const auto fw = direction.Normalized(); // Front
        const auto ri = axis.Cross(fw).Normalized(); // Left
        const auto up = fw.Cross(ri); // Up

        const auto a0 = ri;
        const auto a1 = up;
        const auto a2 = fw;
        const auto a00 = a0.x;
        const auto a10 = a0.y;
        const auto a20 = a0.z;
        const auto a01 = a1.x;
        const auto a11 = a1.y;
        const auto a21 = a1.z;
        const auto a02 = a2.x;
        const auto a12 = a2.y;
        const auto a22 = a2.z;

        const float trace = a00 + a11 + a22;
        if (trace > 0.0f) {
            const float s = 0.5f / std::sqrt(trace + 1.0f);
            return Quaternion(
                    0.25f / s,
                    (a21 - a12) * s,
                    (a02 - a20) * s,
                    (a10 - a01) * s
            ).Normalized();
        } else {
            if (a00 > a11 && a00 > a22) {
                const float s = 2.0f * std::sqrt(1.0f + a00 - a11 - a22);
                return Quaternion(
                        (a21 - a12) / s,
                        0.25f * s,
                        (a01 + a10) / s,
                        (a02 + a20) / s
                ).Normalized();
            } else if (a11 > a22) {
                const float s = 2.0f * std::sqrt(1.0f + a11 - a00 - a22);
                return Quaternion(
                        (a02 - a20) / s,
                        (a01 + a10) / s,
                        0.25f * s,
                        (a12 + a21) / s
                ).Normalized();
            } else {
                const float s = 2.0f * std::sqrt(1.0f + a22 - a00 - a11);
                return Quaternion(
                        (a10 - a01) / s,
                        (a02 + a20) / s,
                        (a12 + a21) / s,
                        0.25f * s
                ).Normalized();
            }
        }*/

        //////////

        /*auto&& normalDir = direction.Normalize();

        const Unit dot = up.Dot(normalDir);
        const Unit angle = std::acos(dot);

        const Vector3 axis = up.Cross(normalDir).Normalize();

        const Unit halfAngle = angle * 0.5f;
        const Unit s = std::sin(halfAngle);

        return Quaternion(
            axis.x * s,
            axis.y * s,
            axis.z * s,
            std::cos(halfAngle)
        );*/

        /////////////

        Quaternion q = glm::quatLookAt(direction.Normalize().ToGLM(), up.ToGLM());

        /// чиним возможные аффинные преобразования
        //q = q.EulerAngle().Radians().ToQuat();

        /// проверка существования кватерниона
        if (q.IsFinite()) {
            return q;
        }

        return Identity();
    }

    Unit Quaternion::Magnitude() const noexcept {
        return sqrt(SquaredNorm());
    }

    Unit Quaternion::SquaredNorm() const noexcept {
        return w * w + x * x + y * y + z * z;
    }

    Vector4 <Unit> Quaternion::Vector() const noexcept {
        return Vector4<Unit>(x, y, z, w);
    }

    Unit Quaternion::Distance(const Quaternion& q) const {
        auto&& qd = Inverse() * q;
        return 2 * atan2(qd.Vector().Length(), qd.W());
    }

    Quaternion::Quaternion(const Matrix4x4& matrix) {
        Vector3 m0 = matrix[0].XYZ(),
                m1 = matrix[1].XYZ(),
                m2 = matrix[2].XYZ();

        Unit length_sqr = m0[0] * m0[0] + m1[0] * m1[0] + m2[0] * m2[0];

        if (length_sqr != 1.0f && length_sqr != 0.0f) {
            length_sqr = 1.0f / sqrt(length_sqr);
            m0[0] *= length_sqr;
            m1[0] *= length_sqr;
            m2[0] *= length_sqr;
        }

        length_sqr = m0[1] * m0[1] + m1[1] * m1[1] + m2[1] * m2[1];

        if (length_sqr != 1.0f && length_sqr != 0.0f) {
            length_sqr = 1.0f / sqrt(length_sqr);
            m0[1] *= length_sqr;
            m1[1] *= length_sqr;
            m2[1] *= length_sqr;
        }

        length_sqr = m0[2] * m0[2] + m1[2] * m1[2] + m2[2] * m2[2];

        if (length_sqr != 1.0f && length_sqr != 0.0f) {
            length_sqr = 1.0f / sqrt(length_sqr);
            m0[2] *= length_sqr;
            m1[2] *= length_sqr;
            m2[2] *= length_sqr;
        }

        const Unit tr = m0[0] + m1[1] + m2[2];

        if (tr > 0.0f) {
            Unit S = sqrt(tr + 1.0f) * 2.0f; // S=4*qw

            x = (m2[1] - m1[2]) / S;
            y = (m0[2] - m2[0]) / S;
            z = (m1[0] - m0[1]) / S;
            w = 0.25f * S;
        } else if ((m0[0] > m1[1]) && (m0[0] > m2[2])) {
            Unit S = sqrt(1.0f + m0[0] - m1[1] - m2[2]) * 2.0f; // S=4*qx

            x = 0.25f * S;
            y = (m0[1] + m1[0]) / S;
            z = (m0[2] + m2[0]) / S;
            w = (m2[1] - m1[2]) / S;
        } else if (m1[1] > m2[2]) {
            Unit S = sqrt(1.0f + m1[1] - m0[0] - m2[2]) * 2.0f; // S=4*qy

            x = (m0[1] + m1[0]) / S;
            y = 0.25f * S;
            z = (m1[2] + m2[1]) / S;
            w = (m0[2] - m2[0]) / S;
        } else {
            Unit S = sqrt(1.0f + m2[2] - m0[0] - m1[1]) * 2.0f; // S=4*qz

            x = (m0[2] + m2[0]) / S;
            y = (m1[2] + m2[1]) / S;
            z = 0.25f * S;
            w = (m1[0] - m0[1]) / S;
        }
    }

    glm::mat4 Quaternion::ToMat4x4GLM() const noexcept {
        return mat4_cast(self);
    }

    const glm::quat& Quaternion::ToGLM() const noexcept {
        return self;
    }

    Quaternion::Quaternion(const Quaternion &p_q)
        : x(p_q.x)
        , y(p_q.y)
        , z(p_q.z)
        , w(p_q.w)
    { }

    Quaternion::Quaternion() {
        x = y = z = static_cast<T>(0);
        w = static_cast<T>(1);
    }

    Quaternion::Quaternion(const glm::quat &q) {
        self = q;
    }

    Quaternion::Quaternion(Quaternion::T x, Quaternion::T y, Quaternion::T z, Quaternion::T w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    { }

    Quaternion Quaternion::Identity() {
        return Quaternion(0.0, 0.0, 0.0, 1.0);
    }

    Quaternion Quaternion::Conjurate() const {
        return Quaternion(-x, -y, -z, w);
    }

    Quaternion Quaternion::Inverse() const {
        return Quaternion(glm::inverse(self));
    }

    bool Quaternion::IsEquals(const Quaternion &q, Unit tolerance) const noexcept {
#if SR_SIMD_SUPPORT
        // Загружаем компоненты текущего кватерниона и значения в SIMD регистры
        __m128 this_vec = _mm_set_ps(w, z, y, x);
        __m128 value_vec = _mm_set_ps(q.w, q.z, q.y, q.x);

        // Вычисляем разницу между компонентами
        __m128 diff_vec = _mm_sub_ps(this_vec, value_vec);

        // Загружаем допуск в SIMD регистр
        __m128 tolerance_vec = _mm_set1_ps(tolerance);

        // Вычисляем абсолютное значение разницы
        __m128 abs_diff_vec = _mm_andnot_ps(_mm_set1_ps(-0.0f), diff_vec); // получаем abs
        abs_diff_vec = _mm_cmpge_ps(abs_diff_vec, tolerance_vec); // сравниваем на больше или равно по модулю

        // Проверяем, все ли компоненты проходят проверку на равенство
        int mask = _mm_movemask_ps(abs_diff_vec); // применяем маску
        return mask == 0; // если все 0, то результаты совпадают
#else
        if (!SR_EQUALS_T(x, q.x, tolerance)) {
            return false;
        }

        if (!SR_EQUALS_T(y, q.y, tolerance)) {
            return false;
        }

        if (!SR_EQUALS_T(z, q.z, tolerance)) {
            return false;
        }

        if (!SR_EQUALS_T(w, q.w, tolerance)) {
            return false;
        }

        return true;
#endif
    }

    Quaternion Quaternion::Slerp(const Quaternion &q, Unit t) const {
#if SR_SIMD_SUPPORT
        // Load q1 and q2 into SIMD registers
        __m128 q1_vec = _mm_set_ps(w, z, y, x); // Загрузка в обратном порядке для корректного выравнивания
        __m128 q2_vec = _mm_set_ps(q.w, q.z, q.y, q.x); // Загрузка в обратном порядке для корректного выравнивания

        // Compute the dot product
        __m128 dot_vec = _mm_dp_ps(q1_vec, q2_vec, 0xFF); // Вычисление dot product с использованием SIMD

        // Распаковка результатов
        float dot_result;
        _mm_store_ss(&dot_result, dot_vec);

        // Если dot product отрицателен, инвертируем один кватернион
        __m128 q2_copy = q2_vec;
        if (dot_result < 0.0f) {
            q2_copy = _mm_mul_ps(q2_vec, _mm_set1_ps(-1.0f));
            dot_result = -dot_result;
        }

        const float DOT_THRESHOLD = 0.9995f;
        if (dot_result > DOT_THRESHOLD) {
            // Если кватернионы слишком близки, выполняем линейную интерполяцию и нормализацию результата
            __m128 result_vec = _mm_add_ps(q1_vec, _mm_mul_ps(_mm_sub_ps(q2_copy, q1_vec), _mm_set1_ps(t)));
            float result_array[4];
            _mm_store_ps(result_array, result_vec);
            Quaternion result(result_array[0], result_array[1], result_array[2], result_array[3]);
            return result.Normalized();
        }

        // Вычисляем угол и sin(theta)
        float theta_0 = std::acos(dot_result); // Угол между входными векторами
        float theta = theta_0 * t;      // Угол между q1 и результатом
        float sin_theta = std::sin(theta);        // Вычисляем значение только один раз
        float sin_theta_0 = std::sin(theta_0);    // Вычисляем значение только один раз

        float s0 = std::cos(theta) - dot_result * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
        float s1 = sin_theta / sin_theta_0;

        // Интерполируем
        __m128 s0_vec = _mm_set1_ps(s0);
        __m128 s1_vec = _mm_set1_ps(s1);
        __m128 interp_vec = _mm_add_ps(_mm_mul_ps(q1_vec, s0_vec), _mm_mul_ps(q2_copy, s1_vec));

        // Сохраняем результат
        float result_array[4];
        _mm_store_ps(result_array, interp_vec);
        return Quaternion(result_array[0], result_array[1], result_array[2], result_array[3]);




        /*// Загрузка в SIMD
        __m128 q1_vec = _mm_set_ps(w, z, y, x); // [x,y,z,w]
        __m128 q2_vec = _mm_set_ps(q.w, q.z, q.y, q.x);

        // dot product через SSE2
        __m128 mul = _mm_mul_ps(q1_vec, q2_vec);
        __m128 shuf1 = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2,1,0,3));
        __m128 sum1 = _mm_add_ps(mul, shuf1);
        __m128 shuf2 = _mm_movehl_ps(sum1, sum1);
        __m128 sum2 = _mm_add_ss(sum1, shuf2);
        float dot_result = _mm_cvtss_f32(sum2);

        // Флип, если dot < 0
        __m128 q2_copy = q2_vec;
        if (dot_result < 0.0f) {
            q2_copy = _mm_mul_ps(q2_vec, _mm_set1_ps(-1.0f));
            dot_result = -dot_result;
        }

        // Clamp dot
        dot_result = std::clamp(dot_result, -1.0f, 1.0f);

        const float DOT_THRESHOLD = 0.9995f;
        if (dot_result > DOT_THRESHOLD) {
            // Линейная интерполяция
            __m128 diff = _mm_sub_ps(q2_copy, q1_vec);
            __m128 t_vec = _mm_set1_ps(t);
            __m128 interp = _mm_add_ps(q1_vec, _mm_mul_ps(diff, t_vec));

            float res[4];
            _mm_store_ps(res, interp);
            return Quaternion(res[0], res[1], res[2], res[3]).Normalized();
        }

        // SLERP
        float theta_0 = std::acos(dot_result);
        float theta = theta_0 * t;
        float sin_theta = std::sin(theta);
        float sin_theta_0 = std::sin(theta_0);

        float s0 = std::cos(theta) - dot_result * sin_theta / sin_theta_0;
        float s1 = sin_theta / sin_theta_0;

        __m128 s0_vec = _mm_set1_ps(s0);
        __m128 s1_vec = _mm_set1_ps(s1);
        __m128 interp_vec = _mm_add_ps(_mm_mul_ps(q1_vec, s0_vec), _mm_mul_ps(q2_copy, s1_vec));

        float res[4];
        _mm_store_ps(res, interp_vec);
        return Quaternion(res[0], res[1], res[2], res[3]).Normalized();*/
#else
        return glm::slerp(self, q.self, static_cast<float_t>(t));
#endif
    }

    Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, Unit t) {
        return a.Slerp(b, t);
    }

    Quaternion Quaternion::Normalized() const {
        return Normalize();
    }

    Quaternion Quaternion::Normalize() const {
        //return Quaternion(glm::normalize(self));
        return NormalizeSafe();
    }

    Quaternion Quaternion::NormalizeSafe() const {
        float dot = Dot(*this);
        if (dot > SR_KINDA_SMALL_NUMBER_EPSILON) {
            float rSqrt = 1.0f / SR_SQRT(dot);
            return Quaternion(x * rSqrt, y * rSqrt, z * rSqrt, w * rSqrt);
        }

        return Identity();

        /// const Unit mag = Magnitude();
        /// if (mag > static_cast<Unit>(0)) {
        ///     return (*this) / mag;
        /// }
        /// return Identity();
    }

    Unit Quaternion::Roll() const noexcept {
        return static_cast<Unit>(atan2(static_cast<Unit>(2) * (x * y + w * z), w * w + x * x - y * y - z * z));
    }

    Unit Quaternion::Yaw() const noexcept {
        return asin(SR_CLAMP(static_cast<Unit>(-2) * (x * z - w * y), static_cast<Unit>(-1), static_cast<Unit>(1)));
    }

    std::string Quaternion::ToString() const {
        return "(" + std::to_string(self.x) + ", " + std::to_string(self.y) + ", " + std::to_string(self.z) + ", " + std::to_string(self.w) + ")";
    }

    bool Quaternion::operator!=(const Quaternion &q) const noexcept {
        return !(*this == q);
    }

    bool Quaternion::operator==(const Quaternion &q) const noexcept {
        return
            SR_EQUALS(x, q.x) &&
            SR_EQUALS(y, q.y) &&
            SR_EQUALS(z, q.z) &&
            SR_EQUALS(w, q.w);
    }

    Unit Quaternion::Angle(const Quaternion &q) const {
        return Distance(q);
    }

    bool Quaternion::IsFinite() const noexcept {
        /// если будет inf или nan, то вернет false
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z) && std::isfinite(w);
    }

    bool Quaternion::IsSane() const noexcept {
        const float unitTolerance = 1e-2f;
        return IsFinite() && SR_ABS(Magnitude() - 1) < unitTolerance;
    }

    bool Quaternion::IsIdentity() const noexcept {
    #if defined(SR_SIMD_SUPPORT) && 0
        const __m128 v = _mm_loadu_ps(&x);
        const __m128 id = _mm_set_ps(1.f, 0.f, 0.f, 0.f); // w z y x
        const __m128 cmp = _mm_cmpeq_ps(v, id);
        return _mm_movemask_ps(cmp) == 0b1111;
    #else
        return x == 0.f && y == 0.f && z == 0.f && w == 1.f;
    #endif
    }

    void Quaternion::operator+=(const Quaternion &p_q) {
        self += p_q.self;
    }

    void Quaternion::operator-=(const Quaternion &p_q) {
        self -= p_q.self;
    }

    void Quaternion::operator*=(const Quaternion &p_q) {
        self *= p_q.self;
    }

    void Quaternion::operator*=(const double &s){
        self *= s;
    }

    void Quaternion::operator/=(const double &s) {
        self *= 1.0 / s;
    }

    Quaternion Quaternion::operator+(const Quaternion &q2) const {
        const Quaternion &q1 = *this;
        return Quaternion(q1.self + q2.self);
    }

    Quaternion Quaternion::operator-(const Quaternion &q2) const {
        const Quaternion &q1 = *this;
        return Quaternion(q1.self - q2.self);
    }

    Quaternion Quaternion::operator-() const {
        const Quaternion &q2 = *this;
        return Quaternion(-q2.self);
    }

    Quaternion Quaternion::operator*(const double &s) const {
        glm::quat q = self;
        q *= s;
        return Quaternion(q);
    }

    Quaternion Quaternion::operator/(const double &s) const {
        glm::quat q = self;
        q *= 1.0 / s;
        return Quaternion(q);
    }

    Quaternion Quaternion::operator*(const Quaternion &rhs) const {
        return Quaternion(self * rhs.self);
    }

    Unit Quaternion::Dot(const Quaternion& q) const noexcept {
        float_t dot = x * q.x + y * q.y + z * q.z + w * q.w;
        return dot;
    }

    void Quaternion::ToAxisAngle(Vector3<Unit>& axis, float_t& angle) const {
        // нормализуем кватернион на всякий случай
        SR_MATH_NS::Quaternion nq = NormalizeSafe();

        // угол
        angle = SR_DEG(2.0f * acosf(nq.w));

        // s = sqrt(1 - w*w), нужна для нормализации оси
        float s = sqrtf(1.0f - nq.w * nq.w);

        if (s < 1e-6f) {
            // угол очень маленький, ось может быть любой нормализованной
            axis = SR_MATH_NS::FVector3(1, 0, 0);
        } else {
            axis = SR_MATH_NS::FVector3(nq.x / s, nq.y / s, nq.z / s);
        }
    }

    Quaternion Quaternion::FromBasis(const Vector3<Unit> &right, const Vector3<Unit> &up, const Vector3<Unit> &forward) {
        // Матрица в формате column-major:
        // | right.x   up.x   forward.x |
        // | right.y   up.y   forward.y |
        // | right.z   up.z   forward.z |

        Unit m00 = right.x; Unit m01 = up.x; Unit m02 = forward.x;
        Unit m10 = right.y; Unit m11 = up.y; Unit m12 = forward.y;
        Unit m20 = right.z; Unit m21 = up.z; Unit m22 = forward.z;

        Unit trace = m00 + m11 + m22;

        Quaternion q;

        if (trace > 0.0f) {
            Unit s = SR_SQRT(trace + 1.0f) * 2.0f;
            q.w = 0.25f * s;
            q.x = (m21 - m12) / s;
            q.y = (m02 - m20) / s;
            q.z = (m10 - m01) / s;
        }
        else if (m00 > m11 && m00 > m22) {
            Unit s = SR_SQRT(1.0f + m00 - m11 - m22) * 2.0f;
            q.w = (m21 - m12) / s;
            q.x = 0.25f * s;
            q.y = (m01 + m10) / s;
            q.z = (m02 + m20) / s;
        }
        else if (m11 > m22) {
            Unit s = SR_SQRT(1.0f + m11 - m00 - m22) * 2.0f;
            q.w = (m02 - m20) / s;
            q.x = (m01 + m10) / s;
            q.y = 0.25f * s;
            q.z = (m12 + m21) / s;
        }
        else {
            Unit s = SR_SQRT(1.0f + m22 - m00 - m11) * 2.0f;
            q.w = (m10 - m01) / s;
            q.x = (m02 + m20) / s;
            q.y = (m12 + m21) / s;
            q.z = 0.25f * s;
        }

        return q;
    }

    Unit Quaternion::Angle(const Quaternion &a, const Quaternion &b) {
        const float_t num = SR_MIN(SR_ABS(Quaternion::Dot(a, b)), 1.f);
        return Quaternion::IsEqualUsingDot(num) ? 0.0f : (float) ((double)SR_ACOS(num) * 2.0 * 57.2957801818848);
    }

    Unit Quaternion::Dot(const Quaternion& a, const Quaternion& b) {
        return a.Dot(b);
    }

    bool Quaternion::IsEqualUsingDot(Unit dot) {
        return static_cast<double_t>(dot) > 0.999998986721039;
    }

    Quaternion Quaternion::Inverse(const Quaternion& q) {
        return q.Inverse();
    }

    Quaternion Quaternion::Nlerp(const Quaternion &q, Unit t) const {
    #ifdef SR_SIMD_SUPPORT
        __m128 q1 = _mm_setr_ps(x, y, z, w);
        __m128 q2 = _mm_setr_ps(q.x, q.y, q.z, q.w);

        // dot(q1, q2) broadcast
        __m128 dot = _mm_dp_ps(q1, q2, 0xFF);

        // if dot < 0 -> flip
        __m128 signMask = _mm_cmplt_ps(dot, _mm_setzero_ps());
        __m128 flip = _mm_and_ps(signMask, _mm_set1_ps(-0.0f));
        q2 = _mm_xor_ps(q2, flip);

        // lerp
        __m128 tVec = _mm_set1_ps(t);
        __m128 interp = _mm_add_ps(q1, _mm_mul_ps(_mm_sub_ps(q2, q1), tVec));

        // normalize
        __m128 len2 = _mm_dp_ps(interp, interp, 0xFF);
        __m128 invLen = _mm_rsqrt_ps(len2);
        __m128 norm = _mm_mul_ps(interp, invLen);

        alignas(16) float out[4];
        _mm_store_ps(out, norm);
        return Quaternion(out[0], out[1], out[2], out[3]);
    #else
        Quaternion q2 = q;

        // Вычисляем dot product
        Unit dot = Dot(q2);

        // Если dot отрицателен, инвертируем один кватернион
        if (dot < 0.0f) {
            q2 = Quaternion(-q2.x, -q2.y, -q2.z, -q2.w);
        }

        // Линейная интерполяция
        Quaternion result = (*this) + (q2 - (*this)) * t;

        // Нормализация результата
        return result.NormalizeSafe();
    #endif
    }

    Quaternion Quaternion::Nlerp(const Quaternion& a, const Quaternion& b, Unit t) {
        return a.Nlerp(b, t);
    }
}