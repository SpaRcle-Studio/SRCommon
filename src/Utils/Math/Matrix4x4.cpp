//
// Created by Monika on 16.01.2023.
//

#include <Utils/Math/Matrix4x4.h>
#include <Utils/Math/Matrix4x4Utils.h>
#include <Utils/Math/Matrix3x3.h>
#include <Utils/Debug.h>
#include <Utils/Profile/TracyContext.h>

#define SR_MATRIX_COMPOSE_COMPARE_ENABLED

namespace SR_MATH_NS {
    Matrix4x4::Matrix4x4(const FVector3& translate, const Quaternion& rotation, const FVector3& scale) noexcept
        : Matrix4x4()
    {
    #ifdef SR_MATRIX_COMPOSE_COMPARE_ENABLED
        if (!translate.IsZero()) {
            TranslateMat4x4(*this, translate);
        }

        if (!rotation.IsIdentity()) {
            RotateMat4x4(*this, rotation);
        }

        if (!scale.IsOne()) {
            ScaleMat4x4(*this, scale);
        }
    #else
        TranslateMat4x4(*this, translate);
        RotateMat4x4(*this, rotation);
        ScaleMat4x4(*this, scale);
    #endif
    }

    Matrix4x4::Matrix4x4(const FVector3& translate, const Quaternion& rotation, const FVector3& scale, const FVector3& skew) noexcept
        : Matrix4x4()
    {
        TranslateMat4x4(*this, translate);

        ScaleMat4x4(*this, skew);
        RotateMat4x4(*this, rotation);
        ScaleMat4x4(*this, scale);
    }

    Matrix4x4::Matrix4x4(float_t m00v, float_t m01v, float_t m02v, float_t m03v,
                         float_t m10v, float_t m11v, float_t m12v, float_t m13v,
                         float_t m20v, float_t m21v, float_t m22v, float_t m23v,
                         float_t m30v, float_t m31v, float_t m32v, float_t m33v) noexcept
    {
        m00 = m00v; m01 = m01v; m02 = m02v; m03 = m03v;
        m10 = m10v; m11 = m11v; m12 = m12v; m13 = m13v;
        m20 = m20v; m21 = m21v; m22 = m22v; m23 = m23v;
        m30 = m30v; m31 = m31v; m32 = m32v; m33 = m33v;
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& mat) const {
        Matrix4x4 result(0.f);
        MultiplyMat4x4(result, *this, mat);
        return result;
    }

    Matrix4x4::Matrix4x4(const FVector3& translate, const Quaternion& rotation) noexcept
        : Matrix4x4()
    {
        TranslateMat4x4(*this, translate);
        RotateMat4x4(*this, rotation);
    }

    Matrix4x4::Matrix4x4(const FVector3& translate) noexcept
        : Matrix4x4()
    {
        TranslateMat4x4(*this, translate);
    }

    Matrix4x4::Matrix4x4(const Quaternion& rotation) noexcept
        : Matrix4x4()
    {
        RotateMat4x4(*this, rotation);
    }

    Matrix4x4::Matrix4x4(const FVector3& translate, const FVector3& scale) noexcept
        : Matrix4x4()
    {
        TranslateMat4x4(*this, translate);
        ScaleMat4x4(*this, scale);
    }

    Matrix4x4::Matrix4x4(const Quaternion& rotation, const FVector3& scale) noexcept
        : Matrix4x4()
    {
        RotateMat4x4(*this, rotation);
        ScaleMat4x4(*this, scale);
    }

    Matrix4x4 Matrix4x4::CreateViewMat(Unit pitch, Unit yaw, Unit roll)  {
        return RotationAxis(FVector3(1.f, 0.f, 0.f), pitch)
            * RotationAxis(FVector3(0.f, 1.f, 0.f), yaw)
            * RotationAxis(FVector3(0.f, 0.f, 1.f), roll);
    }

    Matrix4x4::Matrix4x4() noexcept
        : Matrix4x4(1.f)
    { }

    Matrix4x4::Matrix4x4(FVector4* rows) noexcept {
        value[0] = rows[0];
        value[1] = rows[1];
        value[2] = rows[2];
        value[3] = rows[3];
    }

    Matrix4x4::Matrix4x4(const Unit &scalar) noexcept {
        for (int32_t column = 0; column < 4; ++column) {
            for (int32_t row = 0; row < 4; ++row) {
                m[column][row] = column == row ? static_cast<float_t>(scalar) : 0.f;
            }
        }
    }

    Matrix4x4 Matrix4x4::Identity() {
        return Matrix4x4(1); /// NOLINT
    }

    Matrix4x4 Matrix4x4::RotationYawPitchRoll(const FVector3& angles) {
        return Matrix4x4(0.f, angles.ToQuat(), 1.f); /// NOLINT
    }

    Matrix4x4 Matrix4x4::Perspective(float_t FOV, float_t aspect, float_t nearValue, float_t farValue) {
        const float_t tangent = tanf(FOV * 0.5f);
        Matrix4x4 result(0.f);
        result.m00 = 1.f / (aspect * tangent);
        result.m11 = 1.f / tangent;
        result.m22 = farValue / (nearValue - farValue);
        result.m23 = -1.f;
        result.m32 = -(farValue * nearValue) / (farValue - nearValue);
        return result;
    }

    Matrix4x4 Matrix4x4::PerspectiveReverseInfinite(float_t FOV, float_t aspect, float_t nearValue) {
        const float_t tangent = tanf(FOV * 0.5f);
        Matrix4x4 result(0.f);
        result.m00 = 1.f / (aspect * tangent);
        result.m11 = 1.f / tangent;
        result.m22 = 0.f;
        result.m23 = -1.f;
        result.m32 = nearValue;
        return result;
    }

    Matrix4x4 Matrix4x4::ReverseDepth() {
        Matrix4x4 result = Identity();
        result.m22 = -1.f;
        result.m32 = 1.f;
        return result;
    }

