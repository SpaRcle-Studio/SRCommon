//
// Created by Monika on 05.12.2025.
//

#include <Utils/Math/Curve.h>
#include <Utils/Math/Mathematics.h>

namespace SR_MATH_NS {
    float Curve::Evaluate(float t) const {
        int count = (int)keys.size();
        if (count == 0) return 0.f;
        if (count == 1) return keys[0].value;

        float start = keys.front().time;
        float end   = keys.back().time;

        if (t < start)
            t = ApplyWrap(t, start, end, preWrap, true);
        else if (t > end)
            t = ApplyWrap(t, start, end, postWrap, false);

        lastSegmentCache = FindSegment(t);
        if (lastSegmentCache < 0)
            return keys.back().value;

        const CurveKeyframe& k0 = keys[lastSegmentCache];
        const CurveKeyframe& k1 = keys[lastSegmentCache + 1];

        float dt = k1.time - k0.time;
        if (dt < 1e-6f) return k0.value;

        float u = (t - k0.time) / dt;
        if (u <= 0.f) return k0.value;
        if (u >= 1.f) return k1.value;

        switch (k0.mode) {
            case CurveInterpMode::Constant: return k0.value;
            case CurveInterpMode::Linear:   return Lerp(k0.value, k1.value, u);
            case CurveInterpMode::Hermite:  return Hermite(k0, k1, u, dt);
            case CurveInterpMode::WeightedBezier:
                return Bezier(k0, k1, u, dt);
            default:
                SRHalt("Unknown CurveInterpMode!");
                break;
        }

        return 0.f;
    }

    uint64_t Curve::AddKey(const CurveKeyframe &k) {
        auto it = std::lower_bound(keys.begin(), keys.end(), k.time, [](auto const& a, float t) {
            return a.time < t;
        });

        const uint64_t idx = uint64_t(it - keys.begin());
        keys.insert(it, k);
        return idx;
    }

    void Curve::ComputeAutoTangents() {
        int n = (int)keys.size();
        if (n <= 1) return;

        for (int i = 0; i < n; ++i) {
            CurveKeyframe& k = keys[i];

            if (k.tangentMode == CurveTangentMode::Flat) {
                k.inTangent = 0.f;
                k.outTangent = 0.f;
                continue;
            }

            if (k.tangentMode == CurveTangentMode::Smooth) {
                float prevVal = (i > 0)     ? keys[i-1].value : k.value;
                float nextVal = (i + 1 < n) ? keys[i+1].value : k.value;
                float dtPrev  = (i > 0)     ? (k.time - keys[i-1].time) : 1.f;
                float dtNext  = (i + 1 < n) ? (keys[i+1].time - k.time) : 1.f;

                float slope = 0.f;
                if (dtPrev > 0 && dtNext > 0)
                    slope = (nextVal - prevVal) / (dtPrev + dtNext);

                k.inTangent = slope;
                k.outTangent = slope;
                continue;
            }

            if (k.tangentMode == CurveTangentMode::Free ||
                k.tangentMode == CurveTangentMode::Broken) {
                // оставляем как есть
            }
        }
    }

    float Curve::ApplyWrap(float t, float start, float end, CurveWrapMode m, bool isPre) const {
        float len = end - start;
        if (len <= 0.f) return start;

        switch (m) {
            case CurveWrapMode::Clamp:
                return isPre ? start : end;
            case CurveWrapMode::Loop: {
                float x = fmodf(t - start, len);
                if (x < 0.f) x += len;
                return start + x;
            }
            case CurveWrapMode::PingPong: {
                float two = len * 2.f;
                float x = fmodf(t - start, two);
                if (x < 0.f) x += two;
                if (x <= len) return start + x;
                return end - (x - len);
            }
        }
        return t;
    }

    int Curve::FindSegment(float t) const {
        int n = (int)keys.size();
        if (n < 2) {
            return -1;
        }

        if (lastSegmentCache >= 0 && lastSegmentCache + 1 < n) {
            if (t >= keys[lastSegmentCache].time && t <= keys[lastSegmentCache + 1].time) {
                return lastSegmentCache;
            }
        }

        int lo = 0, hi = n - 1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if (t < keys[mid].time) hi = mid - 1;
            else lo = mid + 1;
        }

