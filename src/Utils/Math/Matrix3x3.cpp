//
// Created by Nikita on 01.03.2021.
//

#include <Utils/Math/Matrix3x3.h>

namespace SR_MATH_NS {
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

    Matrix3x3::Matrix3x3(Quaternion rotation) {
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
}