    Matrix4x4 Matrix4x4::FromEulers(const FVector3& eulers) {
        return Matrix4x4::RotationYawPitchRoll(eulers.Radians());
    }

    Matrix4x4 Matrix4x4::Ortho(Unit left, Unit right, Unit bottom, Unit top, Unit zNear, Unit zFar) {
        Matrix4x4 Result(1);
        Result[0][0] = static_cast<Unit>(2) / (right - left);
        Result[1][1] = static_cast<Unit>(2) / (top - bottom);
        Result[3][0] = - (right + left) / (right - left);
        Result[3][1] = - (top + bottom) / (top - bottom);

        Result[2][2] = - static_cast<Unit>(1) / (zFar - zNear);
        Result[3][2] = - zNear / (zFar - zNear);

        return Result;
    }

    Matrix4x4 Matrix4x4::CreateTRS(const SR_MATH_NS::FVector3& translation, const SR_MATH_NS::Quaternion& rotation, const SR_MATH_NS::FVector3& scale) {
        return Matrix4x4(translation, rotation, scale);
    }

    Matrix4x4 Matrix4x4::LookAt(const SR_MATH_NS::FVector3& eye, const SR_MATH_NS::FVector3& center, const SR_MATH_NS::FVector3& up) {
        const FVector3 forward = (center - eye).Normalize();
        const FVector3 right = forward.Cross(up).Normalize();
        const FVector3 vertical = right.Cross(forward);
        return Matrix4x4(
            right.x, vertical.x, -forward.x, 0.f,
            right.y, vertical.y, -forward.y, 0.f,
            right.z, vertical.z, -forward.z, 0.f,
            -right.Dot(eye), -vertical.Dot(eye), forward.Dot(eye), 1.f
        );
    }

    Matrix4x4 Matrix4x4::FromQuaternion(const Quaternion& quaternion) {
        return Matrix4x4(quaternion);
    }

    Matrix4x4 Matrix4x4::RotationAxis(const SR_MATH_NS::FVector4& axis, Unit angle) {
        const Unit length2 = axis.SqrMagnitude();
        if (length2 < FLT_EPSILON) {
            return Matrix4x4::Identity();
        }

        const SR_MATH_NS::FVector4 n = axis * (1.f / sqrtf(length2));
        const Unit s = sinf(angle);
        const Unit c = cosf(angle);
        const Unit k = 1.f - c;

        const Unit xx = n.x * n.x * k + c;
        const Unit yy = n.y * n.y * k + c;
        const Unit zz = n.z * n.z * k + c;
        const Unit xy = n.x * n.y * k;
        const Unit yz = n.y * n.z * k;
        const Unit zx = n.z * n.x * k;
        const Unit xs = n.x * s;
        const Unit ys = n.y * s;
        const Unit zs = n.z * s;

        Matrix4x4 m;

        m.m[0][0] = xx;
        m.m[0][1] = xy + zs;
        m.m[0][2] = zx - ys;
        m.m[0][3] = 0.f;
        m.m[1][0] = xy - zs;
        m.m[1][1] = yy;
        m.m[1][2] = yz + xs;
        m.m[1][3] = 0.f;
        m.m[2][0] = zx + ys;
        m.m[2][1] = yz - xs;
        m.m[2][2] = zz;
        m.m[2][3] = 0.f;
        m.m[3][0] = 0.f;
        m.m[3][1] = 0.f;
        m.m[3][2] = 0.f;
        m.m[3][3] = 1.f;

        return m;
    }

    Matrix4x4 Matrix4x4::RotationAxis(const SR_MATH_NS::FVector3& axis, Unit angle) {
        const Unit length2 = axis.SqrMagnitude();
        if (length2 < FLT_EPSILON) {
            return Matrix4x4::Identity();
        }

        const SR_MATH_NS::FVector3 n = axis * (1.f / sqrtf(length2));
        const Unit s = sinf(angle);
        const Unit c = cosf(angle);
        const Unit k = 1.f - c;

        const Unit xx = n.x * n.x * k + c;
        const Unit yy = n.y * n.y * k + c;
        const Unit zz = n.z * n.z * k + c;
        const Unit xy = n.x * n.y * k;
        const Unit yz = n.y * n.z * k;
        const Unit zx = n.z * n.x * k;
        const Unit xs = n.x * s;
        const Unit ys = n.y * s;
        const Unit zs = n.z * s;

        Matrix4x4 m;

        m.m[0][0] = xx;
        m.m[0][1] = xy + zs;
        m.m[0][2] = zx - ys;
        m.m[0][3] = 0.f;
        m.m[1][0] = xy - zs;
        m.m[1][1] = yy;
        m.m[1][2] = yz + xs;
        m.m[1][3] = 0.f;
        m.m[2][0] = zx + ys;
        m.m[2][1] = yz - xs;
        m.m[2][2] = zz;
        m.m[2][3] = 0.f;
        m.m[3][0] = 0.f;
        m.m[3][1] = 0.f;
        m.m[3][2] = 0.f;
        m.m[3][3] = 1.f;

        return m;
    }

    FVector4 CalcTranslationPlanNormal(const Matrix4x4& model, const SR_MATH_NS::FVector3& cameraEye, const SR_MATH_NS::FVector3& cameraDir, Axis axis) {
        SR_MATH_NS::FVector4 movePlanNormal[] = {
                model.v.right, /// x
                model.v.up,    /// y
                model.v.dir,   /// z
                model.v.right, /// yz
                model.v.up,    /// zx
                model.v.dir,   /// xy
                SR_MATH_NS::FVector4(-cameraDir, 0.f) /// screen (xyz)
        };

        auto&& cameraToModelNormalized = SR_MATH_NS::FVector4((model.v.position.XYZ() - cameraEye).Normalize(), 0.f);

        for (uint8_t i = 0; i < 3; ++i) {
            auto&& orthogonalVector = movePlanNormal[i].Cross(cameraToModelNormalized);
            movePlanNormal[i] = (movePlanNormal[i].Cross(orthogonalVector)).Normalize();
        }

        switch (axis) {
            case Axis::XYZ: return movePlanNormal[6];
            case Axis::YZ: return movePlanNormal[3];
            case Axis::XZ: return movePlanNormal[4];
            case Axis::XY: return movePlanNormal[5];
            case Axis::X: return movePlanNormal[0];
            case Axis::Y: return movePlanNormal[1];
            case Axis::Z: return movePlanNormal[2];
            default:
                break;
        }

        SRHalt("Unknown axis!");

        return SR_MATH_NS::FVector4();
    }

