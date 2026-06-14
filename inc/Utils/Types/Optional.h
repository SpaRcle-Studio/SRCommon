//
// Created by Monika on 02.01.2026.
//

#ifndef SR_ENGINE_UTILS_TYPES_OPTIONAL_H
#define SR_ENGINE_UTILS_TYPES_OPTIONAL_H

#include <Utils/Reflection/Value.h>

namespace SR_HTYPES_NS {
    class OptionalBase {
    public:
        OptionalBase() = default;
        virtual ~OptionalBase() = default;

        SR_NODISCARD virtual bool HasValue() const noexcept = 0;
        SR_NODISCARD virtual Reflection::Value GetReflectionValue() const noexcept = 0;
        virtual void Reset() noexcept = 0;
        virtual void SetValue(const Reflection::Value& value) = 0;

    };

    template<typename T> class Optional : public OptionalBase {
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
        SR_NODISCARD Reflection::Value GetReflectionValue() const noexcept override {
            if (m_hasValue) {
                return Reflection::Value::CreateRef(const_cast<T&>(m_value));
            }
            static const T defaultValue{};
            return Reflection::Value::CreateCRef(defaultValue);
        }

        void SetValue(const Reflection::Value& value) override {
            if (auto&& pValue = value.TryCast<T>()) {
                m_value = *pValue;
                m_hasValue = true;
            }
            else {
                SRHalt("SR_HTYPES_NS::Optional::SetValue() : value is not of the correct type!");
            }
        }

        SR_NODISCARD bool HasValue() const noexcept override { return m_hasValue; }
        SR_NODISCARD T& Value() { return m_value; }
        SR_NODISCARD const T& Value() const { return m_value; }

        SR_NODISCARD bool has_value() const noexcept { return m_hasValue; }
        SR_NODISCARD T& value() { return m_value; }
        SR_NODISCARD const T& value() const { return m_value; }

        SR_NODISCARD T& value_or(T&& defaultValue) { return m_hasValue ? m_value : defaultValue; }
        SR_NODISCARD const T& value_or(const T& defaultValue) const { return m_hasValue ? m_value : defaultValue; }

        SR_NODISCARD T& operator*() { return m_value; }
        SR_NODISCARD const T& operator*() const { return m_value; }

        SR_NODISCARD T* operator->() { return &m_value; }
        SR_NODISCARD const T* operator->() const { return &m_value; }

        SR_NODISCARD operator bool() const noexcept { return m_hasValue; } /// NOLINT

        void reset() noexcept {
            m_hasValue = false;
            m_value = T();
        }

        void Reset() noexcept override {
            reset();
        }

        void FastReset() noexcept {
            m_hasValue = false;
        }

    private:
        bool m_hasValue = false;
        T m_value;
    };
}

#endif //SR_ENGINE_UTILS_TYPES_OPTIONAL_H
