//
// Created by Monika on 27.06.2025.
//

#ifndef SR_ENGINE_COMMON_MATH_VECTOR_RANGES_H
#define SR_ENGINE_COMMON_MATH_VECTOR_RANGES_H

#include <Utils/Math/Vector3.h>

namespace SR_MATH_NS {
    class UVector3Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = SR_MATH_NS::UVector3;
        using difference_type = std::ptrdiff_t;
        using pointer = SR_MATH_NS::UVector3 *;
        using reference = SR_MATH_NS::UVector3;

        UVector3Iterator()
            : m_max(SR_MATH_NS::UVector3(0, 0, 0)), m_index(0)
        { }

        UVector3Iterator(SR_MATH_NS::UVector3 max, std::size_t index)
            : m_max(max), m_index(index)
        { }

        SR_MATH_NS::UVector3 operator*() const {
            uint32_t x = m_index % m_max.x;
            uint32_t y = (m_index / m_max.x) % m_max.y;
            uint32_t z = m_index / (m_max.x * m_max.y);
            return SR_MATH_NS::UVector3{x, y, z};
        }

        UVector3Iterator &operator++() { ++m_index; return *this; }
        UVector3Iterator operator++(int) { UVector3Iterator tmp = *this; ++(*this); return tmp; }
        UVector3Iterator &operator--() { --m_index; return *this; }
        UVector3Iterator operator--(int) { UVector3Iterator tmp = *this; --(*this); return tmp; }
        UVector3Iterator &operator+=(difference_type n) { m_index += n; return *this; }
        UVector3Iterator &operator-=(difference_type n) { m_index -= n; return *this; }
        UVector3Iterator operator+(difference_type n) const { return UVector3Iterator(m_max, m_index + n); }
        UVector3Iterator operator-(difference_type n) const { return UVector3Iterator(m_max, m_index - n); }
        difference_type operator-(const UVector3Iterator &other) const { return m_index - other.m_index; }
        SR_MATH_NS::UVector3 operator[](difference_type n) const { return *(*this + n); }
        bool operator==(const UVector3Iterator &other) const { return m_index == other.m_index; }
        bool operator!=(const UVector3Iterator &other) const { return m_index != other.m_index; }
        bool operator<(const UVector3Iterator &other) const { return m_index < other.m_index; }
        bool operator>(const UVector3Iterator &other) const { return m_index > other.m_index; }
        bool operator<=(const UVector3Iterator &other) const { return m_index <= other.m_index; }
        bool operator>=(const UVector3Iterator &other) const { return m_index >= other.m_index; }

    private:
        SR_MATH_NS::UVector3 m_max;
        std::size_t m_index;

    };

    class UVector3Range {
    public:
        UVector3Range(uint32_t minX, uint32_t minY, uint32_t minZ, uint32_t maxX, uint32_t maxY, uint32_t maxZ)
            : min(minX, minY, minZ)
            , max(maxX, maxY, maxZ)
        { }

        UVector3Range(uint32_t maxX, uint32_t maxY, uint32_t maxZ)
            : min(0, 0, 0)
            , max(maxX, maxY, maxZ)
        { }

        SR_NODISCARD UVector3Iterator begin() const {
            return UVector3Iterator(max, min.x * min.y * min.z);
        }

        SR_NODISCARD UVector3Iterator end() const {
            return UVector3Iterator(max, max.x * max.y * max.z);
        }

    private:
        SR_MATH_NS::UVector3 min;
        SR_MATH_NS::UVector3 max;

    };
}

#endif //SR_ENGINE_COMMON_MATH_VECTOR_RANGES_H
