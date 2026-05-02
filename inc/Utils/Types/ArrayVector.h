//
// Created by Monika on 31.01.2026.
//

#ifndef SR_ENGINE_COMMON_TYPES_ARRAY_VECTOR_H
#define SR_ENGINE_COMMON_TYPES_ARRAY_VECTOR_H

#include <Utils/Common/AssertFwd.h>

namespace SR_HTYPES_NS {
    template<typename T, size_t N> class SR_COMMON_DLL_API ArrayVector {
        using Iterator = T*;
        using ConstIterator = const T*;
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

        Iterator begin() noexcept { return &m_data[0]; }
        Iterator end() noexcept { return &m_data[0] + m_size; }
        ConstIterator begin() const noexcept { return &m_data[0]; }
        ConstIterator end() const noexcept { return &m_data[0] + m_size; }
        ConstIterator cbegin() const noexcept { return &m_data[0]; }
        ConstIterator cend() const noexcept { return &m_data[0] + m_size; }

        Iterator emplace_back() noexcept {
            SRAssert(m_size < N);
            m_data[m_size++] = T{};
            return begin() + m_size - 1;
        }

        Iterator emplace_back(const T& args) noexcept {
            SRAssert(m_size < N);
            m_data[m_size++] = args;
            return begin() + m_size - 1;
        }

        Iterator emplace_back(T&& args) noexcept {
            SRAssert(m_size < N);
            m_data[m_size++] = std::move(args);
            return begin() + m_size - 1;
        }

        Iterator push_back(const T& value) noexcept {
            SRAssert(m_size < N);
            m_data[m_size++] = value;
            return begin() + m_size - 1;
        }

        Iterator push_back(T&& value) noexcept {
            SRAssert(m_size < N);
            m_data[m_size++] = std::move(value);
            return begin() + m_size - 1;
        }

        void clear() noexcept {
            m_size = 0;
            T* pStart = &m_data[0];
            T* pEnd = pStart + N;
            for (T* pIt = pStart; pIt != pEnd; ++pIt) {
                 *pIt = T{};
            }
        }

    private:
        uint64_t m_size = 0;
        T m_data[N] = {};

    };
}

#endif //SR_ENGINE_COMMON_TYPES_ARRAY_VECTOR_H
