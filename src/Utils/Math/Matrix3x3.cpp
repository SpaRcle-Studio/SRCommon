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
}
