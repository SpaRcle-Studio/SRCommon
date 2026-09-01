//
// Created by Monika on 23.04.2024.
//

#ifndef SR_ENGINE_UTILS_SORTED_VECTOR_H
#define SR_ENGINE_UTILS_SORTED_VECTOR_H

#include <Utils/Types/Stack.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_HTYPES_NS {
    template<typename T, typename Predicate = std::less<T>> class SortedVector {
    public:
        using Iterator = typename SR_UTILS_NS::Vector<T>::iterator;
        using ConstIterator = typename SR_UTILS_NS::Vector<T>::const_iterator;

        SortedVector() = default;
        SortedVector(SortedVector&& other) noexcept
            : m_data(SR_UTILS_NS::Exchange(other.m_data, { }))
        { }
        SortedVector(const SortedVector& other)
            : m_data(other.m_data)
        { }

        SortedVector& operator=(SortedVector&& other) noexcept {
            if (this != &other) {
                m_data = SR_UTILS_NS::Exchange(other.m_data, { });
            }
            return *this;
        }

        SortedVector& operator=(const SortedVector& other) {
            if (this != &other) {
                m_data = other.m_data;
            }
            return *this;
        }

        SR_NODISCARD SR_CONSTEXPR bool Empty() const { return m_data.empty(); }
        SR_NODISCARD SR_CONSTEXPR bool empty() const { return m_data.empty(); }

        SR_NODISCARD SR_CONSTEXPR uint64_t Size() const { return m_data.size(); }
        SR_NODISCARD SR_CONSTEXPR uint64_t size() const { return m_data.size(); }

        SR_NODISCARD SR_CONSTEXPR uint64_t Capacity() const { return m_data.capacity(); }
        SR_NODISCARD SR_CONSTEXPR uint64_t capacity() const { return m_data.capacity(); }

        SR_NODISCARD SR_CONSTEXPR T* Data() { return m_data.data(); }
        SR_NODISCARD SR_CONSTEXPR const T* Data() const { return m_data.data(); }

        SR_NODISCARD SR_CONSTEXPR T* data() { return m_data.data(); }
        SR_NODISCARD SR_CONSTEXPR const T* data() const { return m_data.data(); }

        SR_NODISCARD SR_CONSTEXPR T& front() { return m_data.front(); }
        SR_NODISCARD SR_CONSTEXPR const T& front() const { return m_data.front(); }

        SR_NODISCARD SR_CONSTEXPR T& back() { return m_data.back(); }
        SR_NODISCARD SR_CONSTEXPR const T& back() const { return m_data.back(); }

        SR_NODISCARD bool exists(const T& value) const {
            if (m_data.empty()) {
                return false;
            }

            auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_predicate);
            return it != m_data.end() && *it == value;
        }

        SR_NODISCARD uint32_t count(const T& value) const {
            return exists(value) ? 1 : 0;
        }

        SR_NODISCARD SR_CONSTEXPR T& operator[](uint64_t index) {
            return m_data[index];
        }

        SR_NODISCARD SR_CONSTEXPR const T& operator[](uint64_t index) const {
            return m_data[index];
        }

        template<class FwdIt, class Ty, typename CustomPred = Predicate> SR_NODISCARD T* UpperBound(FwdIt pStart, const FwdIt pEnd, const Ty& value, const CustomPred& predicate = CustomPred()) {
            /// return std::upper_bound(pStart, pEnd, value, predicate);

            auto length = pEnd - pStart;
            while (length > 0) {
                auto half = length / 2;
                auto middle = pStart + half;
                if (predicate(*middle, value)) {
                    pStart = middle + 1;
                    length -= half + 1;
                }
                else {
                    length = half;
                }
            }
            return pStart;
        }

        template<class Ty> SR_NODISCARD Iterator LowerBound(const Ty& value) {
            if (m_data.empty()) {
                return m_data.end();
            }
            return std::lower_bound(m_data.begin(), m_data.end(), value, m_predicate);
        }

        template<class Ty, typename CustomPred = Predicate> SR_NODISCARD Iterator LowerBound(const Ty& value, const CustomPred& predicate) {
            if (m_data.empty()) {
                return m_data.end();
            }
            return std::lower_bound(m_data.begin(), m_data.end(), value, predicate);
        }

        void Insert(Iterator it, const T& value) {
            m_data.insert(it, value);
        }

        void insert(Iterator it, const T& value) {
            m_data.insert(it, value);
        }

        bool insert(const T& value) {
            return Add(value);
        }

        bool insert(T&& value) {
            return Add(std::forward<T>(value));
        }

        void Erase(Iterator it) {
            m_data.erase(it);
        }
        void erase(Iterator it) {
            m_data.erase(it);
        }

        SR_NODISCARD SR_CONSTEXPR Iterator begin() { return m_data.begin(); }
        SR_NODISCARD SR_CONSTEXPR ConstIterator begin() const { return m_data.begin(); }

        SR_NODISCARD SR_CONSTEXPR Iterator end() { return m_data.end(); }
        SR_NODISCARD SR_CONSTEXPR ConstIterator end() const { return m_data.end(); }

        SR_NODISCARD bool Contains(const T& value) const {
            if (m_data.empty()) {
                return false;
            }

            auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_predicate);
            return it != m_data.end() && *it == value;
        }

        SR_NODISCARD const T* Find(const T& value) const {
            if (m_data.empty()) {
                return nullptr;
            }

            auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_predicate);
            if (it != m_data.end() && *it == value) {
                return &(*it);
            }
            return nullptr;
        }

        SR_NODISCARD T* Find(const T& value) {
            return const_cast<T*>(static_cast<const SortedVector&>(*this).Find(value));
        }

        SR_NODISCARD bool VerifyDuplicates() const {
            if (m_data.empty()) {
                return true;
            }

            for (auto it = m_data.begin(); it != m_data.end() - 1; ++it) {
                if (*it == *(it + 1)) {
                    return false; // Duplicate found
                }
            }
            return true; // No duplicates
        }

        bool Add(const T& value) {
            SR_TRACY_ZONE;
            if (m_data.empty()) {
                m_data.push_back(value);
                return true;
            }

            auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_predicate);

            if (it != m_data.end() && !m_predicate(value, *it) && !m_predicate(*it, value)) {
                return false; // Duplicate found, do not insert
            }

            m_data.insert(it, value);
            return true;
        }

        bool Add(T&& value) {
            SR_TRACY_ZONE;
            if (m_data.empty()) {
                m_data.push_back(std::forward<T>(value));
                return true;
            }

            auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_predicate);

            if (it != m_data.end() && !m_predicate(value, *it) && !m_predicate(*it, value)) {
                return false; // Duplicate found, do not insert
            }

            m_data.insert(it, std::forward<T>(value));
            return true;
        }

        template<class Ty, typename CustomPred = Predicate> SR_NODISCARD Iterator GetOrCreate(const Ty& value, const CustomPred& predicate) {
            auto it = std::lower_bound(m_data.begin(), m_data.end(), value, predicate);
            if (it == m_data.end() || *it != value) {
                return m_data.insert(it, T());
            }
            return it;
        }

        bool Remove(const T& value) {
            if (m_data.empty()) {
                return false;
            }

            auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_predicate);
            if (it != m_data.end() && *it == value) {
                m_data.erase(it);
                return true;
            }
            return false;
        }

        void Reserve(uint64_t size) {
            m_data.reserve(size);
        }

        void reserve(uint64_t size) {
            m_data.reserve(size);
        }

        void Clear() {
            SR_TRACY_ZONE;
            m_data.clear();
        }

        void clear() {
            SR_TRACY_ZONE;
            m_data.clear();
        }

        void ShrunkToFit() {
            m_data.shrink_to_fit();
        }

    private:
        static constexpr Predicate m_predicate = Predicate();
        SR_UTILS_NS::Vector<T> m_data;

    };
}

#endif //SR_ENGINE_UTILS_SORTED_VECTOR_H
