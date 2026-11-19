//
// Created by Monika on 20.01.2025.
//

#ifndef SR_UTILS_TYPE_TRAITS_VALUE_H
#define SR_UTILS_TYPE_TRAITS_VALUE_H

#include <Utils/Reflection/ValueRef.h>

// #define SR_COMMON_USE_ENTT

namespace SR_UTILS_NS::Reflection {
    enum class ReflectedCategoryType : uint8_t;

    class SR_COMMON_DLL_API SR_NODISCARD Value {
    public:
        Value();
        Value(const Value& other);
        Value(Value&& other) noexcept;
        Value& operator=(const Value& other) noexcept;
        Value& operator=(Value&& other) noexcept;
        ~Value();

        template<typename T> static Value Create(T&& value, const TypeInfo* pTypeInfo);
        template<typename T> static Value CreateRef(T& value, const TypeInfo* pTypeInfo);
        template<typename T> static Value CreateCRef(const T& value, const TypeInfo* pTypeInfo);

        template<typename T> static Value Create(T&& value);
        template<typename T> static Value CreateRef(T& value);
        template<typename T> static Value CreateCRef(const T& value);

        template<typename T> const T* Cast() const;
        template<typename T> T* Cast();
        template<typename T> Value CastSRClass() const;

        SR_NODISCARD static Value CreateDefault(const TypeInfo* pTypeInfo);

        /// Константность ссылки берётся из хранилища (CreateCRef), а не из квалификатора Value
        SR_NODISCARD ValueRef AsRef() const;
        SR_NODISCARD ArrayView AsArray() const;
        SR_NODISCARD MapView AsMap() const;

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
        const TypeInfo* m_typeInfo = nullptr;
        IAllocator* m_allocator = nullptr;

    };

    /// ----------------------------------------------------------------------------------------------------------------

    template<typename T> Value Value::Create(T&& value) {
        return Create(std::forward<T>(value), TypeOf<T>());
    }

    template<typename T> Value Value::CreateRef(T& value) {
        return CreateRef(value, TypeOf<T>());
    }

    template<typename T> Value Value::CreateCRef(const T& value) {
        return CreateCRef(value, TypeOf<T>());
    }

    template<typename T> Value Value::Create(T&& value, const TypeInfo* pTypeInfo) {
        if constexpr (std::is_lvalue_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>) {
            return CreateCRef(value, pTypeInfo);
        }
        else if constexpr (std::is_lvalue_reference_v<T>) {
            return CreateRef(value, pTypeInfo);
        }
        else {
            Value v;
            v.m_allocator = SR_UTILS_NS::IAllocator::GetDefaultAllocator();
            using Type = std::remove_cv_t<std::remove_reference_t<T>>;
            if constexpr (IsReflectedValueEmbeddable<Type>) {
                ReflectedValue::Storage inlineData{};
                new (&inlineData) Type(std::forward<T>(value));
                v.m_storage = ReflectedValue::MakeFromInlineData(inlineData, ReflectedValueStorageType::Embedded);
            }
            else {
                char* buffer = static_cast<char*>(v.m_allocator->Allocate(sizeof(Type), alignof(Type)));
                new (buffer) Type(std::forward<T>(value));
                v.m_storage = ReflectedValue::MakeFromPointer(buffer, ReflectedValueStorageType::Dynamic);
            }
            v.m_typeInfo = pTypeInfo;
            return v;
        }
    }

    template<typename T> Value Value::CreateRef(T& value, const TypeInfo* pTypeInfo) {
        Value v;
        v.m_allocator = SR_UTILS_NS::IAllocator::GetDefaultAllocator();
        v.m_storage = ReflectedValue::MakeFromPointer(&value, ReflectedValueStorageType::Reference);
        v.m_typeInfo = pTypeInfo;
        return v;
    }

    template<typename T> Value Value::CreateCRef(const T& value, const TypeInfo* pTypeInfo) {
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

    template<typename T> Value Value::CastSRClass() const {
        if (!IsValid()) {
            return Value();
        }

        auto&& typeInfo = GetTypeInfo();
        if (typeInfo.kind == TypeKind::SharedPtr) {
            auto&& pCasted = dynamic_cast<T*>(GetSRClass());
            if (!pCasted) {
                return Value();
            }
            return Value::Create(SR_HTYPES_NS::SharedPtr(pCasted));
        }
        else if (typeInfo.kind == TypeKind::Object) {
            auto&& pCasted = dynamic_cast<T*>(GetSRClass());
            if (!pCasted) {
                return Value();
            }
            if (IsRef()) {
                return Value::CreateRef(*pCasted);
            }
            return Value::Create(*pCasted);
        }

        return Value();
    }
}

#endif //SR_UTILS_TYPE_TRAITS_VALUE_H
