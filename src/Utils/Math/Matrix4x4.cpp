//
// Created by Monika on 16.01.2023.
//

#include <Utils/Math/Matrix4x4.h>
#include <Utils/Math/Matrix4x4Utils.h>

namespace SR_MATH_NS {
    Matrix4x4::Matrix4x4(const FVector3& translate, const Quaternion& rotation, const FVector3& scale) noexcept
        : Matrix4x4()
    {
        GLMTranslateMat4x4(self, translate);
        GLMRotateMat4x4(self, rotation.ToGLM());
        GLMScaleMat4x4(self, scale);
    }

    Matrix4x4::Matrix4x4(const FVector3& translate, const Quaternion& rotation, const FVector3& scale, const FVector3& skew) noexcept
        : Matrix4x4()
    {
        GLMTranslateMat4x4(self, translate);

        GLMScaleMat4x4(self, skew);
        GLMRotateMat4x4(self, rotation.ToGLM());
        GLMScaleMat4x4(self, scale);
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& mat) const {
        return Matrix4x4(GLMMultiplyMat4x4(self, mat.self));
    }

    Matrix4x4::Matrix4x4(const FVector3& translate, const Quaternion& rotation) noexcept
        : Matrix4x4()
    {
        GLMTranslateMat4x4(self, translate);
        GLMRotateMat4x4(self, rotation.ToGLM());
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
        GLMRotateMat4x4(self, rotation.ToGLM());
        GLMScaleMat4x4(self, scale);
    }
}
