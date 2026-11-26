//
// Created by Monika on 17.01.2023.
//

#ifndef SR_ENGINE_MATRIX4X4UTILS_H
#define SR_ENGINE_MATRIX4X4UTILS_H

#include <Utils/Math/Matrix4x4.h>
#include <Utils/Math/Vector3.h>

namespace SR_MATH_NS {
    SR_INLINE_STATIC void SR_FASTCALL ScaleMat4x4(Matrix4x4& matrix, const SR_MATH_NS::FVector3& v) {
    #if SR_SIMD_SUPPORT
        const __m128 scaleX = _mm_set1_ps(v.x);
        const __m128 scaleY = _mm_set1_ps(v.y);
        const __m128 scaleZ = _mm_set1_ps(v.z);
        _mm_storeu_ps(matrix.mm, _mm_mul_ps(_mm_loadu_ps(matrix.mm), scaleX));
        _mm_storeu_ps(matrix.mm + 4, _mm_mul_ps(_mm_loadu_ps(matrix.mm + 4), scaleY));
        _mm_storeu_ps(matrix.mm + 8, _mm_mul_ps(_mm_loadu_ps(matrix.mm + 8), scaleZ));
    #else
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
    #endif
    }

    SR_INLINE_STATIC void SR_FASTCALL RotateMat4x4(Matrix4x4& m, const Quaternion& q) noexcept {
        if (q.IsIdentity()) {
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

        FVector4 r0(1 - (yy + zz), xy + wz,       xz - wy,       0);
        FVector4 r1(xy - wz,       1 - (xx + zz), yz + wx,       0);
        FVector4 r2(xz + wy,       yz - wx,       1 - (xx + yy), 0);

    #if SR_SIMD_SUPPORT
        const __m128 column0 = _mm_loadu_ps(m.mm);
        const __m128 column1 = _mm_loadu_ps(m.mm + 4);
        const __m128 column2 = _mm_loadu_ps(m.mm + 8);
        __m128 rotated0 = _mm_mul_ps(column0, _mm_set1_ps(r0.x));
        rotated0 = _mm_add_ps(rotated0, _mm_mul_ps(column1, _mm_set1_ps(r0.y)));
        rotated0 = _mm_add_ps(rotated0, _mm_mul_ps(column2, _mm_set1_ps(r0.z)));
        _mm_storeu_ps(m.mm, rotated0);

        __m128 rotated1 = _mm_mul_ps(column0, _mm_set1_ps(r1.x));
        rotated1 = _mm_add_ps(rotated1, _mm_mul_ps(column1, _mm_set1_ps(r1.y)));
        rotated1 = _mm_add_ps(rotated1, _mm_mul_ps(column2, _mm_set1_ps(r1.z)));
        _mm_storeu_ps(m.mm + 4, rotated1);

        __m128 rotated2 = _mm_mul_ps(column0, _mm_set1_ps(r2.x));
        rotated2 = _mm_add_ps(rotated2, _mm_mul_ps(column1, _mm_set1_ps(r2.y)));
        rotated2 = _mm_add_ps(rotated2, _mm_mul_ps(column2, _mm_set1_ps(r2.z)));
        _mm_storeu_ps(m.mm + 8, rotated2);
    #else
        FVector4 c0 = m[0];
        FVector4 c1 = m[1];
        FVector4 c2 = m[2];

        m[0] = c0 * r0.x + c1 * r0.y + c2 * r0.z;
        m[1] = c0 * r1.x + c1 * r1.y + c2 * r1.z;
        m[2] = c0 * r2.x + c1 * r2.y + c2 * r2.z;
    #endif
    }

    SR_INLINE_STATIC void SR_FASTCALL TranslateMat4x4(Matrix4x4& matrix, const SR_MATH_NS::FVector3& v) {
    #if SR_SIMD_SUPPORT
        __m128 offset = _mm_mul_ps(_mm_loadu_ps(matrix.mm), _mm_set1_ps(v.x));
        offset = _mm_add_ps(offset, _mm_mul_ps(_mm_loadu_ps(matrix.mm + 4), _mm_set1_ps(v.y)));
        offset = _mm_add_ps(offset, _mm_mul_ps(_mm_loadu_ps(matrix.mm + 8), _mm_set1_ps(v.z)));
        _mm_storeu_ps(matrix.mm + 12, _mm_add_ps(_mm_loadu_ps(matrix.mm + 12), offset));
    #else
        auto&& B0 = matrix[0];
        auto&& B1 = matrix[1];
        auto&& B2 = matrix[2];
        auto&& B3 = matrix[3];

        B3.x += B0.x * v.x + B1.x * v.y + B2.x * v.z;
        B3.y += B0.y * v.x + B1.y * v.y + B2.y * v.z;
        B3.z += B0.z * v.x + B1.z * v.y + B2.z * v.z;
        B3.w += B0.w * v.x + B1.w * v.y + B2.w * v.z;
    #endif
    }

    SR_INLINE_STATIC void SR_FASTCALL MultiplyMat4x4(Matrix4x4& result, const Matrix4x4& left, const Matrix4x4& right) noexcept {
        Matrix4x4 product(0.f);
    #if SR_SIMD_SUPPORT
        const __m128 left0 = _mm_loadu_ps(left.mm);
        const __m128 left1 = _mm_loadu_ps(left.mm + 4);
        const __m128 left2 = _mm_loadu_ps(left.mm + 8);
        const __m128 left3 = _mm_loadu_ps(left.mm + 12);
        const __m128 rightColumn0 = _mm_loadu_ps(right.mm);
        __m128 output0 = _mm_mul_ps(left0, _mm_shuffle_ps(rightColumn0, rightColumn0, _MM_SHUFFLE(0, 0, 0, 0)));
        output0 = _mm_add_ps(output0, _mm_mul_ps(left1, _mm_shuffle_ps(rightColumn0, rightColumn0, _MM_SHUFFLE(1, 1, 1, 1))));
        output0 = _mm_add_ps(output0, _mm_mul_ps(left2, _mm_shuffle_ps(rightColumn0, rightColumn0, _MM_SHUFFLE(2, 2, 2, 2))));
        output0 = _mm_add_ps(output0, _mm_mul_ps(left3, _mm_shuffle_ps(rightColumn0, rightColumn0, _MM_SHUFFLE(3, 3, 3, 3))));
        _mm_storeu_ps(product.mm, output0);

        const __m128 rightColumn1 = _mm_loadu_ps(right.mm + 4);
        __m128 output1 = _mm_mul_ps(left0, _mm_shuffle_ps(rightColumn1, rightColumn1, _MM_SHUFFLE(0, 0, 0, 0)));
        output1 = _mm_add_ps(output1, _mm_mul_ps(left1, _mm_shuffle_ps(rightColumn1, rightColumn1, _MM_SHUFFLE(1, 1, 1, 1))));
        output1 = _mm_add_ps(output1, _mm_mul_ps(left2, _mm_shuffle_ps(rightColumn1, rightColumn1, _MM_SHUFFLE(2, 2, 2, 2))));
        output1 = _mm_add_ps(output1, _mm_mul_ps(left3, _mm_shuffle_ps(rightColumn1, rightColumn1, _MM_SHUFFLE(3, 3, 3, 3))));
        _mm_storeu_ps(product.mm + 4, output1);

        const __m128 rightColumn2 = _mm_loadu_ps(right.mm + 8);
        __m128 output2 = _mm_mul_ps(left0, _mm_shuffle_ps(rightColumn2, rightColumn2, _MM_SHUFFLE(0, 0, 0, 0)));
        output2 = _mm_add_ps(output2, _mm_mul_ps(left1, _mm_shuffle_ps(rightColumn2, rightColumn2, _MM_SHUFFLE(1, 1, 1, 1))));
        output2 = _mm_add_ps(output2, _mm_mul_ps(left2, _mm_shuffle_ps(rightColumn2, rightColumn2, _MM_SHUFFLE(2, 2, 2, 2))));
        output2 = _mm_add_ps(output2, _mm_mul_ps(left3, _mm_shuffle_ps(rightColumn2, rightColumn2, _MM_SHUFFLE(3, 3, 3, 3))));
        _mm_storeu_ps(product.mm + 8, output2);

        const __m128 rightColumn3 = _mm_loadu_ps(right.mm + 12);
        __m128 output3 = _mm_mul_ps(left0, _mm_shuffle_ps(rightColumn3, rightColumn3, _MM_SHUFFLE(0, 0, 0, 0)));
        output3 = _mm_add_ps(output3, _mm_mul_ps(left1, _mm_shuffle_ps(rightColumn3, rightColumn3, _MM_SHUFFLE(1, 1, 1, 1))));
        output3 = _mm_add_ps(output3, _mm_mul_ps(left2, _mm_shuffle_ps(rightColumn3, rightColumn3, _MM_SHUFFLE(2, 2, 2, 2))));
        output3 = _mm_add_ps(output3, _mm_mul_ps(left3, _mm_shuffle_ps(rightColumn3, rightColumn3, _MM_SHUFFLE(3, 3, 3, 3))));
        _mm_storeu_ps(product.mm + 12, output3);
    #else
        for (int32_t column = 0; column < 4; ++column) {
            for (int32_t row = 0; row < 4; ++row) {
                product.m[column][row] = left.m[0][row] * right.m[column][0]
                    + left.m[1][row] * right.m[column][1]
                    + left.m[2][row] * right.m[column][2]
                    + left.m[3][row] * right.m[column][3];
            }
        }
    #endif
        result = product;
    }
}

#endif // SR_ENGINE_MATRIX4X4UTILS_H
