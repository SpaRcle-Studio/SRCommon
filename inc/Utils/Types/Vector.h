//
// Created by Monika on 10.10.2024.
//

#ifndef SR_COMMON_VECTOR_H
#define SR_COMMON_VECTOR_H

#include <Utils/Common/AssertFwd.h>

#define SR_VECTOR_REALLOCATE_MULTIPLIER 2
#define SR_VECTOR_INITIAL_CAPACITY 4

namespace SR_UTILS_NS {
    template<typename T> class Vector {
    public:
        using SizeType = size_t;

        using value_type = T;
        using const_reference = const T&;
        using reference = T&;

        using Iterator = T*;
        using ConstIterator = const T*;

        using iterator = Iterator;
        using const_iterator = ConstIterator;

    public:
        SR_CONSTEXPR Vector() noexcept = default;
        SR_CONSTEXPR Vector(const Vector& other);
        SR_CONSTEXPR Vector(Vector&& other) noexcept;
        SR_CONSTEXPR Vector(std::initializer_list<T> init);

        SR_CONSTEXPR explicit Vector(SizeType count);
        SR_CONSTEXPR Vector(SizeType count, const T& value);

        SR_CONSTEXPR ~Vector();

    public:
        SR_CONSTEXPR Vector& operator=(const Vector& other);
        SR_CONSTEXPR Vector& operator=(Vector&& other) noexcept;
        SR_CONSTEXPR Vector& operator=(std::initializer_list<T> init);

    public:
        void reserve(SizeType newCapacity);
        void resize(SizeType newSize);
        void shrink_to_fit();
        void clear() noexcept;
        void swap(Vector& other) noexcept;
        void assign(SizeType count, const T& value);
        void pop_back();
        void push_back(const T& value);
        void push_back(T&& value);
        template <class... ValueType> T& emplace_back(ValueType&&... value);
        SR_NODISCARD ConstIterator find(const T& value) const noexcept;
        SR_NODISCARD Iterator find(const T& value) noexcept;
        Iterator erase(ConstIterator pos);
        Iterator erase(ConstIterator first, ConstIterator last);
        template <class... ValueType> Iterator insert(ConstIterator pos, ValueType&&... value);
        SR_NODISCARD T& at(SizeType index);
        SR_NODISCARD const T& at(SizeType index) const { return const_cast<Vector*>(this)->at(index); }

        SR_NODISCARD bool empty() const noexcept { return m_size == 0; }
        SR_NODISCARD SizeType size() const noexcept { return m_size; }
        SR_NODISCARD SizeType capacity() const noexcept { return m_capacity; }
        SR_NODISCARD static SizeType max_size() noexcept { return std::numeric_limits<SizeType>::max() / sizeof(T); }

        SR_NODISCARD Iterator begin() noexcept { return static_cast<T*>(m_data); }
        SR_NODISCARD Iterator end() noexcept { return static_cast<T*>(m_data) + m_size; }
        SR_NODISCARD ConstIterator begin() const noexcept { return static_cast<const T*>(m_data); }
        SR_NODISCARD ConstIterator end() const noexcept { return static_cast<const T*>(m_data) + m_size; }
        SR_NODISCARD ConstIterator cbegin() const noexcept { return static_cast<const T*>(m_data); }
        SR_NODISCARD ConstIterator cend() const noexcept { return static_cast<const T*>(m_data) + m_size; }

        SR_NODISCARD T& operator[](SizeType index) noexcept {
            SRAssert2(index < m_size, "Index {} is out of bounds! Size is {}!", index, m_size);
            return static_cast<T*>(m_data)[index];
        }
        SR_NODISCARD const T& operator[](SizeType index) const noexcept {
            SRAssert2(index < m_size, "Index {} is out of bounds! Size is {}!", index, m_size);
            return static_cast<const T*>(m_data)[index];
        }

        SR_NODISCARD T* data() noexcept { return static_cast<T*>(m_data); }
        SR_NODISCARD T& front() noexcept { return static_cast<T*>(m_data)[0]; }
        SR_NODISCARD T& back() noexcept { return static_cast<T*>(m_data)[m_size - 1]; }
        SR_NODISCARD const T* data() const noexcept { return static_cast<const T*>(m_data); }
        SR_NODISCARD const T& front() const noexcept { return static_cast<const T*>(m_data)[0]; }
        SR_NODISCARD const T& back() const noexcept { return static_cast<const T*>(m_data)[m_size - 1]; }

    private:
        void ConstructRange(SizeType start, SizeType end);
        void ConstructRange(SizeType start, SizeType end, const T& value);
        void DestructRange(SizeType start, SizeType end);

    private:
        T* m_data = nullptr;
        SizeType m_size = 0;
        SizeType m_capacity = 0;

    };

