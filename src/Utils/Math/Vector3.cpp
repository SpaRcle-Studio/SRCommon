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

    FVector3 KelvinToRGB(float_t K) {
        K = SR_MATH_NS::Clamp(K, 1000.0f, 20000.0f) / 100.0f;

        float_t r = 0.f, g = 0.f, b = 0.f;

        if (K <= 66.0f) {
            r = 1.0f;
            g = SR_MATH_NS::Clamp(0.3900815788f * static_cast<float>(log(K)) - 0.6318414438f, 0.0f, 1.0f);
            b = (K <= 19.0f) ? 0.0f : SR_MATH_NS::Clamp(0.5432067891f * static_cast<float>(log(K - 10.0f)) - 1.1962540891f, 0.0f, 1.0f);
        }
        else {
            r = SR_MATH_NS::Clamp(1.2929361861f * static_cast<float>(pow(K - 60.0f, -0.1332047592f)), 0.0f, 1.0f);
            g = SR_MATH_NS::Clamp(1.1298908616f * static_cast<float>(pow(K - 60.0f, -0.0755148492f)), 0.0f, 1.0f);
            b = 1.0f;
        }

        return FVector3(r, g, b);
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
