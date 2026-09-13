//
// Created by Nikita on 01.03.2021.
//

#include <Utils/Math/Matrix3x3.h>

namespace SR_MATH_NS {
    Matrix3x3::Matrix3x3() {
        elements[0] = FVector3(1, 0, 0);
        elements[1] = FVector3(0, 1, 0);
        elements[2] = FVector3(0, 0, 1);
    }

    FVector3 Matrix3x3::GetEulerXYZ() const {
        FVector3 euler;
        double sy = elements[0][2];
        if (sy < (1.0 - CMP_EPSILON)) {
            if (sy > -(1.0 - CMP_EPSILON)) {
                // is this a pure Y rotation?
                if (elements[1][0] == 0.0 && elements[0][1] == 0.0 && elements[1][2] == 0 && elements[2][1] == 0 && elements[1][1] == 1) {
                    // return the simplest form (human friendlier in editor and scripts)
                    euler.x = 0;
                    euler.y = atan2(elements[0][2], elements[0][0]);
                    euler.z = 0;
                } else {
                    euler.x = atan2(-elements[1][2], elements[2][2]);
                    euler.y = static_cast<float_t>(asin(sy));
                    euler.z = atan2(-elements[0][1], elements[0][0]);
                }
            } else {
                euler.x = atan2(elements[2][1], elements[1][1]);
                euler.y = static_cast<float_t>(-SR_PI / 2.0);
                euler.z = 0.0;
            }
        } else {
            euler.x = atan2(elements[2][1], elements[1][1]);
            euler.y = static_cast<float_t>(SR_PI / 2.0);
            euler.z = 0.0;
        }
        return euler;
    }

    Matrix3x3 Matrix3x3::Inverse() const {
        Matrix3x3 result;
        double det = elements[0][0] * (elements[1][1] * elements[2][2] - elements[1][2] * elements[2][1]) -
                     elements[0][1] * (elements[1][0] * elements[2][2] - elements[1][2] * elements[2][0]) +
                     elements[0][2] * (elements[1][0] * elements[2][1] - elements[1][1] * elements[2][0]);

        if (SR_ABS(det) < CMP_EPSILON) {
            return Matrix3x3::Identity();
        }

        double invDet = 1.0 / det;

        result.elements[0][0] = (elements[1][1] * elements[2][2] - elements[1][2] * elements[2][1]) * invDet;
        result.elements[0][1] = (elements[0][2] * elements[2][1] - elements[0][1] * elements[2][2]) * invDet;
        result.elements[0][2] = (elements[0][1] * elements[1][2] - elements[0][2] * elements[1][1]) * invDet;

        result.elements[1][0] = (elements[1][2] * elements[2][0] - elements[1][0] * elements[2][2]) * invDet;
        result.elements[1][1] = (elements[0][0] * elements[2][2] - elements[0][2] * elements[2][0]) * invDet;
        result.elements[1][2] = (elements[0][2] * elements[1][0] - elements[0][0] * elements[1][2]) * invDet;

        result.elements[2][0] = (elements[1][0] * elements[2][1] - elements[1][1] * elements[2][0]) * invDet;
        result.elements[2][1] = (elements[0][1] * elements[2][0] - elements[0][0] * elements[2][1]) * invDet;
        result.elements[2][2] = (elements[0][0] * elements[1][1] - elements[0][1] * elements[1][0]) * invDet;

        return result;
    }

    Matrix3x3 Matrix3x3::Identity() const {
        return Matrix3x3();
    }

    Matrix3x3 Matrix3x3::operator*(const Matrix3x3 &other) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.elements[i][j] = elements[i][0] * other.elements[0][j] +
                                        elements[i][1] * other.elements[1][j] +
                                        elements[i][2] * other.elements[2][j];
            }
        }
        return result;
    }

    Matrix3x3::Matrix3x3(const Quaternion& rotation) {
        float_t x = rotation.x;
        float_t y = rotation.y;
        float_t z = rotation.z;
        float_t w = rotation.w;

        elements[0][0] = 1 - 2 * (y * y + z * z);
        elements[0][1] = 2 * (x * y - z * w);
        elements[0][2] = 2 * (x * z + y * w);

        elements[1][0] = 2 * (x * y + z * w);
        elements[1][1] = 1 - 2 * (x * x + z * z);
        elements[1][2] = 2 * (y * z - x * w);

        elements[2][0] = 2 * (x * z - y * w);
        elements[2][1] = 2 * (y * z + x * w);
        elements[2][2] = 1 - 2 * (x * x + y * y);
    }

    Quaternion Matrix3x3::ToQuaternion() const {
        const Unit fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
        const Unit fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
        const Unit fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
        const Unit fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

        int32_t biggestIndex = 0;
        Unit fourBiggestSquaredMinus1 = fourWSquaredMinus1;
        if(fourXSquaredMinus1 > fourBiggestSquaredMinus1) {
            fourBiggestSquaredMinus1 = fourXSquaredMinus1;
            biggestIndex = 1;
        }
        if(fourYSquaredMinus1 > fourBiggestSquaredMinus1) {
            fourBiggestSquaredMinus1 = fourYSquaredMinus1;
            biggestIndex = 2;
        }
        if(fourZSquaredMinus1 > fourBiggestSquaredMinus1) {
            fourBiggestSquaredMinus1 = fourZSquaredMinus1;
            biggestIndex = 3;
        }

        Unit biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<Unit>(1)) * static_cast<Unit>(0.5);
        Unit mult = static_cast<Unit>(0.25) / biggestVal;

        /// матрица хранится по строкам (m[row][column]), поэтому антисимметричные разности
        /// берутся в порядке, обратном column-major реализации glm::quat_cast
        switch(biggestIndex) {
            case 0:
                return Quaternion::WXYZ(biggestVal, (m[2][1] - m[1][2]) * mult, (m[0][2] - m[2][0]) * mult, (m[1][0] - m[0][1]) * mult);
            case 1:
                return Quaternion::WXYZ((m[2][1] - m[1][2]) * mult, biggestVal, (m[1][0] + m[0][1]) * mult, (m[0][2] + m[2][0]) * mult);
            case 2:
                return Quaternion::WXYZ((m[0][2] - m[2][0]) * mult, (m[1][0] + m[0][1]) * mult, biggestVal, (m[2][1] + m[1][2]) * mult);
            case 3:
                return Quaternion::WXYZ((m[1][0] - m[0][1]) * mult, (m[0][2] + m[2][0]) * mult, (m[2][1] + m[1][2]) * mult, biggestVal);
            default:
                SRHalt("Matrix3x3::ToQuaternion() : invalid biggestIndex!");
                return Quaternion::WXYZ(1, 0, 0, 0);
        }
    }
}
