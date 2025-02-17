//
// Created by Monika on 20.01.2025.
//

#ifndef SR_UTILS_TYPE_TRAITS_VALUE_H
#define SR_UTILS_TYPE_TRAITS_VALUE_H

#include <Utils/Reflection/ValueImpl.h>

#include <entt/entt.hpp>

namespace SR_UTILS_NS::Reflection {
    class Value;
    class ValueSequenceContainer;

    class ValueSequenceContainerIterator {
        friend ValueSequenceContainer;
    private:
        explicit ValueSequenceContainerIterator(entt::meta_sequence_container::iterator&& iterator)
            : m_iterator(std::move(iterator))
        { }

    public:
        ValueSequenceContainerIterator& operator++() noexcept { ++m_iterator; return *this; }
        ValueSequenceContainerIterator& operator--() noexcept { --m_iterator; return *this; }

        ValueSequenceContainerIterator operator++(int32_t value) noexcept {
            ValueSequenceContainerIterator orig = *this;
            ++m_iterator;
            return orig;
        }

        ValueSequenceContainerIterator operator--(int32_t value) noexcept {
            ValueSequenceContainerIterator orig = *this;
            --m_iterator;
            return orig;
        }

        SR_NODISCARD Value operator*() const;
        SR_NODISCARD InputIteratorPointer<Value> operator->() const;

        SR_NODISCARD operator bool() const noexcept { return static_cast<bool>(m_iterator); } /// NOLINT

        SR_NODISCARD bool operator==(const ValueSequenceContainerIterator& other) const noexcept {
            return m_iterator == other.m_iterator;
        }

        SR_NODISCARD bool operator!=(const ValueSequenceContainerIterator& other) const noexcept {
            return !(*this == other);
        }

    private:
        entt::meta_sequence_container::iterator m_iterator;

    };

    class SR_DLL_EXPORT SR_NODISCARD ValueSequenceContainer {
        friend Value;
    private:
        explicit ValueSequenceContainer(entt::meta_sequence_container&& storage)
            : m_storage(storage)
        { }

    public:
        SR_NODISCARD ValueSequenceContainerIterator begin() { return ValueSequenceContainerIterator(m_storage.begin()); }
        SR_NODISCARD ValueSequenceContainerIterator end() { return ValueSequenceContainerIterator(m_storage.end()); }

        void Clear();
        void Resize(uint64_t size);
        void Reserve(uint64_t size);

        SR_NODISCARD uint64_t Size() const { return m_storage.size(); }
        SR_NODISCARD bool Empty() const { return Size() == 0; }

    private:
        entt::meta_sequence_container m_storage;
    };

    /// ----------------------------------------------------------------------------------------------------------------

    class SR_DLL_EXPORT SR_NODISCARD Value {
        friend ValueSequenceContainerIterator;
    private:
        explicit Value(entt::meta_any&& storage)
            : m_storage(std::move(storage))
        { }

    public:
        Value() = default;

        Value(const Value& other)
            : m_storage(other.IsRef() ? other.m_storage.as_ref() : other.m_storage)
        { }

        Value(Value& other)
            : m_storage(other.IsRef() ? other.m_storage.as_ref() : other.m_storage)
        { }

        Value& operator=(const Value& other) noexcept {
            if (this != &other) {
                m_storage = other.IsRef() ? const_cast<entt::meta_any*>(&other.m_storage)->as_ref() : other.m_storage;
            }
            return *this;
        }

        Value& operator=(Value&& other) noexcept {
            if (this != &other) {
                m_storage = other.IsRef() ? other.m_storage.as_ref() : other.m_storage;
            }
            return *this;
        }

        template<typename T> static Value Create(T&& value);
        template<typename T> static Value CreateRef(T& value);
        template<typename T> static Value CreateCRef(const T& value);

        template<typename T> const T* TryCast() const { return m_storage.try_cast<T>(); }
        template<typename T> T* TryCast() { return m_storage.try_cast<T>(); }

        Value& Detach();
        Value& DetachIfConst();

        SR_NODISCARD ValueSequenceContainer AsSequenceContainer();
        SR_NODISCARD ValueSequenceContainer AsSequenceContainer() const;

        SR_NODISCARD Value Ref();
        SR_NODISCARD Value Copy() const;

        SR_NODISCARD bool IsRef() const;
        SR_NODISCARD bool IsConst() const;

        SR_NODISCARD bool IsSequenceContainer() const;
        SR_NODISCARD bool IsAssociativeContainer() const;
        SR_NODISCARD bool IsBitMap() const;

        SR_NODISCARD bool IsSmartPtr() const;
        SR_NODISCARD bool IsString() const;
        SR_NODISCARD bool IsStringView() const;
        SR_NODISCARD bool IsPath() const;
        SR_NODISCARD bool IsMathVector() const;
        SR_NODISCARD bool IsMathSize() const;
        SR_NODISCARD bool IsBool() const;
        SR_NODISCARD bool IsArithmetic() const;
        SR_NODISCARD bool IsClass() const;
        SR_NODISCARD bool IsTemplate() const;
        SR_NODISCARD bool IsIntegral() const;
        SR_NODISCARD bool IsSigned() const;
        SR_NODISCARD bool IsEnum() const;
        SR_NODISCARD std::string_view GetTypeName() const;
        SR_NODISCARD std::string_view GetSharedPtrType() const;
        SR_NODISCARD uint64_t SizeOf() const;
        SR_NODISCARD void* Data();
        SR_NODISCARD const void* Data() const;
        SR_NODISCARD std::string_view GetEnumType() const;

        SR_NODISCARD operator bool() const noexcept; /// NOLINT

    private:
        entt::meta_any m_storage;
    };

    /// Implementation

    template<typename T> Value Value::Create(T&& value) {
        return Value(entt::meta_any(std::forward<T>(value)));
    }

    template<typename T> Value Value::CreateRef(T& value) {
        return Value(entt::meta_any::create_ref(value));
    }

    template<typename T> Value Value::CreateCRef(const T& value) {
        return Value(entt::meta_any::create_cref(value));
    }
}

#endif //SR_UTILS_TYPE_TRAITS_VALUE_H
