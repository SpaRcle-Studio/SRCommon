//
// Created by Monika on 31.01.2026.
//

#ifndef SR_ENGINE_COMMON_TYPES_ARRAY_VECTOR_H
#define SR_ENGINE_COMMON_TYPES_ARRAY_VECTOR_H

#include <Utils/Debug.h>

namespace SR_HTYPES_NS {
    template<typename T, size_t N> class ArrayVector {
        using Iterator = typename std::array<T, N>::iterator;
        using ConstIterator = typename std::array<T, N>::const_iterator;
    public:
        SR_NODISCARD T& operator[](size_t index) {
            SRAssert(index < m_size);
            return m_data[index];
        }

        SR_NODISCARD const T& operator[](size_t index) const {
            SRAssert(index < m_size);
            return m_data[index];
        }

        SR_NODISCARD size_t size() const noexcept { return m_size; }
        SR_NODISCARD size_t capacity() const noexcept { return N; }
        SR_NODISCARD bool empty() const noexcept { return m_size == 0; }

        Iterator begin() noexcept { return m_data.begin(); }
        Iterator end() noexcept { return m_data.begin() + m_size; }
        ConstIterator begin() const noexcept { return m_data.begin(); }
        ConstIterator end() const noexcept { return m_data.begin() + m_size; }
        ConstIterator cbegin() const noexcept { return m_data.cbegin(); }
        ConstIterator cend() const noexcept { return m_data.cbegin() + m_size; }

        Iterator emplace_back() noexcept {
            SRAssert(m_size < N);
            m_data[m_size++] = T{};
            return m_data.begin() + m_size - 1;
        }

        Iterator emplace_back(const T& args) noexcept {
            SRAssert(m_size < N);
            m_data[m_size++] = args;
            return m_data.begin() + m_size - 1;
        }

        Iterator emplace_back(T&& args) noexcept {
            SRAssert(m_size < N);
            m_data[m_size++] = std::move(args);
            return m_data.begin() + m_size - 1;
        }

        Iterator push_back(const T& value) noexcept {
            SRAssert(m_size < N);
            m_data[m_size++] = value;
            return m_data.begin() + m_size - 1;
        }

        Iterator push_back(T&& value) noexcept {
            SRAssert(m_size < N);
            m_data[m_size++] = std::move(value);
            return m_data.begin() + m_size - 1;
        }

        void clear() noexcept {
            m_size = 0;
            for (auto& item : m_data) {
                item = T{};
            }
        }

    private:
        uint64_t m_size = 0;
        std::array<T, N> m_data{};

    };
}

#endif //SR_ENGINE_COMMON_TYPES_ARRAY_VECTOR_H