        int seg = lo - 1;
        if (seg < 0) seg = 0;
        if (seg >= n - 1) seg = n - 2;

        return seg;
    }

    float Curve::Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    float Curve::Hermite(const CurveKeyframe &k0, const CurveKeyframe &k1, float u, float dt) {
        float u2 = u * u;
        float u3 = u2 * u;

        float h00 = 2.f * u3 - 3.f * u2 + 1.f;
        float h10 = u3 - 2.f * u2 + u;
        float h01 = -2.f * u3 + 3.f * u2;
        float h11 = u3 - u2;

        float m0 = k0.outTangent * dt;
        float m1 = k1.inTangent * dt;

        return h00 * k0.value + h10 * m0 + h01 * k1.value + h11 * m1;
    }

    float Curve::Bezier(const CurveKeyframe &k0, const CurveKeyframe &k1, float u, float dt) {
        float p0 = k0.value;

        float p1 = p0 + k0.outTangent * (k0.outWeight * dt);
        float p3 = k1.value;
        float p2 = p3 - k1.inTangent * (k1.inWeight * dt);

        float u2 = u * u;
        float u3 = u2 * u;

        float inv = 1.f - u;
        float inv2 = inv * inv;
        float inv3 = inv2 * inv;

        return p0 * inv3 +
               3.f * p1 * inv2 * u +
               3.f * p2 * inv * u2 +
               p3 * u3;
    }

    void Curve::RemoveKey(int index) {
        if (index >= 0 && index < (int)keys.size()) {
            keys.erase(keys.begin() + index);
        }
    }

    float_t Curve::SinusoidalEaseInOut(float_t t) {
        return -0.5f * (cosf(static_cast<float_t>(SR_PI) * t) - 1.f);
    }

    float_t Curve::QuadraticEaseInOut(float_t t) {
        return t < 0.5f ? 2.f * t * t : -1.f + (4.f - 2.f * t) * t;
    }

    float_t Curve::CubicEaseInOut(float_t t) {
        return t < 0.5f ? 4.f * t * t * t : (t - 1.f) * (2.f * t - 2.f) * (2.f * t - 2.f) + 1.f;
    }

    float_t Curve::QuarticEaseInOut(float_t t) {
        return t < 0.5f ? 8.f * t * t * t * t : 1.f - 8.f * (t - 1.f) * (t - 1.f) * (t - 1.f) * (t - 1.f);
    }

    float_t Curve::QuinticEaseInOut(float_t t) {
        if (t < 0.5f)
            return 16.f * t * t * t * t * t;

        float u = (2.f * t) - 2.f;
        return 1.f + 0.5f * (u * u * u * u * u);
    }

    float_t Curve::ExponentialEaseInOut(float_t t) {
        if (t == 0.f) return 0.f;
        if (t == 1.f) return 1.f;
        if (t < 0.5f) return 0.5f * powf(2.f, (20.f * t) - 10.f);
        return -0.5f * powf(2.f, (-20.f * t) + 10.f) + 1.f;
    }

    float_t Curve::CircularEaseInOut(float_t t) {
        if (t < 0.5f) {
            return 0.5f * (1.f - SR_MATH_NS::Sqrt(1.f - (2.f * t) * (2.f * t)));
        }
        return 0.5f * (SR_MATH_NS::Sqrt(1.f - (2.f * t - 2.f) * (2.f * t - 2.f)) + 1.f);
    }

    float_t Curve::SmoothStep(float_t edge0, float_t edge1, float_t x) {
        const double_t t = SR_CLAMP((static_cast<double_t>(x) - static_cast<double_t>(edge0)) / (static_cast<double_t>(edge1) - static_cast<double_t>(edge0)), 0.0, 1.0);
        return static_cast<float_t>(t * t * (3.0 - 2.0 * t));
    }
}
