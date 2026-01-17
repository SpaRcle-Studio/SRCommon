//
// Created by Monika on 17.01.2023.
//

#ifndef SR_ENGINE_MATRIX4X4UTILS_H
#define SR_ENGINE_MATRIX4X4UTILS_H

#include <Utils/Math/Mathematics.h>
#include <Utils/Math/Vector3.h>

namespace SR_MATH_NS {
    SR_INLINE_STATIC void SR_FASTCALL GLMScaleMat4x4(glm::mat4& matrix, const SR_MATH_NS::FVector3& v) {
        // колонка 0
        matrix[0][0] *= v.x;
        matrix[0][1] *= v.x;
        matrix[0][2] *= v.x;
        matrix[0][3] *= v.x;

        // колонка 1
        matrix[1][0] *= v.y;
        matrix[1][1] *= v.y;
        matrix[1][2] *= v.y;
        matrix[1][3] *= v.y;

        // колонка 2
        matrix[2][0] *= v.z;
        matrix[2][1] *= v.z;
        matrix[2][2] *= v.z;
        matrix[2][3] *= v.z;
    }

    SR_INLINE_STATIC void SR_FASTCALL GLMRotateMat4x4_Fast(glm::mat4& m, const glm::quat& q) noexcept {
        if (q.w == 1.f) {
            return; // identity rotation
        }

        const float x2 = q.x + q.x;
        const float y2 = q.y + q.y;
        const float z2 = q.z + q.z;

        const float xx = q.x * x2;
        const float yy = q.y * y2;
        const float zz = q.z * z2;
        const float xy = q.x * y2;
        const float xz = q.x * z2;
        const float yz = q.y * z2;
        const float wx = q.w * x2;
        const float wy = q.w * y2;
        const float wz = q.w * z2;

        glm::vec4 r0(1 - (yy + zz), xy + wz,       xz - wy,       0);
        glm::vec4 r1(xy - wz,       1 - (xx + zz), yz + wx,       0);
        glm::vec4 r2(xz + wy,       yz - wx,       1 - (xx + yy), 0);

        glm::vec4 c0 = m[0];
        glm::vec4 c1 = m[1];
        glm::vec4 c2 = m[2];

        m[0] = c0 * r0.x + c1 * r0.y + c2 * r0.z;
        m[1] = c0 * r1.x + c1 * r1.y + c2 * r1.z;
        m[2] = c0 * r2.x + c1 * r2.y + c2 * r2.z;
    }

    SR_INLINE_STATIC void SR_FASTCALL GLMTranslateMat4x4(glm::mat4& matrix, const SR_MATH_NS::FVector3& v) {
        auto& B0 = matrix[0];
        auto& B1 = matrix[1];
        auto& B2 = matrix[2];
        auto& B3 = matrix[3];

        B3.x += B0.x * v.x + B1.x * v.y + B2.x * v.z;
        B3.y += B0.y * v.x + B1.y * v.y + B2.y * v.z;
        B3.z += B0.z * v.x + B1.z * v.y + B2.z * v.z;
        B3.w += B0.w * v.x + B1.w * v.y + B2.w * v.z;
    }

