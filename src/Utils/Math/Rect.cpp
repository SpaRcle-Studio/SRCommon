//
// Created by Monika on 18.11.2022.
//

#include <Utils/Math/Rect.h>

namespace SR_MATH_NS {
    FVector2 ClipParallelLineToRect(const FRect& rect, const FVector2& linePoint, const FVector2& lineDir) {
        const FVector2 center = rect.Center();

        const float halfW = rect.Width() * 0.5f;
        const float halfH = rect.Height() * 0.5f;

        const float left   = center.x - halfW;
        const float right  = center.x + halfW;
        const float top    = center.y - halfH;
        const float bottom = center.y + halfH;

        float bestDistSq = std::numeric_limits<float>::max();
        FVector2 bestPoint = center;

        auto check = [&](float t) {
            const FVector2 point = linePoint + lineDir * t;

            constexpr float epsilon = 0.001f;

            if (point.x < left - epsilon ||
                point.x > right + epsilon ||
                point.y < top - epsilon ||
                point.y > bottom + epsilon) {
                return;
            }

            const FVector2 delta = point - center;
            const float distSq = delta.x * delta.x + delta.y * delta.y;

            if (distSq < bestDistSq) {
                bestDistSq = distSq;
                bestPoint = point;
            }
        };

        // Левая / правая стороны.
        if (std::abs(lineDir.x) > 0.0001f) {
            check((left  - linePoint.x) / lineDir.x);
            check((right - linePoint.x) / lineDir.x);
        }

        // Верхняя / нижняя стороны.
        if (std::abs(lineDir.y) > 0.0001f) {
            check((top    - linePoint.y) / lineDir.y);
            check((bottom - linePoint.y) / lineDir.y);
        }

        return bestPoint;
    }

    bool ClipRayToRect(const FRect& rect, const FVector2& linePoint, const FVector2& dir, FVector2& outPoint) {
        const FVector2 min = rect.Min();
        const FVector2 max = rect.Max();

        float bestT = std::numeric_limits<float>::infinity();

        auto check = [&](float t) {
            if (t < 0.0f || t >= bestT) {
                return;
            }

            const FVector2 p = linePoint + dir * t;

            if (p.x >= min.x - 0.001f &&
                p.x <= max.x + 0.001f &&
                p.y >= min.y - 0.001f &&
                p.y <= max.y + 0.001f) {
                bestT = t;
            }
        };

        if (std::abs(dir.x) > 0.0001f) {
            check((min.x - linePoint.x) / dir.x);
            check((max.x - linePoint.x) / dir.x);
        }

        if (std::abs(dir.y) > 0.0001f) {
            check((min.y - linePoint.y) / dir.y);
            check((max.y - linePoint.y) / dir.y);
        }

        if (!std::isfinite(bestT)) {
            return false;
        }

        outPoint = linePoint + dir * bestT;
        return true;
    }

    bool ClipLineToRect(const FRect& rect, const FVector2& linePoint, const FVector2& lineDir, FVector2& outNear, FVector2& outFar) {
        const FVector2 min = rect.Min();
        const FVector2 max = rect.Max();

        float tMin = -std::numeric_limits<float>::infinity();
        float tMax =  std::numeric_limits<float>::infinity();

        // X slab
        if (std::abs(lineDir.x) < 0.0001f) {
            if (linePoint.x < min.x || linePoint.x > max.x) {
                return false;
            }
        }
        else {
            const float t1 = (min.x - linePoint.x) / lineDir.x;
            const float t2 = (max.x - linePoint.x) / lineDir.x;

            tMin = std::max(tMin, std::min(t1, t2));
            tMax = std::min(tMax, std::max(t1, t2));
        }

        // Y slab
        if (std::abs(lineDir.y) < 0.0001f) {
            if (linePoint.y < min.y || linePoint.y > max.y) {
                return false;
            }
        }
        else {
            const float t1 = (min.y - linePoint.y) / lineDir.y;
            const float t2 = (max.y - linePoint.y) / lineDir.y;

            tMin = std::max(tMin, std::min(t1, t2));
            tMax = std::min(tMax, std::max(t1, t2));
        }

        if (tMin > tMax) {
            return false;
        }

        outNear = linePoint + lineDir * tMin;
        outFar  = linePoint + lineDir * tMax;

        return true;
    }

    FVector2 ClipToRectEdge(const FRect& rect, const FVector2& from, const FVector2& to) {
        const FVector2 c = rect.Center();
        FVector2 dir = to - from;
        const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        dir = len > 0.0001f ? (dir / len) : FVector2(1.f, 0.f);

        const float halfW = rect.Width() * 0.5f;
        const float halfH = rect.Height() * 0.5f;

        const float adx = std::abs(dir.x);
        const float ady = std::abs(dir.y);

        float s = 0.f;
        if (adx * halfH > ady * halfW) {
            // hit left/right
            s = halfW / (adx > 0.0001f ? adx : 1.f);
        }
        else {
            // hit top/bottom
            s = halfH / (ady > 0.0001f ? ady : 1.f);
        }

        return c + dir * s;
    }
}