    template<typename T> template<class... ValueType> T& Vector<T>::emplace_back(ValueType &&... value) {
        if (m_size >= m_capacity) {
            reserve(m_capacity > 0 ? m_capacity * SR_VECTOR_REALLOCATE_MULTIPLIER : SR_VECTOR_INITIAL_CAPACITY);
        }

        if constexpr (std::is_trivially_copyable_v<T>) {
            T temp(std::forward<ValueType>(value)...);
            std::memcpy(static_cast<T*>(m_data) + m_size, &temp, sizeof(T));
        }
        else {
            new (static_cast<T*>(m_data) + m_size) T(std::forward<ValueType>(value)...);
        }
        ++m_size;
        return back();
    }

    template<typename T> SR_CONSTEXPR Vector<T>& Vector<T>::operator=(const Vector& other) {
        if (this != &other) {
            if (other.m_size > m_capacity) {
                Vector temp(other);
                swap(temp);
            }
            else {
                DestructRange(0, m_size);
                if (std::is_trivially_copyable_v<T>) {
                    std::memcpy(m_data, other.m_data, sizeof(T) * other.m_size);
                }
                else {
                    for (size_t i = 0; i < other.m_size; ++i) {
                        new (static_cast<T*>(m_data) + i) T(static_cast<const T*>(other.m_data)[i]);
                    }
                }
                m_size = other.m_size;
            }
        }
        return *this;
    }

    template<typename T> T& Vector<T>::at(SizeType index) {
        if (index >= m_size) {
            SRHalt("Vector::at() : index {} is out of bounds! Size is {}!", index, m_size);
            static T dummy{};
            return dummy;
        }
        return static_cast<T*>(m_data)[index];
    }

    template<typename T> Vector<T>::Iterator Vector<T>::erase(Vector::ConstIterator pos) {
        if (pos == end()) {
            return end();
        }

        SizeType index = pos - begin();
        T* data = static_cast<T*>(m_data);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(data + index, data + index + 1, sizeof(T) * (m_size - index - 1));
        }
        else {
            for (SizeType i = index; i < m_size - 1; ++i) {
                data[i] = std::move(data[i + 1]);
            }
            data[m_size - 1].~T();
        }

        --m_size;

