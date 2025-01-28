//
// Created by Monika on 20.01.2025.
//

#ifndef SR_UTILS_TYPE_TRAITS_VALUE_H
#define SR_UTILS_TYPE_TRAITS_VALUE_H

#include <Utils/Common/Enumerations.h>
#include <Utils/Common/TypeInfo.h>
#include <Utils/Types/StringAtom.h>

namespace SR_UTILS_NS::Reflection {
    /*SR_ENUM_NS_CLASS_T(ValueTypeKind, uint8_t,
        None,
        /// trivial types
        Bool,
        Int8, Int16, Int32, Int64,
        UInt8, UInt16, UInt32, UInt64,
        Float, Double,
        /// strings
        String, StringAtom,
        /// containers
        Vector, Map, Set,
        /// reflected enums
        Enum,

        WeakPtr, StrongPtr,
        Object
    );*/

    class Value {
        using DeleterFn = void(*)(void*);
        using CopyFn = void(*)(void*& pDstRef, const void* pSource);
    public:
        Value() = default;
        ~Value();

        Value(const Value& other);
        Value(Value&& other) noexcept;
        Value& operator=(const Value& other);
        Value& operator=(Value&& other) noexcept;

        template<class T> static Value Create(T&& value, bool isConst = false);
        template<class T> static Value CreateReference(T& value, bool isConst = false);

        template<class T> static DeleterFn GetDeleter();

        template<class T> bool Map(T*& pValue) const;
        template<class T> T* Map() const;
        const char* MapString() const { return static_cast<const char*>(m_data); }
        char* MapString(){ return static_cast<char*>(m_data); }

        SR_NODISCARD StandardType GetType() const { return m_type; }
        SR_NODISCARD Value Clone() const;

    private:
        void Destroy();

    private:
        void* m_data = nullptr;
        DeleterFn m_deleter = nullptr;
        CopyFn m_copier = nullptr;
        StandardType m_type = StandardType::Unknown;
        bool m_isReference = false;
        bool m_isConst = false;
    };

    /// Implementation

    template<class T> Value Value::Create(T&& value, bool isConst)  {
        Value result;

        constexpr StandardType type = GetStandardType<T>();
        if constexpr (static_cast<uint16_t>(type) == static_cast<uint16_t>(StandardType::Unknown)) {
            static_assert(AlwaysFalseV<T>, "Unknown type!");
        }

        result.m_type = type;
        result.m_isReference = false;
        result.m_isConst = isConst;

        using CopiedType = SR_UTILS_NS::RemoveQualifiersT<T>;

        result.m_data = const_cast<void*>(static_cast<const void*>(new CopiedType(std::forward<T>(value))));
        result.m_deleter = GetDeleter<T>();

        result.m_copier = [](void*& pDstRef, const void* pSource) {
            pDstRef = new CopiedType(*static_cast<const CopiedType*>(pSource));
        };

        return result;
    }

    template<class T> Value Value::CreateReference(T& value, bool isConst) {
        Value result;

        constexpr StandardType type = GetStandardType<T>();
        if constexpr (static_cast<uint16_t>(type) == static_cast<uint16_t>(StandardType::Unknown)) {
            static_assert(AlwaysFalseV<T>, "Unknown type!");
        }

        result.m_type = type;

        result.m_isReference = true;
        result.m_isConst = isConst;

        result.m_data = const_cast<void*>(static_cast<const void*>(&value));
        result.m_deleter = GetDeleter<T>();

        using CopiedType = SR_UTILS_NS::RemoveQualifiersT<T>;
        result.m_copier = [](void*& pDstRef, const void* pSource) {
            pDstRef = new CopiedType(*static_cast<const CopiedType*>(pSource));
        };

        return result;
    }

    template<typename T> Value::DeleterFn Value::GetDeleter() {
        using CopiedType = SR_UTILS_NS::RemoveQualifiersT<T>;
        return [](void* pData) {
            delete static_cast<CopiedType*>(pData);
        };
    }

    template<class T> bool Value::Map(T*& pValue) const {
        if (m_type != GetStandardType<T>()) {
            return false;
        }

        pValue = static_cast<T*>(m_data);
        return true;
    }

    template<class T> T* Value::Map() const {
        T* pValue = nullptr;
        if (!Map(pValue)) {
            return nullptr;
        }
        return pValue;
    }
}

#endif //SR_UTILS_TYPE_TRAITS_VALUE_H
