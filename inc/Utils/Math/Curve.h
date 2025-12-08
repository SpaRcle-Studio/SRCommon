//
// Created by Monika on 05.12.2025.
//

#ifndef SR_ENGINE_UTILS_MATH_CURVE_H
#define SR_ENGINE_UTILS_MATH_CURVE_H

#include <Utils/Common/Enumerations.h>

namespace SR_MATH_NS {
    SR_ENUM_NS_CLASS_T(CurveWrapMode, uint8_t,
        Clamp,
        Loop,
        PingPong
    )

    SR_ENUM_NS_CLASS_T(CurveInterpMode, uint8_t,
       Constant,
       Linear,
       Hermite,         // обычный Hermite
       WeightedBezier   // Unity-style weighted tangents
    )

    SR_ENUM_NS_CLASS_T(CurveTangentMode, uint8_t,
       Free,
       Smooth,
       Flat,
       Broken
    )

    SR_ENUM_NS_CLASS_T(CurveWeightedMode, uint8_t,
       None,
       In,
       Out,
       Both
    )

    struct CurveKeyframe {
        float_t time = 0.f;
        float_t value = 0.f;

        float_t inTangent = 0.f;
        float_t outTangent = 0.f;

        float_t inWeight = 0.333333f;
        float_t outWeight = 0.333333f;

        CurveTangentMode tangentMode = CurveTangentMode::Free;
        CurveWeightedMode weightedMode = CurveWeightedMode::None;

        CurveInterpMode mode = CurveInterpMode::Hermite;

        CurveKeyframe() = default;
        CurveKeyframe(float_t t, float_t v)
            : time(t)
            , value(v)
        { }
    };

    class Curve {
    public:
        Curve() = default;

        uint64_t AddKey(const CurveKeyframe& k);
        void RemoveKey(int index);

        SR_NODISCARD uint64_t KeyCount() const { return (int)keys.size(); }

        void SetPreWrap(CurveWrapMode m) { preWrap = m; }
        void SetPostWrap(CurveWrapMode m) { postWrap = m; }

        SR_NODISCARD float Evaluate(float t) const;

        std::vector<CurveKeyframe>& GetKeys() { return keys; }
        const std::vector<CurveKeyframe>& GetKeys() const { return keys; }

        // Автоматическое вычисление тангентов
        void ComputeAutoTangents();

    public:
        SR_NODISCARD static float_t SinusoidalEaseInOut(float_t t);
        SR_NODISCARD static float_t QuadraticEaseInOut(float_t t);
        SR_NODISCARD static float_t CubicEaseInOut(float_t t);
        SR_NODISCARD static float_t QuarticEaseInOut(float_t t);
        SR_NODISCARD static float_t QuinticEaseInOut(float_t t);
        SR_NODISCARD static float_t ExponentialEaseInOut(float_t t);
        SR_NODISCARD static float_t CircularEaseInOut(float_t t);
        SR_NODISCARD static float_t SmoothStep(float_t edge0, float_t edge1, float_t x);

    private:
        std::vector<CurveKeyframe> keys;
        CurveWrapMode preWrap = CurveWrapMode::Clamp;
        CurveWrapMode postWrap = CurveWrapMode::Clamp;

        mutable int lastSegmentCache = -1;

    private:
        float ApplyWrap(float t, float start, float end, CurveWrapMode m, bool isPre) const;
        int FindSegment(float t) const;

        static float Lerp(float a, float b, float t);
        static float Hermite(const CurveKeyframe& k0, const CurveKeyframe& k1, float u, float dt);
        static float Bezier(const CurveKeyframe& k0, const CurveKeyframe& k1, float u, float dt);
    };
}

#endif //SR_ENGINE_UTILS_MATH_CURVE_H
