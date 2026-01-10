//
// Created by Monika on 02.01.2026.
//

#ifndef SR_ENGINE_UTILS_TYPES_OPTIONAL_H
#define SR_ENGINE_UTILS_TYPES_OPTIONAL_H

#include <Utils/stdInclude.h>

namespace SR_HTYPES_NS {
    template<typename T> class Optional {
    public:
        Optional() noexcept = default;

        Optional(const T& value) noexcept
            : m_hasValue(true)
            , m_value(value)
        { }

        Optional(T&& value) noexcept
            : m_hasValue(true)
            , m_value(std::move(value))
        { }

        Optional(const Optional<T>& other) noexcept
            : m_hasValue(other.m_hasValue)
            , m_value(other.m_value)
        { }

        Optional(Optional<T>&& other) noexcept
            : m_hasValue(other.m_hasValue)
            , m_value(std::move(other.m_value))
        {
            other.m_hasValue = false;
        }

        ~Optional() = default;

        Optional<T>& operator=(const Optional<T>& other) noexcept {
            m_hasValue = other.m_hasValue;
            m_value = other.m_value;
            return *this;
        }

        Optional<T>& operator=(Optional<T>&& other) noexcept {
            m_hasValue = other.m_hasValue;
            m_value = std::move(other.m_value);
            other.m_hasValue = false;
            return *this;
        }

    public:
        SR_NODISCARD bool HasValue() const noexcept { return m_hasValue; }
        SR_NODISCARD T& Value() { return m_value; }
        SR_NODISCARD const T& Value() const { return m_value; }

        SR_NODISCARD bool has_value() const noexcept { return m_hasValue; }
        SR_NODISCARD T& value() { return m_value; }
        SR_NODISCARD const T& value() const { return m_value; }

        SR_NODISCARD T& operator*() { return m_value; }
        SR_NODISCARD const T& operator*() const { return m_value; }

        SR_NODISCARD T* operator->() { return &m_value; }
        SR_NODISCARD const T* operator->() const { return &m_value; }

    private:
        bool m_hasValue = false;
        T m_value;
    };
}

#endif //SR_ENGINE_UTILS_TYPES_OPTIONAL_H
