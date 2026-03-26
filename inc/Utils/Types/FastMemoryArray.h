//
// Created by Monika on 27.06.2025.
//

#ifndef SR_ENGINE_COMMON_TYPES_FAST_MEMORY_ARRAY_H
#define SR_ENGINE_COMMON_TYPES_FAST_MEMORY_ARRAY_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/Common/AssertFwd.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_HTYPES_NS {
    template<typename T, bool FastMode = true> class FastMemoryArray {
    public:
        using ValueType = T;

    public:
        using Iterator = T*;
        using ConstIterator = const T*;
        using SizeType = size_t;

        FastMemoryArray() = default;

        FastMemoryArray(const FastMemoryArray& other) {
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            if (m_capacity > 0) {
                SR_TRACY_ZONE;
                m_data = new T[m_capacity];
                CopyData(m_data, other.m_data);
            }
        }

        FastMemoryArray(FastMemoryArray&& other) noexcept
            : m_size(other.m_size), m_capacity(other.m_capacity), m_data(other.m_data) {
            other.m_size = 0;
            other.m_capacity = 0;
            other.m_data = nullptr;
        }

        FastMemoryArray& operator=(const FastMemoryArray& other) {
            if (this != &other) {
                SR_TRACY_ZONE;
                if (m_data) {
                    delete[] m_data;
                }
                m_size = other.m_size;
                m_capacity = other.m_capacity;
                if (m_capacity > 0) {
                    m_data = new T[m_capacity];
                    CopyData(m_data, other.m_data);
                } else {
                    m_data = nullptr;
                }
            }
            return *this;
        }

        FastMemoryArray(const std::vector<T>& other) {
            SR_TRACY_ZONE;
            m_size = other.size();
            m_capacity = m_size;
            if (m_capacity > 0) {
                m_data = new T[m_capacity];
                CopyData(m_data, other.data());
            } else {
                m_data = nullptr;
            }
        }

        FastMemoryArray& operator=(FastMemoryArray&& other) noexcept {
            if (this != &other) {
                SR_TRACY_ZONE;
                if (m_data) {
                    delete[] m_data;
                }
                m_size = other.m_size;
                m_capacity = other.m_capacity;
                m_data = other.m_data;
                other.m_size = 0;
                other.m_capacity = 0;
                other.m_data = nullptr;
            }
            return *this;
        }

        ~FastMemoryArray() {
            if (m_data) {
                delete[] m_data;
            }
        }

        void Destroy() noexcept {
            SR_TRACY_ZONE;
            if (m_data) {
                delete[] m_data;
                m_data = nullptr;
            }
            m_size = 0;
            m_capacity = 0;
        }

        SR_NODISCARD Iterator begin() noexcept { return m_data; }
        SR_NODISCARD Iterator end() noexcept { return m_data + m_size; }
        SR_NODISCARD ConstIterator begin() const noexcept { return m_data; }
        SR_NODISCARD ConstIterator end() const noexcept { return m_data + m_size; }

        Iterator erase_verify(Iterator pos) noexcept {
            if (pos < begin() || pos >= end()) SR_UNLIKELY_ATTRIBUTE {
                SRHalt("FastMemoryArray::erase() : iterator out of range!");
                return end();
            }
            return EraseNoVerify(pos);
        }

        Iterator erase(Iterator pos) noexcept {
            const SizeType index = static_cast<SizeType>(pos - begin());
            if (index != m_size - 1) {
                if constexpr (FastMode) {
                    std::memmove(&m_data[index], &m_data[index + 1], (m_size - index - 1) * sizeof(T));
                }
                else {
                    for (SizeType i = index; i < m_size - 1; ++i) {
                        m_data[i] = std::move(m_data[i + 1]);
                    }
                }
            }

            if constexpr (!FastMode) {
                m_data[m_size - 1].~T();
            }

            --m_size;
            return begin() + index;
        }

        SR_NODISCARD SizeType size() const noexcept { return m_size; }
        SR_NODISCARD SizeType capacity() const noexcept { return m_capacity; }
        SR_NODISCARD bool empty() const noexcept { return m_size == 0; }
        SR_NODISCARD T* data() noexcept { return m_data; }
        SR_NODISCARD const T* data() const noexcept { return m_data; }
        SR_NODISCARD T& operator[](SizeType index) noexcept { return m_data[index]; }
        SR_NODISCARD const T& operator[](SizeType index) const noexcept { return m_data[index]; }

        void FillZero() noexcept {
            SR_TRACY_ZONE;
            if (m_data) {
                memset(m_data, 0, m_size * sizeof(T));
            }
        }

        void FillInt(int32_t value) noexcept {
            SR_TRACY_ZONE;
            if (m_data) {
                memset(m_data, value, m_size * sizeof(T));
            }
        }

        void clear() noexcept { m_size = 0; }

        T& back() {
            if (m_size == 0) SR_UNLIKELY_ATTRIBUTE {
                SRHalt("FastMemoryArray::back() : array is empty!");
            }
            return m_data[m_size - 1];
        }

        const T& back() const {
            if (m_size == 0) SR_UNLIKELY_ATTRIBUTE {
                SRHalt("FastMemoryArray::back() : array is empty!");
            }
            return m_data[m_size - 1];
        }

        void push_back(const T& value) {
            CheckOverflow();
            m_data[m_size++] = value;
        }

        void push_back(T&& value) {
            CheckOverflow();
            m_data[m_size++] = std::move(value);
        }

        void pop_back() {
            if (m_size > 0) SR_LIKELY_ATTRIBUTE {
                --m_size;
            }
        }

        void emplace_back(T&& value) {
            CheckOverflow();
            m_data[m_size++] = std::move(value);
        }

        void emplace_back(const T& value) {
            CheckOverflow();
            m_data[m_size++] = value;
        }

        void resize(SizeType newSize) {
            SR_TRACY_ZONE;
            if (newSize > m_capacity) {
                m_capacity = newSize;
                Reallocate();
            }
            m_size = newSize;
        }

        void fill(const T& value) {
            SR_TRACY_ZONE;
            for (SizeType i = 0; i < m_size; ++i) {
                m_data[i] = value;
            }
        }

        void reserve(SizeType newCapacity) {
            SR_TRACY_ZONE;
            if (newCapacity > m_capacity) {
                m_capacity = newCapacity;
                Reallocate();
            }
        }

        void shrink_to_fit() {
            SR_TRACY_ZONE;
            if (m_size < m_capacity) SR_UNLIKELY_ATTRIBUTE {
                m_capacity = m_size;
                Reallocate();
            }
        }

    private:
        void CheckOverflow() {
            if (m_size >= m_capacity) SR_UNLIKELY_ATTRIBUTE {
                m_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
                Reallocate();
            }
        }

        void Reallocate() {
            SR_TRACY_ZONE;
            T* pNewData = new T[m_capacity];
            if (m_data) {
                CopyData(pNewData, m_data);
                delete[] m_data;
            }
            m_data = pNewData;
        }

        void CopyData(T* pDest, const T* pSrc) {
            if (m_size > 0) SR_LIKELY_ATTRIBUTE {
                memcpy(pDest, pSrc, m_size * sizeof(T));
            }
        }

    private:
        SizeType m_size = 0;
        SizeType m_capacity = 0;
        T* m_data = nullptr;

    };
}

#endif //SR_ENGINE_COMMON_TYPES_FAST_MEMORY_ARRAY_H