        return data + index;
    }

    template<typename T> Vector<T>::Iterator Vector<T>::erase(Vector::ConstIterator first, Vector::ConstIterator last) {
        if (first == last) {
            return const_cast<Iterator>(first);
        }

        SizeType firstIndex = first - begin();
        SizeType lastIndex  = last - begin();

        SizeType count = lastIndex - firstIndex;

        T* data = static_cast<T*>(m_data);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(data + firstIndex, data + lastIndex, sizeof(T) * (m_size - lastIndex));
        }
        else {
            for (SizeType i = firstIndex; i < m_size - count; ++i) {
                data[i] = std::move(data[i + count]);
            }

            for (SizeType i = m_size - count; i < m_size; ++i) {
                data[i].~T();
            }
        }

        m_size -= count;

        return data + firstIndex;
    }

    template<typename T> template<class... ValueType> Vector<T>::Iterator Vector<T>::insert(Vector::ConstIterator pos, ValueType&&... value) {
        SizeType index = pos - begin();

        if (index > m_size) {
            return end();
        }

        if (m_size >= m_capacity) {
            reserve(m_capacity > 0 ? m_capacity * SR_VECTOR_REALLOCATE_MULTIPLIER : SR_VECTOR_INITIAL_CAPACITY);
        }

        T* data = static_cast<T*>(m_data);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(data + index + 1, data + index, sizeof(T) * (m_size - index));
            T temp(std::forward<ValueType>(value)...);
            std::memcpy(data + index, &temp, sizeof(T));
        }
        else {
            T temp(std::forward<ValueType>(value)...);
            for (SizeType i = m_size; i > index; --i) {
                new (data + i) T(std::move(data[i - 1]));
                data[i - 1].~T();
            }
            new (data + index) T(std::move(temp));
        }

        ++m_size;

        return data + index;
    }

    template<typename T> Vector<T>::ConstIterator Vector<T>::find(const T& value) const noexcept {
        for (SizeType i = 0; i < m_size; ++i) {
            if (static_cast<T*>(m_data)[i] == value) {
                return begin() + i;
            }
        }
        return end();
    }

    template<typename T> Vector<T>::Iterator Vector<T>::find(const T& value) noexcept {
        return const_cast<Iterator>(static_cast<const Vector*>(this)->find(value));
    }

    template<typename T> void Vector<T>::push_back(const T& value) {
        if (m_size >= m_capacity) {
            reserve(m_capacity > 0 ? m_capacity * SR_VECTOR_REALLOCATE_MULTIPLIER : SR_VECTOR_INITIAL_CAPACITY);
        }

        if (std::is_trivially_copyable_v<T>) {
            std::memcpy(static_cast<T*>(m_data) + m_size, &value, sizeof(T));
        }
        else {
            new (static_cast<T*>(m_data) + m_size) T(value);
        }
        ++m_size;
    }

    template<typename T> void Vector<T>::push_back(T&& value) {
        if (m_size >= m_capacity) {
            reserve(m_capacity > 0 ? m_capacity * SR_VECTOR_REALLOCATE_MULTIPLIER : SR_VECTOR_INITIAL_CAPACITY);
        }

        if (std::is_trivially_copyable_v<T>) {
            std::memcpy(static_cast<T*>(m_data) + m_size, &value, sizeof(T));
        }
        else {
            new (static_cast<T*>(m_data) + m_size) T(std::move(value));
        }
        ++m_size;
    }

    template<typename T> void Vector<T>::pop_back() {
        if (m_size > 0) {
            DestructRange(m_size - 1, m_size);
            --m_size;
        }
    }

    template<typename T> void Vector<T>::assign(SizeType count, const T& value) {
        if (count > m_capacity) {
            Vector temp(count, value);
            swap(temp);
        }
        else {
            DestructRange(0, m_size);
            ConstructRange(0, count, value);
            m_size = count;
        }
    }

    template<typename T> void Vector<T>::swap(Vector& other) noexcept {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    template<typename T> SR_CONSTEXPR Vector<T>& Vector<T>::operator=(std::initializer_list<T> init) {
        if (init.size() > m_capacity) {
            Vector temp(init);
            swap(temp);
        }
        else {
            DestructRange(0, m_size);
            if (std::is_trivially_copyable_v<T>) {
                std::memcpy(m_data, init.begin(), sizeof(T) * init.size());
            }
            else {
                for (size_t i = 0; i < init.size(); ++i) {
                    new (static_cast<T*>(m_data) + i) T(init.begin()[i]);
                }
            }
            m_size = init.size();
        }
        return *this;
    }

    template<typename T> void Vector<T>::clear() noexcept {
        if (m_data) {
            DestructRange(0, m_size);
            m_size = 0;
        }
    }

    template<typename T> SR_CONSTEXPR Vector<T>& Vector<T>::operator=(Vector&& other) noexcept {
        if (this != &other) {
            if (m_data) {
                DestructRange(0, m_size);
                SRFree(m_data);
            }

            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    template<typename T> SR_CONSTEXPR Vector<T>::Vector(std::initializer_list<T> init) {
        m_data = static_cast<T*>(SRMalloc(sizeof(T) * init.size()));
        m_size = init.size();
        m_capacity = init.size();

        if (std::is_trivially_copyable_v<T>) {
            std::memcpy(m_data, init.begin(), sizeof(T) * init.size());
        }
        else {
            for (size_t i = 0; i < init.size(); ++i) {
                new (static_cast<T*>(m_data) + i) T(init.begin()[i]);
            }
        }
    }

    template<typename T> SR_CONSTEXPR Vector<T>::Vector(const Vector& other) {
        m_data = static_cast<T*>(SRMalloc(sizeof(T) * other.m_size));
        m_size = other.m_size;
        m_capacity = other.m_size;
        if (std::is_trivially_copyable_v<T>) {
            std::memcpy(m_data, other.m_data, sizeof(T) * other.m_size);
        }
        else {
            for (size_t i = 0; i < other.m_size; ++i) {
                new (static_cast<T*>(m_data) + i) T(static_cast<T*>(other.m_data)[i]);
            }
        }
    }

    template<typename T> SR_CONSTEXPR Vector<T>::Vector(Vector&& other) noexcept {
        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;

        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    template<typename T> SR_CONSTEXPR Vector<T>::Vector(const SizeType count) {
        m_data = static_cast<T*>(SRMalloc(sizeof(T) * count));
        m_size = count;
        m_capacity = count;
        ConstructRange(0, count);
    }

    template<typename T> SR_CONSTEXPR Vector<T>::Vector(const SizeType count, const T& value) {
        m_data = static_cast<T*>(SRMalloc(sizeof(T) * count));
        m_size = count;
        m_capacity = count;
        ConstructRange(0, count, value);
    }

    template<typename T> SR_CONSTEXPR Vector<T>::~Vector() {
        if (m_data) {
            DestructRange(0, m_size);
            SRFree(m_data);
        }
    }

    template<typename T> void Vector<T>::DestructRange(SizeType start, SizeType end) {
        if (std::is_trivially_destructible_v<T>) {
            return;
        }
        for (SizeType i = start; i < end; ++i) {
            static_cast<T*>(m_data)[i].~T();
        }
    }

    template<typename T> void Vector<T>::ConstructRange(SizeType start, SizeType end) {
        if constexpr (std::is_trivially_default_constructible_v<T>) {
            std::memset(static_cast<T*>(m_data) + start, 0, sizeof(T) * (end - start));
            return;
        }
        for (SizeType i = start; i < end; ++i) {
            new (static_cast<T*>(m_data) + i) T();
        }
    }

    template<typename T> void Vector<T>::ConstructRange(SizeType start, SizeType end, const T& value) {
        if (std::is_trivially_copy_constructible_v<T>) {
            for (SizeType i = start; i < end; ++i) {
                std::memcpy(static_cast<T*>(m_data) + i, &value, sizeof(T));
            }
            return;
        }
        for (SizeType i = start; i < end; ++i) {
            new (static_cast<T*>(m_data) + i) T(value);
        }
    }

    template<typename T> void Vector<T>::shrink_to_fit() {
        if (m_size < m_capacity) {
            void* pOldData = m_data;
            m_data = static_cast<T*>(SRMalloc(sizeof(T) * m_size));
            m_capacity = m_size;

            if (pOldData) {
                if (std::is_trivially_copyable_v<T>) {
                    std::memcpy(m_data, pOldData, sizeof(T) * m_size);
                }
                else {
                    for (SizeType i = 0; i < m_size; ++i) {
                        new (static_cast<T*>(m_data) + i) T(std::move(static_cast<T*>(pOldData)[i]));
                        static_cast<T*>(pOldData)[i].~T();
                    }
                }
                SRFree(pOldData);
            }
        }
    }

    template<typename T> void Vector<T>::resize(const SizeType newSize) {
        if (newSize > m_size) {
            reserve(newSize);
            ConstructRange(m_size, newSize);
        }
        else if (newSize < m_size) {
            DestructRange(newSize, m_size);
        }
        m_size = newSize;
    }

    template<typename T> void Vector<T>::reserve(const SizeType newCapacity) {
        if (newCapacity > m_capacity) {
            void* pOldData = m_data;
            m_data = static_cast<T*>(SRMalloc(sizeof(T) * newCapacity));
            m_capacity = newCapacity;

            if (pOldData) {
                if (std::is_trivially_copyable_v<T>) {
                    std::memcpy(m_data, pOldData, sizeof(T) * m_size);
                }
                else {
                    for (SizeType i = 0; i < m_size; ++i) {
                        new (static_cast<T*>(m_data) + i) T(std::move(static_cast<T*>(pOldData)[i]));
                        static_cast<T*>(pOldData)[i].~T();
                    }
                }
                SRFree(pOldData);
            }
        }
    }
}

#endif //SR_COMMON_VECTOR_H