    SR_INLINE_STATIC void SR_FASTCALL GLMMultiplyMat4x4(glm::mat4& result, const glm::mat4& m1, const glm::mat4& m2) noexcept {
    #if SR_SIMD_SUPPORT && 0
        const __m128 a0 = _mm_loadu_ps(&m1[0][0]);
        const __m128 a1 = _mm_loadu_ps(&m1[1][0]);
        const __m128 a2 = _mm_loadu_ps(&m1[2][0]);
        const __m128 a3 = _mm_loadu_ps(&m1[3][0]);

        __m128 b = _mm_loadu_ps(&m2[0][0]);
        __m128 r = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(b, b, 0x00), a0), _mm_mul_ps(_mm_shuffle_ps(b, b, 0x55), a1)), _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(b, b, 0xAA), a2), _mm_mul_ps(_mm_shuffle_ps(b, b, 0xFF), a3)));
        _mm_storeu_ps(&result[0][0], r);

        b = _mm_loadu_ps(&m2[1][0]);
        r = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(b, b, 0x00), a0), _mm_mul_ps(_mm_shuffle_ps(b, b, 0x55), a1)), _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(b, b, 0xAA), a2), _mm_mul_ps(_mm_shuffle_ps(b, b, 0xFF), a3)));
        _mm_storeu_ps(&result[1][0], r);

        b = _mm_loadu_ps(&m2[2][0]);
        r = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(b, b, 0x00), a0), _mm_mul_ps(_mm_shuffle_ps(b, b, 0x55), a1)), _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(b, b, 0xAA), a2), _mm_mul_ps(_mm_shuffle_ps(b, b, 0xFF), a3)));
        _mm_storeu_ps(&result[2][0], r);

        b = _mm_loadu_ps(&m2[3][0]);
        r = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(b, b, 0x00), a0), _mm_mul_ps(_mm_shuffle_ps(b, b, 0x55), a1)), _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(b, b, 0xAA), a2), _mm_mul_ps(_mm_shuffle_ps(b, b, 0xFF), a3)));
        _mm_storeu_ps(&result[3][0], r);
    #else
        const float* SR_RESTRICT a = &(m1[0].x);
        const float* SR_RESTRICT b = &(m2[0].x);
        float* SR_RESTRICT r = &(result[0].x);

        r[0]  = a[0] * b[0]  + a[4] * b[1]  + a[8]  * b[2]  + a[12] * b[3];
        r[1]  = a[1] * b[0]  + a[5] * b[1]  + a[9]  * b[2]  + a[13] * b[3];
        r[2]  = a[2] * b[0]  + a[6] * b[1]  + a[10] * b[2]  + a[14] * b[3];
        r[3]  = a[3] * b[0]  + a[7] * b[1]  + a[11] * b[2]  + a[15] * b[3];

        r[4]  = a[0] * b[4]  + a[4] * b[5]  + a[8]  * b[6]  + a[12] * b[7];
        r[5]  = a[1] * b[4]  + a[5] * b[5]  + a[9]  * b[6]  + a[13] * b[7];
        r[6]  = a[2] * b[4]  + a[6] * b[5]  + a[10] * b[6]  + a[14] * b[7];
        r[7]  = a[3] * b[4]  + a[7] * b[5]  + a[11] * b[6]  + a[15] * b[7];

        r[8]  = a[0] * b[8]  + a[4] * b[9]  + a[8]  * b[10] + a[12] * b[11];
        r[9]  = a[1] * b[8]  + a[5] * b[9]  + a[9]  * b[10] + a[13] * b[11];
        r[10] = a[2] * b[8]  + a[6] * b[9]  + a[10] * b[10] + a[14] * b[11];
        r[11] = a[3] * b[8]  + a[7] * b[9]  + a[11] * b[10] + a[15] * b[11];

        r[12] = a[0] * b[12] + a[4] * b[13] + a[8]  * b[14] + a[12] * b[15];
        r[13] = a[1] * b[12] + a[5] * b[13] + a[9]  * b[14] + a[13] * b[15];
        r[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
        r[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
    #endif
    }

    SR_INLINE_STATIC void SR_FASTCALL GLMMultiplyMat4x4(glm::mat4& result, const glm::mat4& m1, const glm::mat3& m2) noexcept {
        typename glm::mat4::col_type& SrcA0 = *(typename glm::mat4::col_type*)(((char*)&m1) + sizeof(typename glm::mat4::col_type) * 0);
        typename glm::mat4::col_type& SrcA1 = *(typename glm::mat4::col_type*)(((char*)&m1) + sizeof(typename glm::mat4::col_type) * 1);
        typename glm::mat4::col_type& SrcA2 = *(typename glm::mat4::col_type*)(((char*)&m1) + sizeof(typename glm::mat4::col_type) * 2);
        typename glm::mat4::col_type& SrcA3 = *(typename glm::mat4::col_type*)(((char*)&m1) + sizeof(typename glm::mat4::col_type) * 3);

        typename glm::mat3::col_type& SrcB0 = *(typename glm::mat3::col_type*)(((char*)&m2) + sizeof(typename glm::mat3::col_type) * 0);
        typename glm::mat3::col_type& SrcB1 = *(typename glm::mat3::col_type*)(((char*)&m2) + sizeof(typename glm::mat3::col_type) * 1);
        typename glm::mat3::col_type& SrcB2 = *(typename glm::mat3::col_type*)(((char*)&m2) + sizeof(typename glm::mat3::col_type) * 2);

        typename glm::mat4::col_type& DstB0 = *(typename glm::mat4::col_type*)(((char*)&result) + sizeof(typename glm::mat4::col_type) * 0);
        typename glm::mat4::col_type& DstB1 = *(typename glm::mat4::col_type*)(((char*)&result) + sizeof(typename glm::mat4::col_type) * 1);
        typename glm::mat4::col_type& DstB2 = *(typename glm::mat4::col_type*)(((char*)&result) + sizeof(typename glm::mat4::col_type) * 2);
        typename glm::mat4::col_type& DstB3 = *(typename glm::mat4::col_type*)(((char*)&result) + sizeof(typename glm::mat4::col_type) * 3);

        DstB0.x = SrcA0.x * SrcB0.x + SrcA1.x * SrcB0.y + SrcA2.x * SrcB0.z;
        DstB0.y = SrcA0.y * SrcB0.x + SrcA1.y * SrcB0.y + SrcA2.y * SrcB0.z;
        DstB0.z = SrcA0.z * SrcB0.x + SrcA1.z * SrcB0.y + SrcA2.z * SrcB0.z;
        DstB0.w = SrcA0.w * SrcB0.x + SrcA1.w * SrcB0.y + SrcA2.w * SrcB0.z;

        DstB1.x = SrcA0.x * SrcB1.x + SrcA1.x * SrcB1.y + SrcA2.x * SrcB1.z;
        DstB1.y = SrcA0.y * SrcB1.x + SrcA1.y * SrcB1.y + SrcA2.y * SrcB1.z;
        DstB1.z = SrcA0.z * SrcB1.x + SrcA1.z * SrcB1.y + SrcA2.z * SrcB1.z;
        DstB1.w = SrcA0.w * SrcB1.x + SrcA1.w * SrcB1.y + SrcA2.w * SrcB1.z;

        DstB2.x = SrcA0.x * SrcB2.x + SrcA1.x * SrcB2.y + SrcA2.x * SrcB2.z;
        DstB2.y = SrcA0.y * SrcB2.x + SrcA1.y * SrcB2.y + SrcA2.y * SrcB2.z;
        DstB2.z = SrcA0.z * SrcB2.x + SrcA1.z * SrcB2.y + SrcA2.z * SrcB2.z;
        DstB2.w = SrcA0.w * SrcB2.x + SrcA1.w * SrcB2.y + SrcA2.w * SrcB2.z;

        DstB3.x = SrcA3.x;
        DstB3.y = SrcA3.y;
        DstB3.z = SrcA3.z;
        DstB3.w = SrcA3.w;
    }

    SR_NODISCARD SR_INLINE_STATIC glm::mat4 SR_FASTCALL GLMMultiplyMat4x4(const glm::mat4& m1, const glm::mat4& m2) noexcept {
        glm::mat4 result;
        GLMMultiplyMat4x4(result, m1, m2);
        return result;
    }

    SR_INLINE_STATIC void SR_FASTCALL GLMRotateMat4x4(glm::mat4& matrix, const glm::quat& q) {
        static const glm::mat3 mat3x3Identity = glm::mat3(1);
        glm::mat3 mat3x3 = mat3x3Identity;

        typename glm::mat3::col_type& Src0 = *(typename glm::mat3::col_type*)(((char*)&mat3x3) + sizeof(typename glm::mat3::col_type) * 0);
        typename glm::mat3::col_type& Src1 = *(typename glm::mat3::col_type*)(((char*)&mat3x3) + sizeof(typename glm::mat3::col_type) * 1);
        typename glm::mat3::col_type& Src2 = *(typename glm::mat3::col_type*)(((char*)&mat3x3) + sizeof(typename glm::mat3::col_type) * 2);

        /// rotation matrix
        {
            const float_t qxx(q.x * q.x);
            const float_t qyy(q.y * q.y);
            const float_t qzz(q.z * q.z);
            const float_t qxz(q.x * q.z);
            const float_t qxy(q.x * q.y);
            const float_t qyz(q.y * q.z);
            const float_t qwx(q.w * q.x);
            const float_t qwy(q.w * q.y);
            const float_t qwz(q.w * q.z);

            Src0.x = float_t(1) - float_t(2) * (qyy + qzz);
            Src0.y = float_t(2) * (qxy + qwz);
            Src0.z = float_t(2) * (qxz - qwy);

            Src1.x = float_t(2) * (qxy - qwz);
            Src1.y = float_t(1) - float_t(2) * (qxx + qzz);
            Src1.z = float_t(2) * (qyz + qwx);

            Src2.x = float_t(2) * (qxz + qwy);
            Src2.y = float_t(2) * (qyz - qwx);
            Src2.z = float_t(1) - float_t(2) * (qxx + qyy);
        }

        glm::mat4 source = matrix;
        GLMMultiplyMat4x4(matrix, source, mat3x3);
    }
}

#endif //SR_ENGINE_MATRIX4X4UTILS_H
