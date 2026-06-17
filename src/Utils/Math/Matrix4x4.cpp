//
// Created by Monika on 16.01.2023.
//

#include <Utils/Math/Matrix4x4.h>
#include <Utils/Math/Matrix4x4Utils.h>
#include <Utils/Debug.h>
#include <Utils/Profile/TracyContext.h>

#include <glm/gtx/matrix_interpolation.hpp>

#define SR_MATRIX_COMPOSE_COMPARE_ENABLED

namespace SR_MATH_NS {
    Matrix4x4::Matrix4x4(const FVector3& translate, const Quaternion& rotation, const FVector3& scale) noexcept
        : Matrix4x4()
    {
    #ifdef SR_MATRIX_COMPOSE_COMPARE_ENABLED
        if (!translate.IsZero()) {
            GLMTranslateMat4x4(self, translate);
        }

        if (!rotation.IsIdentity()) {
            GLMRotateMat4x4_Fast(self, rotation.ToGLM());
        }

        if (!scale.IsOne()) {
            GLMScaleMat4x4(self, scale);
        }
    #else
        GLMTranslateMat4x4(self, translate);
        GLMRotateMat4x4_Fast(self, rotation.ToGLM());
        GLMScaleMat4x4(self, scale);
    #endif
    }

    Matrix4x4::Matrix4x4(const FVector3& translate, const Quaternion& rotation, const FVector3& scale, const FVector3& skew) noexcept
        : Matrix4x4()
    {
        GLMTranslateMat4x4(self, translate);

        GLMScaleMat4x4(self, skew);
        GLMRotateMat4x4_Fast(self, rotation.ToGLM());
        GLMScaleMat4x4(self, scale);
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
        return Matrix4x4(GLMMultiplyMat4x4(self, mat.self));
    }

    Matrix4x4::Matrix4x4(const FVector3& translate, const Quaternion& rotation) noexcept
        : Matrix4x4()
    {
        GLMTranslateMat4x4(self, translate);
        GLMRotateMat4x4_Fast(self, rotation.ToGLM());
    }

    Matrix4x4::Matrix4x4(const FVector3& translate) noexcept
        : Matrix4x4()
    {
        GLMTranslateMat4x4(self, translate);
    }

    Matrix4x4::Matrix4x4(const Quaternion& rotation) noexcept
        : Matrix4x4()
    {
        GLMRotateMat4x4_Fast(self, rotation.ToGLM());
    }

    Matrix4x4::Matrix4x4(const FVector3& translate, const FVector3& scale) noexcept
        : Matrix4x4()
    {
        GLMTranslateMat4x4(self, translate);
        GLMScaleMat4x4(self, scale);
    }

    Matrix4x4::Matrix4x4(const Quaternion& rotation, const FVector3& scale) noexcept
        : Matrix4x4()
    {
        GLMRotateMat4x4_Fast(self, rotation.ToGLM());
        GLMScaleMat4x4(self, scale);
    }

    Matrix4x4 Matrix4x4::CreateViewMat(Unit pitch, Unit yaw, Unit roll)  {
        auto matrix = glm::rotate(glm::mat4(1), (float_t)pitch, { 1, 0, 0 });
        matrix = glm::rotate(matrix,            (float_t)yaw,   { 0, 1, 0 });
        matrix = glm::rotate(matrix,            (float_t)roll,  { 0, 0, 1 });
        return Matrix4x4(glm::translate(matrix, { 0, 0, 0 }));
    }

    Matrix4x4::Matrix4x4() noexcept
        : self(GLM_IDENTITY_MAT4X4)
    { }

    Matrix4x4::Matrix4x4(glm::mat4&& mat) noexcept
        : self(mat)
    { }

    Matrix4x4::Matrix4x4(const glm::mat4& mat) noexcept
        : self(mat)
    { }

    Matrix4x4::Matrix4x4(FVector4* rows) noexcept {
        value[0] = rows[0];
        value[1] = rows[1];
        value[2] = rows[2];
        value[3] = rows[3];
    }

