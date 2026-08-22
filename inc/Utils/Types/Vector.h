//
// Created by Monika on 10.10.2024.
//

#if !defined(SR_COMMON_VECTOR_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
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
        using pointer = Iterator;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

    public:
        Vector() noexcept = default;
        Vector(std::nullptr_t) = delete;
        Vector(IAllocator* allocator) noexcept;
        Vector(const Vector& other);
        Vector(Vector&& other) noexcept;
        Vector(std::initializer_list<T> init);
        Vector(ConstIterator first, ConstIterator last);
        Vector(IAllocator* pAllocator, ConstIterator first, ConstIterator last);

        explicit Vector(SizeType count);
        Vector(SizeType count, const T& value);

        ~Vector();

    public:
        Vector& operator=(const Vector& other);
        Vector& operator=(Vector&& other) noexcept;
        Vector& operator=(std::initializer_list<T> init);

    public:
        void delete_contents();

        SR_NODISCARD SizeType distance(ConstIterator pIt) const noexcept;
        SR_NODISCARD SizeType distance(Iterator pIt) const noexcept;

        void reserve(SizeType newCapacity);
        void resize(SizeType newSize);
        void resize(SizeType newSize, const T& value);
        void shrink_to_fit();
        void clear() noexcept;
        void swap(Vector& other) noexcept;
        void assign(SizeType count, const T& value);
        void assign(ConstIterator first, ConstIterator last);
        void pop_back();
        void push_back(const T& value);
        void push_back(T&& value);
        template <class... ValueType> T& emplace_back(ValueType&&... value);
        SR_NODISCARD ConstIterator find(const T& value) const noexcept;
        SR_NODISCARD Iterator find(const T& value) noexcept;
        template<typename Predicate> SR_NODISCARD ConstIterator find_if(Predicate&& predicate) const noexcept;
        template<typename Predicate> SR_NODISCARD Iterator find_if(Predicate&& predicate) noexcept;
        Iterator erase(ConstIterator pos);
        Iterator erase(ConstIterator first, ConstIterator last);
        template<typename Predicate> Iterator remove_if(Predicate&& predicate);
        template<typename Predicate> SizeType erase_if(Predicate&& predicate);
        template <typename ValueType> Iterator insert(ConstIterator pos, ValueType&& value);
        template <typename ValueType> Iterator insert(ConstIterator pos, SizeType count, ValueType&& value);
        template <typename InputIt> Iterator insert(ConstIterator pos, InputIt pFirst, InputIt pLast);
        SR_NODISCARD T& at(SizeType index);
        SR_NODISCARD const T& at(SizeType index) const { return const_cast<Vector*>(this)->at(index); }

        SR_NODISCARD auto operator<=>(const Vector& other) const noexcept;

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

        SR_NODISCARD bool operator==(const Vector& other) const noexcept;
        SR_NODISCARD bool operator!=(const Vector& other) const noexcept { return !(*this == other); }

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
        SR_NODISCARD IAllocator* GetAllocator() const noexcept { return m_allocator; }
        SR_NODISCARD Vector<T> DetachAllocator() const;

    private:
        void ConstructRange(SizeType start, SizeType end);
        void ConstructRange(SizeType start, SizeType end, const T& value);
        void DestructRange(SizeType start, SizeType end);

        SR_NODISCARD void* AllocateMemory(SizeType size) const;
        void FreeMemory(void* pData, SizeType size) const;

    private:
        T* m_data = nullptr;
        SizeType m_size = 0;
        SizeType m_capacity = 0;
        IAllocator* m_allocator = nullptr;

    };

    template<typename T> auto Vector<T>::operator<=>(const Vector &other) const noexcept {
        if (m_size != other.m_size) {
            return m_size <=> other.m_size;
        }

        if constexpr (std::is_trivially_copyable_v<T>) {
            int cmp = std::memcmp(m_data, other.m_data, sizeof(T) * m_size);
            return cmp <=> 0;
        }
        else {
            for (SizeType i = 0; i < m_size; ++i) {
                T& left = static_cast<T*>(m_data)[i];
                T& right = static_cast<T*>(other.m_data)[i];

                if (left != right) {
                    return left <=> right;
                }
            }
        }

        return std::strong_ordering::equal;
    }

    template<typename T> SizeType Vector<T>::distance(ConstIterator pIt) const noexcept {
        return static_cast<SizeType>(pIt - begin());
    }

    template<typename T> SizeType Vector<T>::distance(Iterator pIt) const noexcept {
        return static_cast<SizeType>(pIt - begin());
    }

    template<typename T> void Vector<T>::delete_contents() {
        if constexpr (!std::is_const_v<T>) {
            if constexpr (std::is_trivially_destructible_v<T>) {
                if constexpr (std::is_pointer_v<T> && !std::is_same_v<T, void*>) {
                    for (SizeType i = 0; i < m_size; ++i) {
                        delete static_cast<T*>(m_data)[i];
                    }
                }
            }
            else {
                for (SizeType i = 0; i < m_size; ++i) {
                    static_cast<T*>(m_data)[i].~T();
                }
                m_size = 0;
            }
        }
    }

    template<typename T> bool Vector<T>::operator==(const Vector& other) const noexcept {
        if (m_size != other.m_size) {
            return false;
        }

        if constexpr (std::is_trivially_copyable_v<T>) {
            return std::memcmp(m_data, other.m_data, sizeof(T) * m_size) == 0;
        }
        else {
            for (SizeType i = 0; i < m_size; ++i) {
                T& left = static_cast<T*>(m_data)[i];
                T& right = static_cast<T*>(other.m_data)[i];

                if (!(left == right)) {
                    return false;
                }
            }
        }

        return true;
    }

    template<typename T> template<typename ValueType> Vector<T>::Iterator Vector<T>::insert(ConstIterator pos, SizeType count, ValueType&& value) {
        SizeType index = pos - begin();
        if (count == 0) {
            return begin() + index;
        }

        if (m_size + count > m_capacity) {
            reserve(m_size + count);
        }

        T* data = static_cast<T*>(m_data);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(data + index + count, data + index, sizeof(T) * (m_size - index));
            for (SizeType i = 0; i < count; ++i) {
                std::memcpy((void*)(data + index + i), (void*)(&value), sizeof(T));
            }
        }
        else {
            for (SizeType i = m_size; i > index; --i) {
                new (data + i + count - 1) T(std::move(data[i - 1]));
                data[i - 1].~T();
            }
            for (SizeType i = 0; i < count; ++i) {
                new (data + index + i) T(std::forward<ValueType>(value));
            }
        }

        m_size += count;
        return data + index;
    }

    template<typename T> template<typename InputIt> Vector<T>::Iterator Vector<T>::insert(ConstIterator pos, InputIt pFirst, InputIt pLast) {
        SizeType index = pos - begin();
        SizeType count = std::distance(pFirst, pLast);
        if (count == 0) {
            return begin() + index;
        }

        if (m_size + count > m_capacity) {
            reserve(m_size + count);
        }

        T* data = static_cast<T*>(m_data);

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(data + index + count, data + index, sizeof(T) * (m_size - index));
            for (SizeType i = 0; i < count; ++i) {
                std::memcpy((void*)(data + index + i), (void*)(&(*pFirst)), sizeof(T));
                ++pFirst;
            }
        }
        else {
            for (SizeType i = m_size; i > index; --i) {
                new (data + i + count - 1) T(std::move(data[i - 1]));
                data[i - 1].~T();
            }
            for (SizeType i = 0; i < count; ++i) {
                new (data + index + i) T(*pFirst);
                ++pFirst;
            }
        }

        m_size += count;
        return data + index;
    }

    template<typename T> void Vector<T>::FreeMemory(void *pData, SizeType size) const {
        if (m_allocator) {
            m_allocator->Free(pData, size);
        }
        else {
            SRFree(pData);
        }
    }

    template<typename T> void* Vector<T>::AllocateMemory(SizeType size) const {
        void* pMemory = m_allocator ? m_allocator->Allocate(size) : SRMalloc(size);
        if (!pMemory) {
            SRHalt("Vector::AllocateMemory() : failed to allocate memory! Size: {}", size);
        }
        return pMemory;
    }

    template<typename T> Vector<T> Vector<T>::DetachAllocator() const {
        Vector<T> newVector;
        newVector.reserve(m_size);
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)newVector.m_data, (void*)m_data, sizeof(T) * m_size);
        }
        else {
            for (SizeType i = 0; i < m_size; ++i) {
                new (newVector.m_data + i) T(static_cast<const T*>(m_data)[i]);
            }
        }
        newVector.m_size = m_size;
        return newVector;
    }

    template<typename T> template<typename... ValueType> T& Vector<T>::emplace_back(ValueType &&... value) {
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

    template<typename T> Vector<T>& Vector<T>::operator=(const Vector& other) {
        if (this != &other) {
            if (other.m_size > m_capacity || m_allocator != other.m_allocator) {
                Vector temp(other);
                swap(temp);
            }
            else {
                DestructRange(0, m_size);
                if (std::is_trivially_copyable_v<T>) {
                    std::memcpy((void*)m_data, (void*)other.m_data, sizeof(T) * other.m_size);
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

    template<typename T> template<typename Predicate> Vector<T>::Iterator Vector<T>::remove_if(Predicate&& predicate) {
        Iterator newEnd = begin();
        for (Iterator it = begin(); it != end(); ++it) {
            if (!predicate(*it)) {
                if (newEnd != it) {
                    *newEnd = std::move(*it);
                }
                ++newEnd;
            }
        }
        SizeType newSize = newEnd - begin();
        DestructRange(newSize, m_size);
        m_size = newSize;
        return newEnd;
    }

    template<typename T> template<typename Predicate> Vector<T>::SizeType Vector<T>::erase_if(Predicate&& predicate) {
        SizeType originalSize = m_size;
        remove_if(std::forward<Predicate>(predicate));
        return originalSize - m_size;
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

    template<typename T> template<typename ValueType> Vector<T>::Iterator Vector<T>::insert(Vector::ConstIterator pos, ValueType&& value) {
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
            std::memcpy((void*)(data + index), (void*)(&value), sizeof(T));
        }
        else {
            for (SizeType i = m_size; i > index; --i) {
                new (data + i) T(std::move(data[i - 1]));
                data[i - 1].~T();
            }
            new (data + index) T(std::forward<ValueType>(value));
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

    template<typename T> template<typename Predicate> Vector<T>::ConstIterator Vector<T>::find_if(Predicate&& predicate) const noexcept {
        for (SizeType i = 0; i < m_size; ++i) {
            if (predicate(static_cast<T*>(m_data)[i])) {
                return begin() + i;
            }
        }
        return end();
    }

    template<typename T> template<typename Predicate> Vector<T>::Iterator Vector<T>::find_if(Predicate&& predicate) noexcept {
        return const_cast<Iterator>(static_cast<const Vector*>(this)->find_if(std::forward<Predicate>(predicate)));
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
        std::swap(m_allocator, other.m_allocator);
    }

    template<typename T> Vector<T>& Vector<T>::operator=(std::initializer_list<T> init) {
        if (init.size() > m_capacity) {
            Vector temp(init);
            swap(temp);
        }
        else {
            DestructRange(0, m_size);
            if (std::is_trivially_copyable_v<T>) {
                std::memcpy((void*)m_data, (void*)init.begin(), sizeof(T) * init.size());
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

    template<typename T> Vector<T>& Vector<T>::operator=(Vector&& other) noexcept {
        if (this != &other) {
            if (m_data) {
                DestructRange(0, m_size);
                FreeMemory(m_data, sizeof(T) * m_capacity);
            }

            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            m_allocator = other.m_allocator;

            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
            other.m_allocator = nullptr;
        }
        return *this;
    }

    template<typename T> Vector<T>::Vector(IAllocator* pAllocator) noexcept {
        m_allocator = pAllocator;
    }

    template<typename T> void Vector<T>::assign(Vector::ConstIterator first, Vector::ConstIterator last) {
        SizeType count = last - first;
        if (count > m_capacity) {
            Vector temp(m_allocator, first, last);
            swap(temp);
        }
        else {
            DestructRange(0, m_size);
            if (std::is_trivially_copyable_v<T>) {
                std::memcpy((void*)m_data, (void*)first, sizeof(T) * count);
            }
            else {
                for (SizeType i = 0; i < count; ++i) {
                    new (static_cast<T*>(m_data) + i) T(first[i]);
                }
            }
            m_size = count;
        }
    }

    template<typename T> Vector<T>::Vector(Vector::ConstIterator first, Vector::ConstIterator last)
        : Vector(nullptr, first, last)
    { }

    template<typename T> Vector<T>::Vector(IAllocator* pAllocator, Vector::ConstIterator first, Vector::ConstIterator last) {
        SizeType count = last - first;
        m_allocator = pAllocator;
        m_data = static_cast<T*>(AllocateMemory(sizeof(T) * count));
        m_size = count;
        m_capacity = count;

        if (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)m_data, (void*)first, sizeof(T) * count);
        }
        else {
            for (SizeType i = 0; i < count; ++i) {
                new (static_cast<T*>(m_data) + i) T(first[i]);
            }
        }
    }

    template<typename T> Vector<T>::Vector(std::initializer_list<T> init) {
        m_data = static_cast<T*>(AllocateMemory(sizeof(T) * init.size()));
        m_size = init.size();
        m_capacity = init.size();

        if (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)m_data, (void*)init.begin(), sizeof(T) * init.size());
        }
        else {
            for (size_t i = 0; i < init.size(); ++i) {
                new (static_cast<T*>(m_data) + i) T(init.begin()[i]);
            }
        }
    }

    template<typename T> Vector<T>::Vector(const Vector& other) {
        m_allocator = other.m_allocator;
        m_size = other.m_size;
        m_capacity = other.m_size;
        m_data = static_cast<T*>(AllocateMemory(sizeof(T) * other.m_size));

        if (std::is_trivially_copyable_v<T>) {
            std::memcpy((void*)m_data, (void*)other.m_data, sizeof(T) * other.m_size);
        }
        else {
            for (size_t i = 0; i < other.m_size; ++i) {
                new (static_cast<T*>(m_data) + i) T(static_cast<T*>(other.m_data)[i]);
            }
        }
    }

    template<typename T> Vector<T>::Vector(Vector&& other) noexcept {
        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        m_allocator = other.m_allocator;

        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
        other.m_allocator = nullptr;
    }

    template<typename T> Vector<T>::Vector(const SizeType count) {
        m_data = static_cast<T*>(AllocateMemory(sizeof(T) * count));
        m_size = count;
        m_capacity = count;
        ConstructRange(0, count);
    }

    template<typename T> Vector<T>::Vector(const SizeType count, const T& value) {
        m_data = static_cast<T*>(AllocateMemory(sizeof(T) * count));
        m_size = count;
        m_capacity = count;
        ConstructRange(0, count, value);
    }

    template<typename T> Vector<T>::~Vector() {
        if (m_data) {
            DestructRange(0, m_size);
            FreeMemory(m_data, sizeof(T) * m_capacity);
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
            m_data = static_cast<T*>(AllocateMemory(sizeof(T) * m_size));

            if (pOldData) {
                if (std::is_trivially_copyable_v<T>) {
                    std::memcpy((void*)m_data, (void*)pOldData, sizeof(T) * m_size);
                }
                else {
                    for (SizeType i = 0; i < m_size; ++i) {
                        new (static_cast<T*>(m_data) + i) T(std::move(static_cast<T*>(pOldData)[i]));
                        static_cast<T*>(pOldData)[i].~T();
                    }
                }
                FreeMemory(pOldData, sizeof(T) * m_capacity);
            }

            m_capacity = m_size;
        }
    }

    template<typename T> void Vector<T>::resize(const SizeType newSize, const T& value) {
        if (newSize > m_size) {
            reserve(newSize);
            ConstructRange(m_size, newSize, value);
        }
        else if (newSize < m_size) {
            DestructRange(newSize, m_size);
        }
        m_size = newSize;
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
            m_data = static_cast<T*>(AllocateMemory(sizeof(T) * newCapacity));

            if (pOldData) {
                if (std::is_trivially_copyable_v<T>) {
                    std::memcpy((void*)m_data, (void*)pOldData, sizeof(T) * m_size);
                }
                else {
                    for (SizeType i = 0; i < m_size; ++i) {
                        new (static_cast<T*>(m_data) + i) T(std::move(static_cast<T*>(pOldData)[i]));
                        static_cast<T*>(pOldData)[i].~T();
                    }
                }
                FreeMemory(pOldData, sizeof(T) * m_capacity);
            }
            m_capacity = newCapacity;
        }
    }

    extern template class Vector<int32_t>;
    extern template class Vector<uint32_t>;
    extern template class Vector<StringAtom>;
}

namespace std {
    template <class T, class Predicate>
    constexpr SR_UTILS_NS::Vector<T>::SizeType erase_if(SR_UTILS_NS::Vector<T>& container, Predicate predicate) {
        return container.erase_if(std::forward<Predicate>(predicate));
    }
}

#endif //SR_COMMON_VECTOR_H
