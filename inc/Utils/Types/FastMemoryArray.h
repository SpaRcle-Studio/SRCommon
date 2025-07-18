//
// Created by Monika on 27.06.2025.
//

#ifndef SR_ENGINE_COMMON_TYPES_FAST_MEMORY_ARRAY_H
#define SR_ENGINE_COMMON_TYPES_FAST_MEMORY_ARRAY_H

#include <Utils/Common/NonCopyable.h>

namespace SR_HTYPES_NS {
    template<typename T> class FastMemoryArray {
    public:
        using ValueType = T;

    public:
        FastMemoryArray() = default;

        FastMemoryArray(const FastMemoryArray& other) {
            SR_TRACY_ZONE;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            if (m_capacity > 0) {
                m_data = new T[m_capacity];
                memcpy(m_data, other.m_data, m_size * sizeof(T));
            }
        }

        FastMemoryArray(FastMemoryArray&& other) noexcept
            : m_size(other.m_size), m_capacity(other.m_capacity), m_data(other.m_data) {
            SR_TRACY_ZONE;
            other.m_size = 0;
            other.m_capacity = 0;
            other.m_data = nullptr;
        }

        FastMemoryArray& operator=(const FastMemoryArray& other) {
            SR_TRACY_ZONE;
            if (this != &other) {
                if (m_data) {
                    delete[] m_data;
                }
                m_size = other.m_size;
                m_capacity = other.m_capacity;
                if (m_capacity > 0) {
                    m_data = new T[m_capacity];
                    memcpy(m_data, other.m_data, m_size * sizeof(T));
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
                memcpy(m_data, other.data(), m_size * sizeof(T));
            } else {
                m_data = nullptr;
            }
        }

        FastMemoryArray& operator=(FastMemoryArray&& other) noexcept {
            SR_TRACY_ZONE;
            if (this != &other) {
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

        void shrink_to_fit() {
            SR_TRACY_ZONE;
            if (m_size < m_capacity) SR_UNLIKELY_ATTRIBUTE {
                T* pNewData = new T[m_size];
                if (m_data) {
                    memcpy(pNewData, m_data, m_size * sizeof(T));
                    delete[] m_data;
                }
                m_data = pNewData;
                m_capacity = m_size;
            }
        }

        T& back() {
            if (m_size == 0) SR_UNLIKELY_ATTRIBUTE {
                SR_PLATFORM_NS::WriteConsoleError("FastMemoryArray::back() : array is empty!");
                SR_UTILS_NS::Breakpoint();
            }
            return m_data[m_size - 1];
        }

        const T& back() const {
            if (m_size == 0) SR_UNLIKELY_ATTRIBUTE {
                SR_PLATFORM_NS::WriteConsoleError("FastMemoryArray::back() : array is empty!");
                SR_UTILS_NS::Breakpoint();
            }
            return m_data[m_size - 1];
        }

        void push_back(const T& value) {
            if (m_size >= m_capacity) SR_UNLIKELY_ATTRIBUTE {
                SizeType newCapacity = m_capacity == 0 ? 1 : m_capacity * 2;
                T* pNewData = new T[newCapacity];
                if (m_data) {
                    memcpy(pNewData, m_data, m_size * sizeof(T));
                    delete[] m_data;
                }
                m_data = pNewData;
                m_capacity = newCapacity;
            }
            m_data[m_size++] = value;
        }

        void push_back(T&& value) {
            if (m_size >= m_capacity) SR_UNLIKELY_ATTRIBUTE {
                SizeType newCapacity = m_capacity == 0 ? 1 : m_capacity * 2;
                T* pNewData = new T[newCapacity];
                if (m_data) {
                    memcpy(pNewData, m_data, m_size * sizeof(T));
                    delete[] m_data;
                }
                m_data = pNewData;
                m_capacity = newCapacity;
            }
            m_data[m_size++] = std::move(value);
        }

        void pop_back() {
            if (m_size > 0) SR_LIKELY_ATTRIBUTE {
                --m_size;
            }
        }

        void emplace_back(T&& value) {
            if (m_size >= m_capacity) SR_UNLIKELY_ATTRIBUTE {
                SizeType newCapacity = m_capacity == 0 ? 1 : m_capacity * 2;
                T* pNewData = new T[newCapacity];
                if (m_data) {
                    memcpy(pNewData, m_data, m_size * sizeof(T));
                    delete[] m_data;
                }
                m_data = pNewData;
                m_capacity = newCapacity;
            }
            m_data[m_size++] = std::move(value);
        }

        void emplace_back(const T& value) {
            if (m_size >= m_capacity) SR_UNLIKELY_ATTRIBUTE {
                SizeType newCapacity = m_capacity == 0 ? 1 : m_capacity * 2;
                T* pNewData = new T[newCapacity];
                if (m_data) {
                    memcpy(pNewData, m_data, m_size * sizeof(T));
                    delete[] m_data;
                }
                m_data = pNewData;
                m_capacity = newCapacity;
            }
            m_data[m_size++] = value;
        }

        void resize(SizeType newSize) {
            SR_TRACY_ZONE;
            if (newSize > m_capacity) {
                T* pNewData = new T[newSize];
                if (m_data) {
                    memcpy(pNewData, m_data, m_size * sizeof(T));
                    delete[] m_data;
                }
                m_data = pNewData;
                m_capacity = newSize;
            }
            m_size = newSize;
        }

        void reserve(SizeType newCapacity) {
            SR_TRACY_ZONE;
            if (newCapacity > m_capacity) {
                T* pNewData = new T[newCapacity];
                if (m_data) {
                    memcpy(pNewData, m_data, m_size * sizeof(T));
                    delete[] m_data;
                }
                m_data = pNewData;
                m_capacity = newCapacity;
            }
        }

    private:
        SizeType m_size = 0;
        SizeType m_capacity = 0;
        T* m_data = nullptr;

    };
}

#endif //SR_ENGINE_COMMON_TYPES_FAST_MEMORY_ARRAY_H