    Matrix4x4::Matrix4x4(const Unit &scalar) noexcept
        : self(glm::mat4(static_cast<float_t>(scalar)))
    { }

    Matrix4x4 Matrix4x4::Identity() {
        return Matrix4x4(1); /// NOLINT
    }

    Matrix4x4 Matrix4x4::RotationYawPitchRoll(const FVector3& angles) {
        return Matrix4x4(0.f, angles.ToQuat(), 1.f); /// NOLINT
    }

    Matrix4x4 Matrix4x4::Perspective(float_t FOV, float_t aspect, float_t nearValue, float_t farValue) {
        return Matrix4x4(glm::perspective(FOV, aspect, nearValue, farValue)); /// NOLINT
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

#if GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE
        Result[2][2] = - static_cast<Unit>(1) / (zFar - zNear);
        Result[3][2] = - zNear / (zFar - zNear);
#else
        Result[2][2] = - static_cast<T>(2) / (zFar - zNear);
        Result[3][2] = - (zFar + zNear) / (zFar - zNear);
#endif

        return Result;
    }

    Matrix4x4 Matrix4x4::CreateTRS(const SR_MATH_NS::FVector3& translation, const SR_MATH_NS::Quaternion& rotation, const SR_MATH_NS::FVector3& scale) {
        return Matrix4x4(translation, rotation, scale);
    }

    Matrix4x4 Matrix4x4::LookAt(const SR_MATH_NS::FVector3& eye, const SR_MATH_NS::FVector3& center, const SR_MATH_NS::FVector3& up) {
        const glm::vec3 eyeGLM = glm::vec3(eye.x, eye.y, eye.z);
        const glm::vec3 centerGLM = glm::vec3(center.x, center.y, center.z);
        const glm::vec3 upGLM = glm::vec3(up.x, up.y, up.z);
        return Matrix4x4(glm::lookAtRH(eyeGLM, centerGLM, upGLM));
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
        return Matrix4x4(glm::scale(glm::mat4x4(1), glm::vec3(scale.x, scale.y, scale.z)));
    }

    Matrix4x4 Matrix4x4::FromTranslate(const FVector3& translation) {
        return Matrix4x4(translation);
    }

    SR_NODISCARD Matrix4x4 Matrix4x4::Inverse() const {
        return Matrix4x4(glm::inverse(self));
    }

    SR_NODISCARD Matrix4x4 Matrix4x4::RotateAxis(const FVector3& axis, const double& angle) const {
        return Matrix4x4(glm::rotate(self, glm::radians((float)angle), glm::vec3(axis.x, axis.y, axis.z)));
    }

