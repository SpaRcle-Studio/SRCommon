//
// Created by Nikita on 01.03.2021.
//

#include <Utils/Math/Vector3.h>
#include <Utils/Math/Quaternion.h>

#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SR_MATH_NS {
    SR_MATH_NS::FVector3 ProjectOnPlane(
        const FVector3& point,
        const FVector3& planeOrigin,
        const FVector3& planeTarget,
        const FVector3& planeNormal,
        float_t weight
    ) {
        // Calculate the plane's normal if not provided
        SR_MATH_NS::FVector3 normal = planeNormal;
        if (normal == SR_MATH_NS::FVector3(0, 0, 0)) {
            normal = (planeTarget - planeOrigin).Normalized();
        }

        // Calculate the vector from the plane origin to the point
        SR_MATH_NS::FVector3 toPoint = point - planeOrigin;

        // Project the vector onto the plane
        SR_MATH_NS::FVector3 projection = toPoint - normal * toPoint.Dot(normal);

        // Interpolate the projection based on the weight
        return planeOrigin.Lerp(planeOrigin + projection, weight);
    }

    FVector3 GetPerpendicularVector(const FVector3& direction) {
        Vector3 perpendicular = FVector3::Cross(direction, FVector3::Up());
        if (perpendicular.Magnitude() < 0.0001f)
            perpendicular = FVector3::Cross(direction, FVector3::Right());
        if (perpendicular.Magnitude() < 0.0001f)
            perpendicular = FVector3::Cross(direction, FVector3::Forward());
        return perpendicular.Normalized();
    }

    FVector3 Mix(const FVector3& a, const FVector3& b, const float_t t) {
        return a * (1.0f - t) + b * t;
    }

    FVector3 SafePerpendicular(const FVector3& v) {
        FVector3 perp = GetPerpendicularVector(v);
        const float mag = perp.Magnitude();
        if (mag > 0.0001f) {
            return perp / mag;
        }
        return FVector3::Up();
    }

    FVector3 ProjectOnPlane(const FVector3& v, const FVector3&planeNormal) {
        return v - FVector3::Project(v, planeNormal);
    }
}