    FVector4 CalcRotationPlanNormal(const Matrix4x4& model, const SR_MATH_NS::FVector3& cameraDir, Axis axis) {
        SR_MATH_NS::FVector4 rotatePlanNormal[] = {
                model.v.right, /// x
                model.v.up,    /// y
                model.v.dir,   /// z
                SR_MATH_NS::FVector4(-cameraDir, 0.f) /// screen (xyz)
        };

        switch (axis) {
            case Axis::X: return rotatePlanNormal[0];
            case Axis::Y: return rotatePlanNormal[1];
            case Axis::Z: return rotatePlanNormal[2];
            case Axis::XYZ: return rotatePlanNormal[3];
            default:
                break;
        }

        SRHalt("Unknown axis!");

        return SR_MATH_NS::FVector4();
    }

    FVector4 CalcRotationPlanNormal(const SR_MATH_NS::FVector3& cameraDir, Axis axis) {
        SR_MATH_NS::FVector4 rotatePlanNormal[] = {
                SR_MATH_NS::FVector4(SR_MATH_NS::FVector3::Right(), 0.f), /// x
                SR_MATH_NS::FVector4(SR_MATH_NS::FVector3::Up(), 0.f), /// y
                SR_MATH_NS::FVector4(SR_MATH_NS::FVector3::Forward(), 0.f), /// z
                SR_MATH_NS::FVector4(-cameraDir, 0.f) /// screen (xyz)
        };

        switch (axis) {
            case Axis::X: return rotatePlanNormal[0];
            case Axis::Y: return rotatePlanNormal[1];
            case Axis::Z: return rotatePlanNormal[2];
            case Axis::XYZ: return rotatePlanNormal[3];
            default:
                break;
        }

        SRHalt("Unknown axis!");

        return SR_MATH_NS::FVector4();
    }

    Matrix4x4 Matrix4x4::FromScale(const FVector3& scale) {
        Matrix4x4 result;
        ScaleMat4x4(result, scale);
        return result;
    }

    Matrix4x4 Matrix4x4::FromTranslate(const FVector3& translation) {
        return Matrix4x4(translation);
    }

