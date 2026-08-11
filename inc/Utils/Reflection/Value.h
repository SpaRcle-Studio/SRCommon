//
// Created by Monika on 20.01.2025.
//

#ifndef SR_UTILS_TYPE_TRAITS_VALUE_H
#define SR_UTILS_TYPE_TRAITS_VALUE_H

#include <Utils/Reflection/ReflectedType.h>

// #define SR_COMMON_USE_ENTT

namespace SR_HTYPES_NS {
    class SharedPtrBase;
}

namespace SR_UTILS_NS {
    class OptionalBase;
}

namespace SR_UTILS_NS::Reflection {
    enum class ReflectedCategoryType : uint8_t;
    class Value;

    /// ----------------------------------------------------------------------------------------------------------------

    class SR_COMMON_DLL_API SR_NODISCARD BaseContainerValueRef {
    public:
        BaseContainerValueRef() = default;
        explicit BaseContainerValueRef(Value* pValue);
        virtual ~BaseContainerValueRef() = default;

    public:
        SR_NODISCARD SizeType Size() const;
        SR_NODISCARD bool Empty() const;

        void Clear();

        SR_NODISCARD ReflectedContainerIterator Begin() const;
        SR_NODISCARD ReflectedContainerIterator End() const;
        ReflectedContainerIterator Erase(ReflectedContainerIterator pIt);

    protected:
        Value* m_value = nullptr;

    };

    /// ----------------------------------------------------------------------------------------------------------------

    class SR_COMMON_DLL_API SR_NODISCARD SequenceContainerValueRef : public BaseContainerValueRef {
    public:
        SequenceContainerValueRef() = default;
        explicit SequenceContainerValueRef(Value* pValue);

        SR_NODISCARD Value Back();

        void Resize(SizeType newSize);

        ReflectedContainerIterator Insert(ReflectedContainerIterator pIt, const Value& value);

        void PushBack(const Value& value);
        void PushFront(const Value& value);

    };

    /// ----------------------------------------------------------------------------------------------------------------

    class SR_COMMON_DLL_API AssociativeContainerValueRef : public BaseContainerValueRef {
    public:
        AssociativeContainerValueRef() = default;
        explicit AssociativeContainerValueRef(Value* pValue);

    public:
        ReflectedContainerIterator Insert(const Value& key, const Value& value);

    };

    /// ----------------------------------------------------------------------------------------------------------------

    class SR_COMMON_DLL_API SR_NODISCARD Value {
        friend class ReflectedContainerIterator;
        friend class SequenceContainerValueRef;
    public:
        Value();
        Value(const Value& other);
        Value(Value&& other) noexcept;
        Value& operator=(const Value& other) noexcept;
        Value& operator=(Value&& other) noexcept;
        ~Value();

        template<typename T> static Value Create(T&& value, TypeInfo* pTypeInfo);
        template<typename T> static Value CreateRef(T& value, TypeInfo* pTypeInfo);
        template<typename T> static Value CreateCRef(const T& value, TypeInfo* pTypeInfo);

        template<typename T> static Value Create(T&& value);
        template<typename T> static Value CreateRef(T& value);
        template<typename T> static Value CreateCRef(const T& value);

        template<typename T> const T* Cast() const;
        template<typename T> T* Cast();

        SR_NODISCARD static Value CreateDefault(TypeInfo* pTypeInfo);

        SR_NODISCARD SequenceContainerValueRef AsSequenceContainer();
        SR_NODISCARD AssociativeContainerValueRef AsAssociativeContainer();

        SR_NODISCARD ReflectedCategoryType GetType() const;

        SR_NODISCARD Value Copy() const;
        SR_NODISCARD Value Ref() const;

        SR_NODISCARD bool IsRef() const;
        SR_NODISCARD bool IsConst() const;
        SR_NODISCARD bool IsEmbedded() const;
        SR_NODISCARD bool IsDynamic() const;

        SR_NODISCARD ReflectedValue& GetStorage();
        SR_NODISCARD const ReflectedValue& GetStorage() const;
        SR_NODISCARD SizeType SizeOf() const;
        SR_NODISCARD bool IsSigned() const;
        SR_NODISCARD bool IsIntegral() const;
        SR_NODISCARD bool IsSharedPtr() const;
        SR_NODISCARD void* Data();
        SR_NODISCARD const void* Data() const;
        SR_NODISCARD SRClass* GetSRClass() const;
        SR_NODISCARD StringAtom GetEnumType() const;
        SR_NODISCARD OptionalBase* GetOptionalBase() const;
        SR_NODISCARD const TypeInfo& GetTypeInfo() const;
        SR_NODISCARD SR_HTYPES_NS::SharedPtrBase* GetSharedPtrBase() const;
        SR_NODISCARD IAllocator* GetAllocator() const;

        SR_NODISCARD bool IsValid() const;
        SR_NODISCARD operator bool() const noexcept; /// NOLINT

    private:
        void Destroy();

    private:
        ReflectedValue m_storage;
        TypeInfo* m_typeInfo = nullptr;
        IAllocator* m_allocator = nullptr;

    };

    /// ----------------------------------------------------------------------------------------------------------------

    template<typename T> Value Value::Create(T&& value) {
        return Create(std::forward<T>(value), DetermineTypeInfoAlloc<T>());
    }

    template<typename T> Value Value::CreateRef(T& value) {
        return CreateRef(value, DetermineTypeInfoAlloc<T>());
    }

    template<typename T> Value Value::CreateCRef(const T& value) {
        return CreateCRef(value, DetermineTypeInfoAlloc<T>());
    }

    template<typename T> Value Value::Create(T&& value, TypeInfo* pTypeInfo) {
        Value v;
        v.m_allocator = SR_UTILS_NS::IAllocator::GetDefaultAllocator();
        using Type = std::remove_cv_t<std::remove_reference_t<T>>;
        if (sizeof(Type) <= ReflectedValueStorageSize) {
            ReflectedValue::Storage inlineData{};
            new (&inlineData) Type(std::forward<Type>(value));
            v.m_storage = ReflectedValue::MakeFromInlineData(inlineData, ReflectedValueStorageType::Embedded);
        }
        else {
            char* buffer = static_cast<char*>(v.m_allocator->Allocate(sizeof(Type), alignof(Type)));
            new (buffer) Type(std::forward<Type>(value));
            v.m_storage = ReflectedValue::MakeFromPointer(buffer, ReflectedValueStorageType::Dynamic);
        }
        v.m_typeInfo = pTypeInfo;
        return v;
    }

    template<typename T> Value Value::CreateRef(T& value, TypeInfo* pTypeInfo) {
        Value v;
        v.m_allocator = SR_UTILS_NS::IAllocator::GetDefaultAllocator();
        v.m_storage = ReflectedValue::MakeFromPointer(&value, ReflectedValueStorageType::Reference);
        v.m_typeInfo = pTypeInfo;
        return v;
    }

    template<typename T> Value Value::CreateCRef(const T& value, TypeInfo* pTypeInfo) {
        Value v;
        v.m_allocator = SR_UTILS_NS::IAllocator::GetDefaultAllocator();
        v.m_storage = ReflectedValue::MakeFromPointer(const_cast<T*>(&value), ReflectedValueStorageType::ConstReference);
        v.m_typeInfo = pTypeInfo;
        return v;
    }

    template<typename T> T* Value::Cast() {
        return static_cast<T*>(m_storage.GetData());
    }

    template<typename T> const T* Value::Cast() const {
        return static_cast<const T*>(m_storage.GetData());
    }
}

#endif //SR_UTILS_TYPE_TRAITS_VALUE_H