    SR_NODISCARD Matrix4x4 Matrix4x4::Rotate(const FVector3& angle) const {
        return Matrix4x4(self * mat4_cast(angle.ToQuat().ToGLM()));
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

    SR_NODISCARD const glm::mat4& Matrix4x4::ToGLM() const {
        return self;
    }

    SR_NODISCARD Matrix4x4 Matrix4x4::Translate(const FVector3& vec3) const {
        return Matrix4x4(glm::translate(self, glm::vec3(vec3.x, vec3.y, vec3.z)));
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

    SR_NODISCARD FVector3 Matrix4x4::GetScale() const {
        SR_MATH_NS::FVector3 scale;
        scale.x = value[0].XYZ().Length();
        scale.y = value[1].XYZ().Length();
        scale.z = value[2].XYZ().Length();
        return scale;
    }

    SR_NODISCARD bool Matrix4x4::IsFinite() const {
        return value[0].IsFinite() && value[1].IsFinite() && value[2].IsFinite() && value[3].IsFinite();
    }

    bool Matrix4x4::Decompose(FVector3& translation, Quaternion& quaternion, FVector3& scale) const {
        translation = value[3].XYZ();

        scale[0] = glm::length(glm::vec3(self[0]));
        scale[1] = glm::length(glm::vec3(self[1]));
        scale[2] = glm::length(glm::vec3(self[2]));

        const glm::mat3 rotMtx(
                glm::vec3(self[0]) / static_cast<float>(scale[0]),
                glm::vec3(self[1]) / static_cast<float>(scale[1]),
                glm::vec3(self[2]) / static_cast<float>(scale[2]));

        quaternion = glm::quat_cast(rotMtx);

        return true;
    }

    bool Matrix4x4::Decompose(FVector3& translation, Quaternion& quaternion) const {
        translation = value[3].XYZ();

        auto&& scaleX = glm::length(glm::vec3(self[0]));
        auto&& scaleY = glm::length(glm::vec3(self[1]));
        auto&& scaleZ = glm::length(glm::vec3(self[2]));

        const glm::mat3 rotMtx(
                glm::vec3(self[0]) / static_cast<float_t>(scaleX),
                glm::vec3(self[1]) / static_cast<float_t>(scaleY),
                glm::vec3(self[2]) / static_cast<float_t>(scaleZ)
        );

        quaternion = glm::quat_cast(rotMtx);

        return true;
    }

    bool Matrix4x4::Decompose(FVector3& translation, FVector3& eulers, FVector3& scale) const {
        translation = value[3].XYZ();

        scale[0] = glm::length(glm::vec3(self[0]));
        scale[1] = glm::length(glm::vec3(self[1]));
        scale[2] = glm::length(glm::vec3(self[2]));

        const glm::mat3 rotMtx(
                glm::vec3(self[0]) / static_cast<float>(scale[0]),
                glm::vec3(self[1]) / static_cast<float>(scale[1]),
                glm::vec3(self[2]) / static_cast<float>(scale[2]));

        glm::vec3 glmEulers = glm::eulerAngles(glm::normalize(glm::quat_cast(rotMtx)));
        eulers = FVector3(glmEulers.x, glmEulers.y, glmEulers.z);
        eulers = eulers.Degrees();

        return true;
    }

    bool Matrix4x4::Decompose(FVector3& translation, FVector3& eulers, FVector3& scale, FVector3& /* skew */) const {
        translation = value[3].XYZ();

        scale[0] = glm::length(glm::vec3(self[0]));
        scale[1] = glm::length(glm::vec3(self[1]));
        scale[2] = glm::length(glm::vec3(self[2]));

        const glm::mat3 rotMtx(
                glm::vec3(self[0]) / static_cast<float>(scale[0]),
                glm::vec3(self[1]) / static_cast<float>(scale[1]),
                glm::vec3(self[2]) / static_cast<float>(scale[2]));

        glm::vec3 glmEulers = glm::eulerAngles(glm::normalize(glm::quat_cast(rotMtx)));
        eulers = FVector3(glmEulers.x, glmEulers.y, glmEulers.z);
        eulers = eulers.Degrees();

        return true;
    }

    bool Matrix4x4::Decompose(FVector3& translation, Quaternion& rotation, FVector3& scale, FVector3& skew) const {
        glm::vec3 _scale;
        glm::quat _rotation;
        glm::vec3 _translation;

        glm::vec3 _skew;
        glm::vec4 _perspective;

        if (glm::decompose(self, _scale, _rotation, _translation, _skew, _perspective)) {
            translation = FVector3(_translation.x, _translation.y, _translation.z);
            rotation = _rotation;
            scale = FVector3(_scale.x, _scale.y, _scale.z);
            skew = FVector3(_skew.x, _skew.y, _skew.z);
            return true;
        }

        return false;
    }

    SR_NODISCARD FVector4 Matrix4x4::TransformPoint(const FVector3& point) const {
        FVector4 out;
        out.x = point.x * m00 + point.y * m10 + point.z * m20 + m30;
        out.y = point.x * m01 + point.y * m11 + point.z * m21 + m31;
        out.z = point.x * m02 + point.y * m12 + point.z * m22 + m32;
        out.w = point.x * m03 + point.y * m13 + point.z * m23 + m33;
        return out;
    }

    SR_NODISCARD FVector4 Matrix4x4::TransformVector(const FVector3& point) const {
        FVector4 out;
        out.x = point.x * m00 + point.y * m10 + point.z * m20;
        out.y = point.x * m01 + point.y * m11 + point.z * m21;
        out.z = point.x * m02 + point.y * m12 + point.z * m22;
        out.w = point.x * m03 + point.y * m13 + point.z * m23;
        return out;
    }

    SR_NODISCARD FVector4 Matrix4x4::TransformVector(const FVector4& point) const {
        FVector4 out;
        out.x = point.x * m00 + point.y * m10 + point.z * m20 + point.w * m30;
        out.y = point.x * m01 + point.y * m11 + point.z * m21 + point.w * m31;
        out.z = point.x * m02 + point.y * m12 + point.z * m22 + point.w * m32;
        out.w = point.x * m03 + point.y * m13 + point.z * m23 + point.w * m33;
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
        glm::mat3 glmMat(self);

        // Gram–Schmidt ортонормализация
        glm::vec3 x = glm::normalize(glmMat[0]);
        glm::vec3 y = glm::normalize(glmMat[1] - x * glm::dot(glmMat[1], x));
        glm::vec3 z = glm::cross(x, y); // автоматическая ортогональность

        return glm::quat_cast(glm::mat3(x, y, z));
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
        return Matrix4x4(self + mat.self);
    }
    void Matrix4x4::operator+=(const Matrix4x4& right) {
        *this = *this + right;
    }

    Matrix4x4 Matrix4x4::operator*(const Unit& scalar) const {
        return Matrix4x4(self * static_cast<float>(scalar));
    }
    void Matrix4x4::operator*=(const Unit& scalar) {
        *this = *this * static_cast<float>(scalar);
    }

    Matrix4x4 Matrix4x4::operator/(Matrix4x4 mat) {
        return Matrix4x4(this->self / mat.self);
    }

    Matrix4x4 Matrix4x4::operator+(Matrix4x4 mat) {
        return Matrix4x4(this->self + mat.self);
    }
    Matrix4x4 Matrix4x4::operator-(Matrix4x4 mat) {
        return Matrix4x4(this->self - mat.self);
    }

    void Matrix4x4::Multiply(Matrix4x4& result, const Matrix4x4& left, const Matrix4x4& right) {
        GLMMultiplyMat4x4(result.self, left.self, right.self);
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

        // 8 углов локального бокса
        std::array<SR_MATH_NS::FVector3, 8> corners = {
                SR_MATH_NS::FVector3(localBox.min.x, localBox.min.y, localBox.min.z),
                SR_MATH_NS::FVector3(localBox.max.x, localBox.min.y, localBox.min.z),
                SR_MATH_NS::FVector3(localBox.min.x, localBox.max.y, localBox.min.z),
                SR_MATH_NS::FVector3(localBox.max.x, localBox.max.y, localBox.min.z),
                SR_MATH_NS::FVector3(localBox.min.x, localBox.min.y, localBox.max.z),
                SR_MATH_NS::FVector3(localBox.max.x, localBox.min.y, localBox.max.z),
                SR_MATH_NS::FVector3(localBox.min.x, localBox.max.y, localBox.max.z),
                SR_MATH_NS::FVector3(localBox.max.x, localBox.max.y, localBox.max.z)
        };

        SR_MATH_NS::FVector3 newMin(
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()
        );
        SR_MATH_NS::FVector3 newMax(
                std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest()
        );

        for (auto& c : corners) {
            SR_MATH_NS::FVector4 world = TransformVector(SR_MATH_NS::FVector4(c, 1.f));

            newMin.x = std::min(newMin.x, world.x);
            newMin.y = std::min(newMin.y, world.y);
            newMin.z = std::min(newMin.z, world.z);

            newMax.x = std::max(newMax.x, world.x);
            newMax.y = std::max(newMax.y, world.y);
            newMax.z = std::max(newMax.z, world.z);
        }

        return { newMin, newMax };
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
}
