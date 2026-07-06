//
// Created by Monika on 18.11.2022.
//

#include <Utils/Math/Rect.h>

namespace SR_MATH_NS {
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
