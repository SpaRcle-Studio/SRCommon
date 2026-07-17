//
// Created by Monika on 16.06.2026.
//

#ifndef SR_ENGINE_COMMON_TYPES_SMALL_VECTOR_H
#define SR_ENGINE_COMMON_TYPES_SMALL_VECTOR_H

#include <Utils/Common/AssertFwd.h>

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

namespace SR_UTILS_NS {
    template<typename T, size_t InlineCapacity> class SmallVector {
        static_assert(InlineCapacity > 0, "SmallVector InlineCapacity must be > 0");

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
        SR_CONSTEXPR SmallVector() noexcept = default;
        SR_CONSTEXPR SmallVector(const SmallVector& other);
        SR_CONSTEXPR SmallVector(SmallVector&& other) noexcept;
        SR_CONSTEXPR SmallVector(std::initializer_list<T> init);

        SR_CONSTEXPR explicit SmallVector(SizeType count);
        SR_CONSTEXPR SmallVector(SizeType count, const T& value);

        SR_CONSTEXPR ~SmallVector();

    public:
        SR_CONSTEXPR SmallVector& operator=(const SmallVector& other);
        SR_CONSTEXPR SmallVector& operator=(SmallVector&& other) noexcept;
        SR_CONSTEXPR SmallVector& operator=(std::initializer_list<T> init);

    public:
        void reserve(SizeType newCapacity);
        void resize(SizeType newSize);
        void shrink_to_fit();
        void clear() noexcept;
        void swap(SmallVector& other) noexcept;
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
        SR_NODISCARD const T& at(SizeType index) const { return const_cast<SmallVector*>(this)->at(index); }

        SR_NODISCARD bool empty() const noexcept { return m_size == 0; }
        SR_NODISCARD SizeType size() const noexcept { return m_size; }
        SR_NODISCARD SizeType capacity() const noexcept { return m_capacity; }
        SR_NODISCARD static SizeType max_size() noexcept { return std::numeric_limits<SizeType>::max() / sizeof(T); }
        SR_NODISCARD static constexpr SizeType inline_capacity() noexcept { return InlineCapacity; }

        SR_NODISCARD Iterator begin() noexcept { return m_data; }
        SR_NODISCARD Iterator end() noexcept { return m_data + m_size; }
        SR_NODISCARD ConstIterator begin() const noexcept { return m_data; }
        SR_NODISCARD ConstIterator end() const noexcept { return m_data + m_size; }
        SR_NODISCARD ConstIterator cbegin() const noexcept { return m_data; }
        SR_NODISCARD ConstIterator cend() const noexcept { return m_data + m_size; }

        SR_NODISCARD T& operator[](SizeType index) noexcept {
            SRAssert2(index < m_size, "Index {} is out of bounds! Size is {}!", index, m_size);
            return m_data[index];
        }
        SR_NODISCARD const T& operator[](SizeType index) const noexcept {
            SRAssert2(index < m_size, "Index {} is out of bounds! Size is {}!", index, m_size);
            return m_data[index];
        }

        SR_NODISCARD T* data() noexcept { return m_data; }
        SR_NODISCARD T& front() noexcept { return m_data[0]; }
        SR_NODISCARD T& back() noexcept { return m_data[m_size - 1]; }
        SR_NODISCARD const T* data() const noexcept { return m_data; }
        SR_NODISCARD const T& front() const noexcept { return m_data[0]; }
        SR_NODISCARD const T& back() const noexcept { return m_data[m_size - 1]; }

        SR_NODISCARD bool is_small() const noexcept { return m_data == InlineData(); }

    private:
        SR_NODISCARD SR_CONSTEXPR T* InlineData() noexcept {
            return reinterpret_cast<T*>(m_inline);
        }
        SR_NODISCARD SR_CONSTEXPR const T* InlineData() const noexcept {
            return reinterpret_cast<const T*>(m_inline);
        }

        void ConstructRange(T* pData, SizeType start, SizeType end);
        void ConstructRange(T* pData, SizeType start, SizeType end, const T& value);
        void DestructRange(T* pData, SizeType start, SizeType end);

        void EnsureCapacityForGrowth(SizeType additionalCount);
        void ReallocateTo(SizeType newCapacity);
        void MoveOrCopyTo(T* pDst, const T* pSrc, SizeType count);

    private:
        alignas(T) unsigned char m_inline[sizeof(T) * InlineCapacity]{};
        T* m_data = reinterpret_cast<T*>(m_inline);
        SizeType m_size = 0;
        SizeType m_capacity = InlineCapacity;
    };

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::MoveOrCopyTo(T* pDst, const T* pSrc, SizeType count) {
        if (count == 0) {
            return;
        }

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)pDst, (void*)pSrc, sizeof(T) * count);
        }
        else {
            for (SizeType i = 0; i < count; ++i) {
                new (pDst + i) T(std::move(const_cast<T&>(pSrc[i])));
            }
        }
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::DestructRange(T* pData, SizeType start, SizeType end) {
        if constexpr (std::is_trivially_destructible_v<T>) {
            return;
        }
        for (SizeType i = start; i < end; ++i) {
            pData[i].~T();
        }
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::ConstructRange(T* pData, SizeType start, SizeType end) {
        if constexpr (std::is_trivially_default_constructible_v<T>) {
            std::memset(pData + start, 0, sizeof(T) * (end - start));
            return;
        }
        for (SizeType i = start; i < end; ++i) {
            new (pData + i) T();
        }
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::ConstructRange(T* pData, SizeType start, SizeType end, const T& value) {
        if constexpr (std::is_trivially_copy_constructible_v<T>) {
            for (SizeType i = start; i < end; ++i) {
                std::memcpy((void*)(pData + i), (void*)(&value), sizeof(T));
            }
            return;
        }
        for (SizeType i = start; i < end; ++i) {
            new (pData + i) T(value);
        }
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::ReallocateTo(SizeType newCapacity) {
        if (newCapacity <= InlineCapacity) {
            if (!is_small()) {
                T* pOld = m_data;
                T* pInline = InlineData();

                if constexpr (std::is_trivially_copyable_v<T>) {
                    std::memcpy((void*)pInline, (void*)pOld, sizeof(T) * m_size);
                }
                else {
                    for (SizeType i = 0; i < m_size; ++i) {
                        new (pInline + i) T(std::move(pOld[i]));
                        pOld[i].~T();
                    }
                }

                SRFree(pOld);
                m_data = pInline;
                m_capacity = InlineCapacity;
            }
            return;
        }

        T* pNew = static_cast<T*>(SRMalloc(sizeof(T) * newCapacity));
        T* pOld = m_data;

        if (pOld) {
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy((void*)pNew, (void*)pOld, sizeof(T) * m_size);
            }
            else {
                for (SizeType i = 0; i < m_size; ++i) {
                    new (pNew + i) T(std::move(pOld[i]));
                }
                DestructRange(pOld, 0, m_size);
            }

            if (!is_small()) {
                SRFree(pOld);
            }
        }

        m_data = pNew;
        m_capacity = newCapacity;
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::EnsureCapacityForGrowth(SizeType additionalCount) {
        if (additionalCount == 0) {
            return;
        }

        const SizeType required = m_size + additionalCount;
        if (required <= m_capacity) {
            return;
        }

        SizeType newCap = m_capacity > 0 ? (m_capacity * 2) : InlineCapacity;
        if (newCap < required) {
            newCap = required;
        }
        if (newCap < InlineCapacity) {
            newCap = InlineCapacity;
        }

        reserve(newCap);
    }

    template<typename T, size_t InlineCapacity>
    SR_CONSTEXPR SmallVector<T, InlineCapacity>::SmallVector(std::initializer_list<T> init) {
        reserve(static_cast<SizeType>(init.size()));
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)m_data, (void*)init.begin(), sizeof(T) * init.size());
        }
        else {
            for (SizeType i = 0; i < init.size(); ++i) {
                new (m_data + i) T(init.begin()[i]);
            }
        }
        m_size = static_cast<SizeType>(init.size());
    }

    template<typename T, size_t InlineCapacity>
    SR_CONSTEXPR SmallVector<T, InlineCapacity>::SmallVector(const SmallVector& other) {
        reserve(other.m_size);
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)m_data, (void*)other.m_data, sizeof(T) * other.m_size);
        }
        else {
            for (SizeType i = 0; i < other.m_size; ++i) {
                new (m_data + i) T(other.m_data[i]);
            }
        }
        m_size = other.m_size;
    }

    template<typename T, size_t InlineCapacity>
    SR_CONSTEXPR SmallVector<T, InlineCapacity>::SmallVector(SmallVector&& other) noexcept {
        if (!other.is_small()) {
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            other.m_data = other.InlineData();
            other.m_size = 0;
            other.m_capacity = InlineCapacity;
        }
        else {
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy((void*)m_data, (void*)other.m_data, sizeof(T) * other.m_size);
            }
            else {
                for (SizeType i = 0; i < other.m_size; ++i) {
                    new (m_data + i) T(std::move(other.m_data[i]));
                    other.m_data[i].~T();
                }
            }
            m_size = other.m_size;
            other.m_size = 0;
        }
    }

    template<typename T, size_t InlineCapacity>
    SR_CONSTEXPR SmallVector<T, InlineCapacity>::SmallVector(const SizeType count) {
        reserve(count);
        ConstructRange(m_data, 0, count);
        m_size = count;
    }

    template<typename T, size_t InlineCapacity>
    SR_CONSTEXPR SmallVector<T, InlineCapacity>::SmallVector(const SizeType count, const T& value) {
        reserve(count);
        ConstructRange(m_data, 0, count, value);
        m_size = count;
    }

    template<typename T, size_t InlineCapacity>
    SR_CONSTEXPR SmallVector<T, InlineCapacity>::~SmallVector() {
        if (m_data) {
            DestructRange(m_data, 0, m_size);
            if (!is_small()) {
                SRFree(m_data);
            }
        }
    }

    template<typename T, size_t InlineCapacity>
    SR_CONSTEXPR SmallVector<T, InlineCapacity>& SmallVector<T, InlineCapacity>::operator=(const SmallVector& other) {
        if (this == &other) {
            return *this;
        }

        if (other.m_size > m_capacity) {
            SmallVector temp(other);
            swap(temp);
            return *this;
        }

        DestructRange(m_data, 0, m_size);
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)m_data, (void*)other.m_data, sizeof(T) * other.m_size);
        }
        else {
            for (SizeType i = 0; i < other.m_size; ++i) {
                new (m_data + i) T(other.m_data[i]);
            }
        }

        m_size = other.m_size;
        return *this;
    }

    template<typename T, size_t InlineCapacity>
    SR_CONSTEXPR SmallVector<T, InlineCapacity>& SmallVector<T, InlineCapacity>::operator=(SmallVector&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (!other.is_small()) {
            DestructRange(m_data, 0, m_size);
            if (!is_small()) {
                SRFree(m_data);
            }

            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            other.m_data = other.InlineData();
            other.m_size = 0;
            other.m_capacity = InlineCapacity;

            return *this;
        }

        if (other.m_size > m_capacity) {
            reserve(other.m_size);
        }

        DestructRange(m_data, 0, m_size);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)m_data, (void*)other.m_data, sizeof(T) * other.m_size);
        }
        else {
            for (SizeType i = 0; i < other.m_size; ++i) {
                new (m_data + i) T(std::move(other.m_data[i]));
                other.m_data[i].~T();
            }
        }

        m_size = other.m_size;
        other.m_size = 0;
        return *this;
    }

    template<typename T, size_t InlineCapacity>
    SR_CONSTEXPR SmallVector<T, InlineCapacity>& SmallVector<T, InlineCapacity>::operator=(std::initializer_list<T> init) {
        if (init.size() > m_capacity) {
            SmallVector temp(init);
            swap(temp);
            return *this;
        }

        DestructRange(m_data, 0, m_size);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)m_data, (void*)init.begin(), sizeof(T) * init.size());
        }
        else {
            for (SizeType i = 0; i < init.size(); ++i) {
                new (m_data + i) T(init.begin()[i]);
            }
        }

        m_size = static_cast<SizeType>(init.size());
        return *this;
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::reserve(const SizeType newCapacity) {
        if (newCapacity <= m_capacity) {
            return;
        }
        ReallocateTo(newCapacity);
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::resize(const SizeType newSize) {
        if (newSize > m_size) {
            reserve(newSize);
            ConstructRange(m_data, m_size, newSize);
        }
        else if (newSize < m_size) {
            DestructRange(m_data, newSize, m_size);
        }
        m_size = newSize;
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::shrink_to_fit() {
        if (m_size == m_capacity) {
            return;
        }

        if (m_size <= InlineCapacity) {
            ReallocateTo(InlineCapacity);
            return;
        }

        ReallocateTo(m_size);
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::clear() noexcept {
        if (m_data) {
            DestructRange(m_data, 0, m_size);
            m_size = 0;
        }
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::swap(SmallVector& other) noexcept {
        if (this == &other) {
            return;
        }

        if (!is_small() && !other.is_small()) {
            std::swap(m_data, other.m_data);
            std::swap(m_size, other.m_size);
            std::swap(m_capacity, other.m_capacity);
            return;
        }

        SmallVector temp(std::move(*this));
        *this = std::move(other);
        other = std::move(temp);
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::assign(SizeType count, const T& value) {
        if (count > m_capacity) {
            SmallVector temp(count, value);
            swap(temp);
            return;
        }

        DestructRange(m_data, 0, m_size);
        ConstructRange(m_data, 0, count, value);
        m_size = count;
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::pop_back() {
        if (m_size > 0) {
            DestructRange(m_data, m_size - 1, m_size);
            --m_size;
        }
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::push_back(const T& value) {
        EnsureCapacityForGrowth(1);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)(m_data + m_size), (void*)(&value), sizeof(T));
        }
        else {
            new (m_data + m_size) T(value);
        }
        ++m_size;
    }

    template<typename T, size_t InlineCapacity>
    void SmallVector<T, InlineCapacity>::push_back(T&& value) {
        EnsureCapacityForGrowth(1);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)(m_data + m_size), (void*)(&value), sizeof(T));
        }
        else {
            new (m_data + m_size) T(std::move(value));
        }
        ++m_size;
    }

    template<typename T, size_t InlineCapacity>
    template<class... ValueType>
    T& SmallVector<T, InlineCapacity>::emplace_back(ValueType&&... value) {
        EnsureCapacityForGrowth(1);

        if constexpr (std::is_trivially_copyable_v<T>) {
            T temp(std::forward<ValueType>(value)...);
            std::memcpy((void*)(m_data + m_size), (void*)(&temp), sizeof(T));
        }
        else {
            new (m_data + m_size) T(std::forward<ValueType>(value)...);
        }
        ++m_size;
        return back();
    }

    template<typename T, size_t InlineCapacity>
    typename SmallVector<T, InlineCapacity>::ConstIterator
    SmallVector<T, InlineCapacity>::find(const T& value) const noexcept {
        for (SizeType i = 0; i < m_size; ++i) {
            if (m_data[i] == value) {
                return begin() + i;
            }
        }
        return end();
    }

    template<typename T, size_t InlineCapacity>
    typename SmallVector<T, InlineCapacity>::Iterator
    SmallVector<T, InlineCapacity>::find(const T& value) noexcept {
        return const_cast<Iterator>(static_cast<const SmallVector*>(this)->find(value));
    }

    template<typename T, size_t InlineCapacity>
    T& SmallVector<T, InlineCapacity>::at(SizeType index) {
        if (index >= m_size) {
            SRHalt("SmallVector::at() : index {} is out of bounds! Size is {}!", index, m_size);
            static T dummy{};
            return dummy;
        }
        return m_data[index];
    }

    template<typename T, size_t InlineCapacity>
    typename SmallVector<T, InlineCapacity>::Iterator
    SmallVector<T, InlineCapacity>::erase(ConstIterator pos) {
        if (pos == end()) {
            return end();
        }

        SizeType index = static_cast<SizeType>(pos - begin());

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(m_data + index, m_data + index + 1, sizeof(T) * (m_size - index - 1));
        }
        else {
            for (SizeType i = index; i < m_size - 1; ++i) {
                m_data[i] = std::move(m_data[i + 1]);
            }
            m_data[m_size - 1].~T();
        }

        --m_size;
        return m_data + index;
    }

    template<typename T, size_t InlineCapacity>
    typename SmallVector<T, InlineCapacity>::Iterator
    SmallVector<T, InlineCapacity>::erase(ConstIterator first, ConstIterator last) {
        if (first == last) {
            return const_cast<Iterator>(first);
        }

        SizeType firstIndex = static_cast<SizeType>(first - begin());
        SizeType lastIndex  = static_cast<SizeType>(last - begin());
        SizeType count = lastIndex - firstIndex;

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(m_data + firstIndex, m_data + lastIndex, sizeof(T) * (m_size - lastIndex));
        }
        else {
            for (SizeType i = firstIndex; i < m_size - count; ++i) {
                m_data[i] = std::move(m_data[i + count]);
            }
            for (SizeType i = m_size - count; i < m_size; ++i) {
                m_data[i].~T();
            }
        }

        m_size -= count;
        return m_data + firstIndex;
    }

    template<typename T, size_t InlineCapacity>
    template<class... ValueType>
    typename SmallVector<T, InlineCapacity>::Iterator
    SmallVector<T, InlineCapacity>::insert(ConstIterator pos, ValueType&&... value) {
        SizeType index = static_cast<SizeType>(pos - begin());

        if (index > m_size) {
            return end();
        }

        EnsureCapacityForGrowth(1);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(m_data + index + 1, m_data + index, sizeof(T) * (m_size - index));
            T temp(std::forward<ValueType>(value)...);
            std::memcpy((void*)(m_data + index), (void*)(&temp), sizeof(T));
        }
        else {
            T temp(std::forward<ValueType>(value)...);
            for (SizeType i = m_size; i > index; --i) {
                new (m_data + i) T(std::move(m_data[i - 1]));
                m_data[i - 1].~T();
            }
            new (m_data + index) T(std::move(temp));
        }

        ++m_size;
        return m_data + index;
    }
}

#endif //SR_ENGINE_COMMON_TYPES_SMALL_VECTOR_H
