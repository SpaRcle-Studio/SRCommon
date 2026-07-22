//
// Created by Monika on 19.01.2025.
//

#ifndef SR_ENGINE_UTILS_MATH_SIZE_H
#define SR_ENGINE_UTILS_MATH_SIZE_H

#include <Utils/Math/Vector2.h>
#include <Utils/Common/Enumerations.h>

namespace SR_MATH_NS {
    SR_ENUM_NS_CLASS_T(SizeMetric, uint8_t,
        Px, Percent
    );

    template<typename T> struct Size {
        using ValueType = T;
        static inline constexpr SizeType Dimensions() { return 1; }

        Size() noexcept
            : value(T())
        { }


        Size(T value, SizeMetric metric) noexcept
            : value(value)
            , metric(metric)
        { }

        union {
            struct {
                T value;
            };
            struct {
                T v;
            };
        };

        SizeMetric metric = SizeMetric::Px;

        SR_NODISCARD bool IsPixel() const noexcept { return metric == SizeMetric::Px; }
        SR_NODISCARD bool IsPercent() const noexcept { return metric == SizeMetric::Percent; }

        SR_NODISCARD T ToPixels() const noexcept { return v; }
        SR_NODISCARD T ToPixels(const T& parentSize) const noexcept {
            if (IsPixel()) {
                return v;
            }
            return (v / 100.f) * parentSize;
        }
    };

    template<typename T> struct Size2 {
        using ValueType = T;
        static inline constexpr SizeType Dimensions() { return 2; }

        Size2() noexcept
            : width(Size<T>())
            , height(Size<T>())
        { }

        union {
            struct {
                Size<T> width;
                Size<T> height;
            };
            struct {
                Size<T> w;
                Size<T> h;
            };
        };

        SR_NODISCARD bool HasPercent() const noexcept { return width.IsPercent() || height.IsPercent(); }

        SR_NODISCARD Vector2<T> ToPixels() const noexcept {
            return SR_MATH_NS::Vector2<T>(width.ToPixels(), height.ToPixels());
        }

        SR_NODISCARD Vector2<T> ToPixels(const Vector2<T>& parentSize) const noexcept {
            return SR_MATH_NS::Vector2<T>(width.ToPixels(parentSize.x), height.ToPixels(parentSize.y));
        }

        void SetPixels(const Vector2<T>& size) noexcept {
            width.v = size.x;
            height.v = size.y;
            width.metric = SizeMetric::Px;
            height.metric = SizeMetric::Px;
        }
    };

    using FSize = Size<float_t>;
    using ISize = Size<int32_t>;
    using USize = Size<uint32_t>;

    using FSize2 = Size2<float_t>;
    using ISize2 = Size2<int32_t>;
    using USize2 = Size2<uint32_t>;
}

#endif //SR_ENGINE_UTILS_MATH_SIZE_H
