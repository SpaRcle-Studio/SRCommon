//
// Created by Monika on 12.02.2026.
//

#ifndef SR_ENGINE_COMMON_TYPES_SET_VECTOR_H
#define SR_ENGINE_COMMON_TYPES_SET_VECTOR_H

#include <Utils/Debug.h>

namespace SR_HTYPES_NS {
    /// отсортированный вектор, который не позволяет вставлять дубликаты
    template<typename T> class SetVector {
    public:
        using VectorType = std::vector<T>;
        using Iterator = typename VectorType::iterator;
        using ConstIterator = typename VectorType::const_iterator;

        SR_NODISCARD Iterator begin() { return m_vector.begin(); }
        SR_NODISCARD Iterator end() { return m_vector.end(); }
        SR_NODISCARD ConstIterator begin() const { return m_vector.begin(); }
        SR_NODISCARD ConstIterator end() const { return m_vector.end(); }
        SR_NODISCARD size_t size() const { return m_vector.size(); }
        SR_NODISCARD bool empty() const { return m_vector.empty(); }
        SR_NODISCARD T& back() { return m_vector.back(); }
        SR_NODISCARD const T& back() const { return m_vector.back(); }

        Iterator insert(const T& value) {
            auto it = std::lower_bound(m_vector.begin(), m_vector.end(), value);
            if (it != m_vector.end() && *it == value) {
                return it; // элемент уже существует, возвращаем итератор на него
            }
            return m_vector.insert(it, value); // вставляем новый элемент и возвращаем итератор на него
        }

        Iterator erase(const T& value) {
            auto it = std::lower_bound(m_vector.begin(), m_vector.end(), value);
            if (it != m_vector.end() && *it == value) {
                return m_vector.erase(it); // удаляем элемент и возвращаем итератор на следующий за ним
            }
            return it; // элемент не найден, возвращаем итератор на позицию вставки
        }

        SR_NODISCARD bool contains(const T& value) const {
            auto it = std::lower_bound(m_vector.begin(), m_vector.end(), value);
            return it != m_vector.end() && *it == value; // проверяем, найден ли элемент
        }

        void pop_back() {
            m_vector.pop_back();
        }

        void clear() {
            m_vector.clear();
        }

        void reserve(size_t newCapacity) {
            m_vector.reserve(newCapacity);
        }

    private:
        VectorType m_vector;

    };}

#endif //SR_ENGINE_COMMON_TYPES_SET_VECTOR_H
