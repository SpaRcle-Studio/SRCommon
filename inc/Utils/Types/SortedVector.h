//
// Created by Monika on 23.04.2024.
//

#ifndef SR_ENGINE_UTILS_SORTED_VECTOR_H
#define SR_ENGINE_UTILS_SORTED_VECTOR_H

#include <Utils/Types/Stack.h>

namespace SR_HTYPES_NS {
    template<typename T, typename Predicate = std::less<T>> class SortedVector {
    public:
        using Iterator = typename std::vector<T>::iterator;
        using ConstIterator = typename std::vector<T>::const_iterator;

        SR_NODISCARD SR_CONSTEXPR bool Empty() const { return m_data.empty(); }
        SR_NODISCARD SR_CONSTEXPR bool empty() const { return m_data.empty(); }

        SR_NODISCARD SR_CONSTEXPR uint64_t Size() const { return m_data.size(); }
        SR_NODISCARD SR_CONSTEXPR uint64_t size() const { return m_data.size(); }

        SR_NODISCARD SR_CONSTEXPR T* Data() { return m_data.data(); }
        SR_NODISCARD SR_CONSTEXPR const T* Data() const { return m_data.data(); }

        SR_NODISCARD SR_CONSTEXPR T* data() { return m_data.data(); }
        SR_NODISCARD SR_CONSTEXPR const T* data() const { return m_data.data(); }

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

        SR_NODISCARD SR_CONSTEXPR Iterator begin() { return m_data.begin(); }
        SR_NODISCARD SR_CONSTEXPR ConstIterator begin() const { return m_data.begin(); }

        SR_NODISCARD SR_CONSTEXPR Iterator end() { return m_data.end(); }
        SR_NODISCARD SR_CONSTEXPR ConstIterator end() const { return m_data.end(); }

        void Add(const T& value) {
            if (m_data.empty()) {
                m_data.push_back(value);
                return;
            }

            auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_predicate);
            m_data.insert(it, value);
        }

        void Add(T&& value) {
            if (m_data.empty()) {
                m_data.push_back(std::move(value));
                return;
            }

            auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_predicate);
            m_data.insert(it, std::move(value));
        }

        bool Remove(const T& value) {
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

        void Clear() {
            m_data.clear();
        }

        void ShrunkToFit() {
            m_data.shrink_to_fit();
        }

    private:
        static constexpr Predicate m_predicate = Predicate();

        std::vector<T> m_data;

    };
}

#endif //SR_ENGINE_UTILS_SORTED_VECTOR_H