    Matrix4x4 Matrix4x4::Inverse() const {
    #if SR_SIMD_SUPPORT
        const __m128 columns[4] = {
            _mm_loadu_ps(mm),
            _mm_loadu_ps(mm + 4),
            _mm_loadu_ps(mm + 8),
            _mm_loadu_ps(mm + 12)
        };
        __m128 cofactors[4];
        {
            // Cyclic permutations of the three remaining columns preserve the minor's sign.
            const __m128 first = columns[1];
            const __m128 second = columns[2];
            const __m128 third = columns[3];

            // Each lane omits its own row: (1,2,3), (0,2,3), (0,1,3), (0,1,2).
            const __m128 first0 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 first1 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 first2 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(2, 3, 3, 3));
            const __m128 second0 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 second1 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 second2 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(2, 3, 3, 3));
            const __m128 third0 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 third1 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 third2 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(2, 3, 3, 3));

            __m128 minors = _mm_mul_ps(first0, _mm_sub_ps(_mm_mul_ps(second1, third2), _mm_mul_ps(second2, third1)));
            minors = _mm_sub_ps(minors, _mm_mul_ps(second0, _mm_sub_ps(_mm_mul_ps(first1, third2), _mm_mul_ps(first2, third1))));
            minors = _mm_add_ps(minors, _mm_mul_ps(third0, _mm_sub_ps(_mm_mul_ps(first1, second2), _mm_mul_ps(first2, second1))));
            const __m128 signs = _mm_set_ps(-1.f, 1.f, -1.f, 1.f);
            cofactors[0] = _mm_mul_ps(minors, signs);
        }

        {
            // Cyclic permutations of the three remaining columns preserve the minor's sign.
            const __m128 first = columns[2];
            const __m128 second = columns[3];
            const __m128 third = columns[0];

            // Each lane omits its own row: (1,2,3), (0,2,3), (0,1,3), (0,1,2).
            const __m128 first0 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 first1 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 first2 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(2, 3, 3, 3));
            const __m128 second0 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 second1 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 second2 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(2, 3, 3, 3));
            const __m128 third0 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 third1 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 third2 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(2, 3, 3, 3));

            __m128 minors = _mm_mul_ps(first0, _mm_sub_ps(_mm_mul_ps(second1, third2), _mm_mul_ps(second2, third1)));
            minors = _mm_sub_ps(minors, _mm_mul_ps(second0, _mm_sub_ps(_mm_mul_ps(first1, third2), _mm_mul_ps(first2, third1))));
            minors = _mm_add_ps(minors, _mm_mul_ps(third0, _mm_sub_ps(_mm_mul_ps(first1, second2), _mm_mul_ps(first2, second1))));
            const __m128 signs = _mm_set_ps(1.f, -1.f, 1.f, -1.f);
            cofactors[1] = _mm_mul_ps(minors, signs);
        }

        {
            // Cyclic permutations of the three remaining columns preserve the minor's sign.
            const __m128 first = columns[3];
            const __m128 second = columns[0];
            const __m128 third = columns[1];

            // Each lane omits its own row: (1,2,3), (0,2,3), (0,1,3), (0,1,2).
            const __m128 first0 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 first1 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 first2 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(2, 3, 3, 3));
            const __m128 second0 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 second1 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 second2 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(2, 3, 3, 3));
            const __m128 third0 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 third1 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 third2 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(2, 3, 3, 3));

            __m128 minors = _mm_mul_ps(first0, _mm_sub_ps(_mm_mul_ps(second1, third2), _mm_mul_ps(second2, third1)));
            minors = _mm_sub_ps(minors, _mm_mul_ps(second0, _mm_sub_ps(_mm_mul_ps(first1, third2), _mm_mul_ps(first2, third1))));
            minors = _mm_add_ps(minors, _mm_mul_ps(third0, _mm_sub_ps(_mm_mul_ps(first1, second2), _mm_mul_ps(first2, second1))));
            const __m128 signs = _mm_set_ps(-1.f, 1.f, -1.f, 1.f);
            cofactors[2] = _mm_mul_ps(minors, signs);
        }

        {
            // Cyclic permutations of the three remaining columns preserve the minor's sign.
            const __m128 first = columns[0];
            const __m128 second = columns[1];
            const __m128 third = columns[2];

            // Each lane omits its own row: (1,2,3), (0,2,3), (0,1,3), (0,1,2).
            const __m128 first0 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 first1 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 first2 = _mm_shuffle_ps(first, first, _MM_SHUFFLE(2, 3, 3, 3));
            const __m128 second0 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 second1 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 second2 = _mm_shuffle_ps(second, second, _MM_SHUFFLE(2, 3, 3, 3));
            const __m128 third0 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(0, 0, 0, 1));
            const __m128 third1 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(1, 1, 2, 2));
            const __m128 third2 = _mm_shuffle_ps(third, third, _MM_SHUFFLE(2, 3, 3, 3));

            __m128 minors = _mm_mul_ps(first0, _mm_sub_ps(_mm_mul_ps(second1, third2), _mm_mul_ps(second2, third1)));
            minors = _mm_sub_ps(minors, _mm_mul_ps(second0, _mm_sub_ps(_mm_mul_ps(first1, third2), _mm_mul_ps(first2, third1))));
            minors = _mm_add_ps(minors, _mm_mul_ps(third0, _mm_sub_ps(_mm_mul_ps(first1, second2), _mm_mul_ps(first2, second1))));
            const __m128 signs = _mm_set_ps(1.f, -1.f, 1.f, -1.f);
            cofactors[3] = _mm_mul_ps(minors, signs);
        }

        _MM_TRANSPOSE4_PS(cofactors[0], cofactors[1], cofactors[2], cofactors[3]);
        const __m128 products = _mm_mul_ps(_mm_set_ps(m30, m20, m10, m00), cofactors[0]);
        __m128 determinant = _mm_add_ss(products, _mm_shuffle_ps(products, products, _MM_SHUFFLE(1, 1, 1, 1)));
        determinant = _mm_add_ss(determinant, _mm_shuffle_ps(products, products, _MM_SHUFFLE(2, 2, 2, 2)));
        determinant = _mm_add_ss(determinant, _mm_shuffle_ps(products, products, _MM_SHUFFLE(3, 3, 3, 3)));
        const __m128 divisor = _mm_shuffle_ps(determinant, determinant, _MM_SHUFFLE(0, 0, 0, 0));

        Matrix4x4 result(0.f);
        _mm_storeu_ps(result.mm, _mm_div_ps(cofactors[0], divisor));
        _mm_storeu_ps(result.mm + 4, _mm_div_ps(cofactors[1], divisor));
        _mm_storeu_ps(result.mm + 8, _mm_div_ps(cofactors[2], divisor));
        _mm_storeu_ps(result.mm + 12, _mm_div_ps(cofactors[3], divisor));
        return result;
    #else
        Matrix4x4 cofactors(0.f);
        for (int32_t column = 0; column < 4; ++column) {
            for (int32_t row = 0; row < 4; ++row) {
                float_t minor[9];
                int32_t index = 0;
                for (int32_t sourceRow = 0; sourceRow < 4; ++sourceRow) {
                    if (sourceRow == row) {
                        continue;
                    }
                    for (int32_t sourceColumn = 0; sourceColumn < 4; ++sourceColumn) {
                        if (sourceColumn != column) {
                            minor[index++] = m[sourceColumn][sourceRow];
                        }
                    }
                }

                const float_t determinant = minor[0] * (minor[4] * minor[8] - minor[5] * minor[7])
                    - minor[1] * (minor[3] * minor[8] - minor[5] * minor[6])
                    + minor[2] * (minor[3] * minor[7] - minor[4] * minor[6]);
                cofactors.m[column][row] = ((column + row) % 2) ? -determinant : determinant;
            }
        }

        const float_t determinant = m[0][0] * cofactors.m[0][0] + m[1][0] * cofactors.m[1][0]
            + m[2][0] * cofactors.m[2][0] + m[3][0] * cofactors.m[3][0];
        Matrix4x4 result(0.f);
        for (int32_t column = 0; column < 4; ++column) {
            for (int32_t row = 0; row < 4; ++row) {
                result.m[column][row] = cofactors.m[row][column] / determinant;
            }
        }
        return result;
    #endif
    }

    SR_NODISCARD Matrix4x4 Matrix4x4::RotateAxis(const FVector3& axis, const double& angle) const {
        return *this * RotationAxis(axis, SR_RAD(static_cast<float_t>(angle)));
    }

    SR_NODISCARD Matrix4x4 Matrix4x4::Rotate(const FVector3& angle) const {
        return Rotate(angle.ToQuat());
    }

    SR_NODISCARD Matrix4x4 Matrix4x4::Rotate(const SR_MATH_NS::Quaternion& q) const {
        return Matrix4x4(*this * q.ToMat4x4());
    }

    /// потенциально что-то не рабочее
    SR_NODISCARD Matrix4x4 Matrix4x4::OrthogonalNormalize() const {
        Matrix4x4 copy = *this;
        copy.v.right = copy.v.right.Normalize();
        copy.v.up = copy.v.up.Normalize();
        copy.v.dir = copy.v.dir.Normalize();
        return copy;
    }

    SR_NODISCARD Matrix4x4 Matrix4x4::Translate(const FVector3& vec3) const {
        Matrix4x4 result = *this;
        TranslateMat4x4(result, vec3);
        return result;
    }

    SR_NODISCARD FVector4 Matrix4x4::GetAxis(Axis axis) const {
        switch (axis) {
            case Axis::X: return value[0];
            case Axis::Y: return value[1];
            case Axis::Z: return value[2];
            default:
                SRHalt("Wrong axis!");
                return SR_MATH_NS::FVector4();
        }
    }

    SR_NODISCARD FVector3 Matrix4x4::GetTranslate() const {
        return value[3].XYZ();
    }

    SR_NODISCARD FVector3 Matrix4x4::GetTranslation() const {
        return value[3].XYZ();
    }

    SR_NODISCARD FVector3 Matrix4x4::GetScale() const {
        SR_MATH_NS::FVector3 scale;
        scale.x = value[0].XYZ().Length();
        scale.y = value[1].XYZ().Length();
        scale.z = value[2].XYZ().Length();
        return scale;
    }

    SR_NODISCARD bool Matrix4x4::IsFinite() const {
        for (int32_t i = 0; i < 16; ++i) {
            if (!std::isfinite(mm[i])) {
                return false;
            }
        }
        return true;
    }

    bool Matrix4x4::Decompose(FVector3& translation, Quaternion& quaternion, FVector3& scale) const {
        translation = GetTranslation();
        scale = GetScale();
        if (scale.x <= SR_FLT_EPSILON || scale.y <= SR_FLT_EPSILON || scale.z <= SR_FLT_EPSILON || !IsFinite()) {
            return false;
        }

        const FVector3 right = value[0].XYZ() / scale.x;
        const FVector3 up = value[1].XYZ() / scale.y;
        const FVector3 forward = value[2].XYZ() / scale.z;
        quaternion = Matrix3x3(
            FVector3(right.x, up.x, forward.x),
            FVector3(right.y, up.y, forward.y),
            FVector3(right.z, up.z, forward.z)
        ).ToQuaternion();
        return true;
    }

    bool Matrix4x4::Decompose(FVector3& translation, Quaternion& quaternion) const {
        FVector3 scale;
        return Decompose(translation, quaternion, scale);
    }

    bool Matrix4x4::Decompose(FVector3& translation, FVector3& eulers, FVector3& scale) const {
        Quaternion rotation;
        if (!Decompose(translation, rotation, scale)) {
            return false;
        }
        rotation = rotation.Normalize();
        eulers = FVector3(rotation.Pitch(), rotation.Yaw(), rotation.Roll()).Degrees();
        return true;
    }

    bool Matrix4x4::Decompose(FVector3& translation, FVector3& eulers, FVector3& scale, FVector3& /* skew */) const {
        return Decompose(translation, eulers, scale);
    }

    bool Matrix4x4::Decompose(FVector3& translation, Quaternion& rotation, FVector3& scale, FVector3& skew) const {
        if (!IsFinite() || fabsf(m33) <= SR_FLT_EPSILON) {
            return false;
        }

        const float_t inverseW = 1.f / m33;
        FVector3 right = value[0].XYZ() * inverseW;
        FVector3 up = value[1].XYZ() * inverseW;
        FVector3 forward = value[2].XYZ() * inverseW;
        const float_t determinant = right.Dot(up.Cross(forward));
        if (fabsf(determinant) <= SR_FLT_EPSILON) {
            return false;
        }

        scale.x = right.Length();
        right /= scale.x;
        skew.z = right.Dot(up);
        up -= right * skew.z;
        scale.y = up.Length();
        up /= scale.y;
        skew.z /= scale.y;

        skew.y = right.Dot(forward);
        forward -= right * skew.y;
        skew.x = up.Dot(forward);
        forward -= up * skew.x;
        scale.z = forward.Length();
        forward /= scale.z;
        skew.y /= scale.z;
        skew.x /= scale.z;

        if (right.Dot(up.Cross(forward)) < 0.f) {
            scale = -scale;
            right = -right;
            up = -up;
            forward = -forward;
        }

        translation = value[3].XYZ() * inverseW;
        rotation = Matrix3x3(
            FVector3(right.x, up.x, forward.x),
            FVector3(right.y, up.y, forward.y),
            FVector3(right.z, up.z, forward.z)
        ).ToQuaternion();
        return true;
    }

    SR_NODISCARD FVector4 Matrix4x4::TransformPoint(const FVector3& point) const {
        FVector4 out;
    #if SR_SIMD_SUPPORT
        __m128 transformed = _mm_mul_ps(_mm_loadu_ps(mm), _mm_set1_ps(point.x));
        transformed = _mm_add_ps(transformed, _mm_mul_ps(_mm_loadu_ps(mm + 4), _mm_set1_ps(point.y)));
        transformed = _mm_add_ps(transformed, _mm_mul_ps(_mm_loadu_ps(mm + 8), _mm_set1_ps(point.z)));
        transformed = _mm_add_ps(transformed, _mm_loadu_ps(mm + 12));
        _mm_storeu_ps(out.coord, transformed);
    #else
        out.x = point.x * m00 + point.y * m10 + point.z * m20 + m30;
        out.y = point.x * m01 + point.y * m11 + point.z * m21 + m31;
        out.z = point.x * m02 + point.y * m12 + point.z * m22 + m32;
        out.w = point.x * m03 + point.y * m13 + point.z * m23 + m33;
    #endif
        return out;
    }

    SR_NODISCARD FVector4 Matrix4x4::TransformVector(const FVector3& point) const {
        FVector4 out;
    #if SR_SIMD_SUPPORT
        __m128 transformed = _mm_mul_ps(_mm_loadu_ps(mm), _mm_set1_ps(point.x));
        transformed = _mm_add_ps(transformed, _mm_mul_ps(_mm_loadu_ps(mm + 4), _mm_set1_ps(point.y)));
        transformed = _mm_add_ps(transformed, _mm_mul_ps(_mm_loadu_ps(mm + 8), _mm_set1_ps(point.z)));
        _mm_storeu_ps(out.coord, transformed);
    #else
        out.x = point.x * m00 + point.y * m10 + point.z * m20;
        out.y = point.x * m01 + point.y * m11 + point.z * m21;
        out.z = point.x * m02 + point.y * m12 + point.z * m22;
        out.w = point.x * m03 + point.y * m13 + point.z * m23;
    #endif
        return out;
    }

    SR_NODISCARD FVector4 Matrix4x4::TransformVector(const FVector4& point) const {
        FVector4 out;
    #if SR_SIMD_SUPPORT
        const __m128 input = _mm_loadu_ps(point.coord);
        __m128 transformed = _mm_mul_ps(_mm_loadu_ps(mm), _mm_shuffle_ps(input, input, _MM_SHUFFLE(0, 0, 0, 0)));
        transformed = _mm_add_ps(transformed, _mm_mul_ps(_mm_loadu_ps(mm + 4), _mm_shuffle_ps(input, input, _MM_SHUFFLE(1, 1, 1, 1))));
        transformed = _mm_add_ps(transformed, _mm_mul_ps(_mm_loadu_ps(mm + 8), _mm_shuffle_ps(input, input, _MM_SHUFFLE(2, 2, 2, 2))));
        transformed = _mm_add_ps(transformed, _mm_mul_ps(_mm_loadu_ps(mm + 12), _mm_shuffle_ps(input, input, _MM_SHUFFLE(3, 3, 3, 3))));
        _mm_storeu_ps(out.coord, transformed);
    #else
        out.x = point.x * m00 + point.y * m10 + point.z * m20 + point.w * m30;
        out.y = point.x * m01 + point.y * m11 + point.z * m21 + point.w * m31;
        out.z = point.x * m02 + point.y * m12 + point.z * m22 + point.w * m32;
        out.w = point.x * m03 + point.y * m13 + point.z * m23 + point.w * m33;
    #endif
        return out;
    }

    SR_NODISCARD SR_MATH_NS::Unit Matrix4x4::GetSegmentLengthClipSpace(
            const SR_MATH_NS::FVector3& start,
            const SR_MATH_NS::FVector3& end,
            SR_MATH_NS::Unit displayRatio
    ) const {
        auto&& startOfSegment = TransformPoint(start);
        if (fabsf(startOfSegment.w) > SR_FLT_EPSILON) {
            startOfSegment *= 1.f / startOfSegment.w;
        }

        auto&& endOfSegment = TransformPoint(end);
        if (fabsf(endOfSegment.w) > SR_FLT_EPSILON) {
            endOfSegment *= 1.f / endOfSegment.w;
        }

        auto&& clipSpaceAxis = (endOfSegment - startOfSegment).XY();

        if (displayRatio < 1.0) {
            clipSpaceAxis.x *= displayRatio;
        }
        else {
            clipSpaceAxis.y /= displayRatio;
        }

        const SR_MATH_NS::Unit segmentLengthInClipSpace = sqrtf(clipSpaceAxis.x * clipSpaceAxis.x + clipSpaceAxis.y * clipSpaceAxis.y);
        return segmentLengthInClipSpace;
    }

    SR_NODISCARD Quaternion Matrix4x4::GetQuat() const {
        const FVector3 right = value[0].XYZ().Normalize();
        const FVector3 up = (value[1].XYZ() - right * value[1].XYZ().Dot(right)).Normalize();
        const FVector3 forward = right.Cross(up);
        return Matrix3x3(
            FVector3(right.x, up.x, forward.x),
            FVector3(right.y, up.y, forward.y),
            FVector3(right.z, up.z, forward.z)
        ).ToQuaternion();
    }

    SR_NODISCARD FVector3 Matrix4x4::GetEulers() const {
        return GetQuat().EulerAngle();
    }

    SR_NODISCARD FVector3 Matrix4x4::Right() const {
        return value[0].XYZ().Normalize();
    }

    SR_NODISCARD FVector3 Matrix4x4::Up() const {
        return value[1].XYZ().Normalize();
    }

    SR_NODISCARD FVector3 Matrix4x4::Forward() const {
        return value[2].XYZ().Normalize();
    }

    const SR_MATH_NS::FVector4& Matrix4x4::operator[](int32_t row) const {
        return value[row];
    }

    SR_MATH_NS::FVector4& Matrix4x4::operator[](int32_t row) {
        return value[row];
    }

    void Matrix4x4::operator*=(const Matrix4x4& right) {
        *this = *this * right;
    }

    Matrix4x4 Matrix4x4::operator+(const Matrix4x4& mat) const {
        Matrix4x4 result(0.f);
    #if SR_SIMD_SUPPORT
        _mm_storeu_ps(result.mm, _mm_add_ps(_mm_loadu_ps(mm), _mm_loadu_ps(mat.mm)));
        _mm_storeu_ps(result.mm + 4, _mm_add_ps(_mm_loadu_ps(mm + 4), _mm_loadu_ps(mat.mm + 4)));
        _mm_storeu_ps(result.mm + 8, _mm_add_ps(_mm_loadu_ps(mm + 8), _mm_loadu_ps(mat.mm + 8)));
        _mm_storeu_ps(result.mm + 12, _mm_add_ps(_mm_loadu_ps(mm + 12), _mm_loadu_ps(mat.mm + 12)));
    #else
        for (int32_t i = 0; i < 16; ++i) {
            result.mm[i] = mm[i] + mat.mm[i];
        }
    #endif
        return result;
    }

    void Matrix4x4::operator+=(const Matrix4x4& right) {
        *this = *this + right;
    }

    Matrix4x4 Matrix4x4::operator*(const Unit& scalar) const {
        Matrix4x4 result(0.f);
    #if SR_SIMD_SUPPORT
        const __m128 multiplier = _mm_set1_ps(static_cast<float_t>(scalar));
        _mm_storeu_ps(result.mm, _mm_mul_ps(_mm_loadu_ps(mm), multiplier));
        _mm_storeu_ps(result.mm + 4, _mm_mul_ps(_mm_loadu_ps(mm + 4), multiplier));
        _mm_storeu_ps(result.mm + 8, _mm_mul_ps(_mm_loadu_ps(mm + 8), multiplier));
        _mm_storeu_ps(result.mm + 12, _mm_mul_ps(_mm_loadu_ps(mm + 12), multiplier));
    #else
        for (int32_t i = 0; i < 16; ++i) {
            result.mm[i] = mm[i] * static_cast<float_t>(scalar);
        }
    #endif
        return result;
    }

    void Matrix4x4::operator*=(const Unit& scalar) {
        *this = *this * static_cast<float>(scalar);
    }

    Matrix4x4 Matrix4x4::operator/(Matrix4x4 mat) {
        return *this * mat.Inverse();
    }

    Matrix4x4 Matrix4x4::operator+(Matrix4x4 mat) {
        return static_cast<const Matrix4x4&>(*this) + mat;
    }

    Matrix4x4 Matrix4x4::operator-(Matrix4x4 mat) {
        Matrix4x4 result(0.f);
    #if SR_SIMD_SUPPORT
        _mm_storeu_ps(result.mm, _mm_sub_ps(_mm_loadu_ps(mm), _mm_loadu_ps(mat.mm)));
        _mm_storeu_ps(result.mm + 4, _mm_sub_ps(_mm_loadu_ps(mm + 4), _mm_loadu_ps(mat.mm + 4)));
        _mm_storeu_ps(result.mm + 8, _mm_sub_ps(_mm_loadu_ps(mm + 8), _mm_loadu_ps(mat.mm + 8)));
        _mm_storeu_ps(result.mm + 12, _mm_sub_ps(_mm_loadu_ps(mm + 12), _mm_loadu_ps(mat.mm + 12)));
    #else
        for (int32_t i = 0; i < 16; ++i) {
            result.mm[i] = mm[i] - mat.mm[i];
        }
    #endif
        return result;
    }

    void Matrix4x4::Multiply(Matrix4x4& result, const Matrix4x4& left, const Matrix4x4& right) {
        MultiplyMat4x4(result, left, right);
    }

    Matrix4x4 Matrix4x4::CreateOrthographicOffCenter(Unit left, Unit right, Unit bottom, Unit top, Unit zNear, Unit zFar) {
        Matrix4x4 result = Matrix4x4::Identity();

        result.mm[0]  = 2.0f / (right - left);
        result.mm[5]  = 2.0f / (top - bottom);
        result.mm[10] = -2.0f / (zFar - zNear);

        result.mm[12] = -(right + left) / (right - left);
        result.mm[13] = -(top + bottom) / (top - bottom);
        result.mm[14] = -(zFar + zNear) / (zFar - zNear);
        result.mm[15] = 1.0f;

        return result;
    }

    AABB Matrix4x4::TransformAABB(const AABB& localBox) const {
        SR_TRACY_ZONE;

        /// 8 углов локального бокса
        SR_MATH_NS::FVector3 corners[8] = {
            SR_MATH_NS::FVector3(localBox.min.x, localBox.min.y, localBox.min.z),
            SR_MATH_NS::FVector3(localBox.max.x, localBox.min.y, localBox.min.z),
            SR_MATH_NS::FVector3(localBox.min.x, localBox.max.y, localBox.min.z),
            SR_MATH_NS::FVector3(localBox.max.x, localBox.max.y, localBox.min.z),
            SR_MATH_NS::FVector3(localBox.min.x, localBox.min.y, localBox.max.z),
            SR_MATH_NS::FVector3(localBox.max.x, localBox.min.y, localBox.max.z),
            SR_MATH_NS::FVector3(localBox.min.x, localBox.max.y, localBox.max.z),
            SR_MATH_NS::FVector3(localBox.max.x, localBox.max.y, localBox.max.z)
        };

    #if SR_SIMD_SUPPORT
        const __m128 column0 = _mm_loadu_ps(mm);
        const __m128 column1 = _mm_loadu_ps(mm + 4);
        const __m128 column2 = _mm_loadu_ps(mm + 8);
        const __m128 translation = _mm_loadu_ps(mm + 12);
        __m128 minimum = _mm_set1_ps(std::numeric_limits<float_t>::max());
        __m128 maximum = _mm_set1_ps(std::numeric_limits<float_t>::lowest());
        __m128 world0 = _mm_mul_ps(column0, _mm_set1_ps(corners[0].x));
        world0 = _mm_add_ps(world0, _mm_mul_ps(column1, _mm_set1_ps(corners[0].y)));
        world0 = _mm_add_ps(world0, _mm_mul_ps(column2, _mm_set1_ps(corners[0].z)));
        world0 = _mm_add_ps(world0, translation);
        minimum = _mm_min_ps(world0, minimum);
        maximum = _mm_max_ps(world0, maximum);

        __m128 world1 = _mm_mul_ps(column0, _mm_set1_ps(corners[1].x));
        world1 = _mm_add_ps(world1, _mm_mul_ps(column1, _mm_set1_ps(corners[1].y)));
        world1 = _mm_add_ps(world1, _mm_mul_ps(column2, _mm_set1_ps(corners[1].z)));
        world1 = _mm_add_ps(world1, translation);
        minimum = _mm_min_ps(world1, minimum);
        maximum = _mm_max_ps(world1, maximum);

        __m128 world2 = _mm_mul_ps(column0, _mm_set1_ps(corners[2].x));
        world2 = _mm_add_ps(world2, _mm_mul_ps(column1, _mm_set1_ps(corners[2].y)));
        world2 = _mm_add_ps(world2, _mm_mul_ps(column2, _mm_set1_ps(corners[2].z)));
        world2 = _mm_add_ps(world2, translation);
        minimum = _mm_min_ps(world2, minimum);
        maximum = _mm_max_ps(world2, maximum);

        __m128 world3 = _mm_mul_ps(column0, _mm_set1_ps(corners[3].x));
        world3 = _mm_add_ps(world3, _mm_mul_ps(column1, _mm_set1_ps(corners[3].y)));
        world3 = _mm_add_ps(world3, _mm_mul_ps(column2, _mm_set1_ps(corners[3].z)));
        world3 = _mm_add_ps(world3, translation);
        minimum = _mm_min_ps(world3, minimum);
        maximum = _mm_max_ps(world3, maximum);

        __m128 world4 = _mm_mul_ps(column0, _mm_set1_ps(corners[4].x));
        world4 = _mm_add_ps(world4, _mm_mul_ps(column1, _mm_set1_ps(corners[4].y)));
        world4 = _mm_add_ps(world4, _mm_mul_ps(column2, _mm_set1_ps(corners[4].z)));
        world4 = _mm_add_ps(world4, translation);
        minimum = _mm_min_ps(world4, minimum);
        maximum = _mm_max_ps(world4, maximum);

        __m128 world5 = _mm_mul_ps(column0, _mm_set1_ps(corners[5].x));
        world5 = _mm_add_ps(world5, _mm_mul_ps(column1, _mm_set1_ps(corners[5].y)));
        world5 = _mm_add_ps(world5, _mm_mul_ps(column2, _mm_set1_ps(corners[5].z)));
        world5 = _mm_add_ps(world5, translation);
        minimum = _mm_min_ps(world5, minimum);
        maximum = _mm_max_ps(world5, maximum);

        __m128 world6 = _mm_mul_ps(column0, _mm_set1_ps(corners[6].x));
        world6 = _mm_add_ps(world6, _mm_mul_ps(column1, _mm_set1_ps(corners[6].y)));
        world6 = _mm_add_ps(world6, _mm_mul_ps(column2, _mm_set1_ps(corners[6].z)));
        world6 = _mm_add_ps(world6, translation);
        minimum = _mm_min_ps(world6, minimum);
        maximum = _mm_max_ps(world6, maximum);

        __m128 world7 = _mm_mul_ps(column0, _mm_set1_ps(corners[7].x));
        world7 = _mm_add_ps(world7, _mm_mul_ps(column1, _mm_set1_ps(corners[7].y)));
        world7 = _mm_add_ps(world7, _mm_mul_ps(column2, _mm_set1_ps(corners[7].z)));
        world7 = _mm_add_ps(world7, translation);
        minimum = _mm_min_ps(world7, minimum);
        maximum = _mm_max_ps(world7, maximum);

        FVector4 newMin;
        FVector4 newMax;
        _mm_storeu_ps(newMin.coord, minimum);
        _mm_storeu_ps(newMax.coord, maximum);
        return { newMin.XYZ(), newMax.XYZ() };
    #else
        SR_MATH_NS::FVector3 newMin(
            std::numeric_limits<float_t>::max(),
            std::numeric_limits<float_t>::max(),
            std::numeric_limits<float_t>::max()
        );

        SR_MATH_NS::FVector3 newMax(
            std::numeric_limits<float_t>::lowest(),
            std::numeric_limits<float_t>::lowest(),
            std::numeric_limits<float_t>::lowest()
        );

        for (auto&& c : corners) {
            SR_MATH_NS::FVector4 world = TransformVector(SR_MATH_NS::FVector4(c, 1.f));

            newMin.x = std::min(newMin.x, world.x);
            newMin.y = std::min(newMin.y, world.y);
            newMin.z = std::min(newMin.z, world.z);

            newMax.x = std::max(newMax.x, world.x);
            newMax.y = std::max(newMax.y, world.y);
            newMax.z = std::max(newMax.z, world.z);
        }

        return { newMin, newMax };
    #endif
    }

    Matrix4x4 Matrix4x4::Orthonormalize() const {
        SR_TRACY_ZONE;

        Matrix4x4 result = *this;

        FVector3 xAxis = result.Right();
        FVector3 yAxis = result.Up();
        FVector3 zAxis;

        // Ортогонализация с помощью метода Грама-Шмидта
        xAxis = xAxis.Normalize();
        yAxis = (yAxis - xAxis * xAxis.Dot(yAxis)).Normalize();
        zAxis = xAxis.Cross(yAxis).Normalize();

        result.v.right = FVector4(xAxis, 0.0f);
        result.v.up    = FVector4(yAxis, 0.0f);
        result.v.dir   = FVector4(zAxis, 0.0f);

        return result;
    }

    Matrix4x4 Matrix4x4::FromSkew(const FVector3& skew) {
        Matrix4x4 result = Matrix4x4::Identity();

        result.m[0][1] = skew.x; // Skew XY
        result.m[0][2] = skew.y; // Skew XZ
        result.m[1][2] = skew.z; // Skew YZ

        return result;
    }

    Matrix4x4::Matrix4x4(const Matrix3x3& mat3x3) noexcept {
        /// Matrix3x3 хранится по строкам, а Matrix4x4 - по столбцам,
        /// поэтому при копировании нужно транспонировать
        m00 = mat3x3[0][0]; m01 = mat3x3[1][0]; m02 = mat3x3[2][0]; m03 = 0.f;
        m10 = mat3x3[0][1]; m11 = mat3x3[1][1]; m12 = mat3x3[2][1]; m13 = 0.f;
        m20 = mat3x3[0][2]; m21 = mat3x3[1][2]; m22 = mat3x3[2][2]; m23 = 0.f;
        m30 = 0.f; m31 = 0.f; m32 = 0.f; m33 = 1.f;
    }
}
