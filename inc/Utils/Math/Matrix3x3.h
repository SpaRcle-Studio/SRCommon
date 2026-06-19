//
// Created by Nikita on 01.03.2021.
//

#if !defined(SR_ENGINE_MATRIX3X3_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_ENGINE_MATRIX3X3_H

#include <Utils/Math/Quaternion.h>
#include <Utils/Math/Vector3.h>

namespace SR_MATH_NS {
    class SR_COMMON_DLL_API Matrix3x3 {
    public:
        FVector3 elements[3] = {
            FVector3(1, 0, 0),
            FVector3(0, 1, 0),
            FVector3(0, 0, 1)
        };

        Matrix3x3() = default;
        Matrix3x3(Quaternion rotation);
        Matrix3x3(const FVector3& row0, const FVector3& row1, const FVector3& row2) {
            elements[0] = row0;
            elements[1] = row1;
            elements[2] = row2;
        }

        SR_NODISCARD Matrix3x3 Inverse() const;
        SR_NODISCARD Matrix3x3 Identity() const;
        SR_NODISCARD Matrix3x3 operator*(const Matrix3x3& other) const;

        SR_FORCE_INLINE const FVector3 &operator[](int axis) const {
            return elements[axis];
        }
        SR_FORCE_INLINE FVector3 &operator[](int axis) {
            return elements[axis];
        }

        SR_FORCE_INLINE void Set(double xx, double xy, double xz, double yx, double yy, double yz, double zx, double zy, double zz) {
            elements[0][0] = xx;
            elements[0][1] = xy;
            elements[0][2] = xz;
            elements[1][0] = yx;
            elements[1][1] = yy;
            elements[1][2] = yz;
            elements[2][0] = zx;
            elements[2][1] = zy;
            elements[2][2] = zz;
        }

        FVector3 GetEulerXYZ() const;
    };
}

#endif //SR_ENGINE_MATRIX3X3_H
