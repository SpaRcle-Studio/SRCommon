//
// Created by Monika on 06.07.2026.
//

#include <Utils/Math/Vector2.h>

namespace SR_MATH_NS {
    float_t DistanceToLineSegment(const FVector2 &point, const FVector2 &lineStart, const FVector2 &lineEnd) {
        const FVector2 line = lineEnd - lineStart;
        const float_t lineLengthSquared = line.Length() * line.Length();

        if (lineLengthSquared == 0.0f) {
            return point.Distance(lineStart);
        }

        const float_t t = std::max(0.0f, std::min(1.0f, ((point - lineStart).x * line.x + (point - lineStart).y * line.y) / lineLengthSquared));
        const FVector2 projection = lineStart + line * t;

        return point.Distance(projection);
    }
}