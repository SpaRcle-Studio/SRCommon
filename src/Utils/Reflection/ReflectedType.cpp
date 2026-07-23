//
// Created by Monika on 21.07.2026.
//

#include <Utils/Reflection/ReflectedType.h>
#include <Utils/Types/Vector.h>
#include <Utils/Types/Optional.h>
#include <Utils/Types/UnicodeString.h>
#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS::Reflection {
    ReflectedValue ReflectedValue::MakeFromPointer(void* pData, ReflectedValueStorageType storageType) {
        ReflectedValue value;
        value.pData = pData;
        value.storageType = storageType;
        return value;
    }

    ReflectedValue ReflectedValue::MakeFromNumeric(uint64_t numeric, ReflectedValueStorageType storageType) {
        ReflectedValue value;
        value.numeric = numeric;
        value.storageType = storageType;
        return value;
    }

    ReflectedValue ReflectedValue::MakeFromInlineData(const ReflectedValue::InlineData& inlineData, ReflectedValueStorageType storageType) {
        ReflectedValue value;
        value.inlineData = inlineData;
        value.storageType = storageType;
        return value;
    }

    bool TypeInfo::operator==(const TypeInfo& other) const noexcept {
        if (category != other.category || detailedType != other.detailedType && detailedSize != other.detailedSize) {
            return false;
        }
        if (pNext && other.pNext) {
            return *pNext == *other.pNext;
        }
        return pNext == other.pNext;
    }

    bool TypeInfo::operator!=(const TypeInfo& other) const noexcept {
        return !(*this == other);
    }

    template<typename T, typename Enable = void> struct DetermineTypeName {
        static StringAtom Get() { return StringAtom(); }
    };

    template<> struct DetermineTypeName<int8_t> { static StringAtom Get() { static StringAtom name = "int8"; return name; } };
    template<> struct DetermineTypeName<int16_t> { static StringAtom Get() { static StringAtom name = "int16"; return name; } };
    template<> struct DetermineTypeName<int32_t> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<int64_t> { static StringAtom Get() { static StringAtom name = "int64"; return name; } };
    template<> struct DetermineTypeName<uint8_t> { static StringAtom Get() { static StringAtom name = "uint8"; return name; } };
    template<> struct DetermineTypeName<uint16_t> { static StringAtom Get() { static StringAtom name = "uint16"; return name; } };
    template<> struct DetermineTypeName<uint32_t> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<uint64_t> { static StringAtom Get() { static StringAtom name = "uint64"; return name; } };
    template<> struct DetermineTypeName<float> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<double> { static StringAtom Get() { static StringAtom name = "double"; return name; } };
    template<> struct DetermineTypeName<bool> { static StringAtom Get() { static StringAtom name = "bool"; return name; } };
    template<> struct DetermineTypeName<Path> { static StringAtom Get() { static StringAtom name = "Path"; return name; } };
    template<> struct DetermineTypeName<String> { static StringAtom Get() { static StringAtom name = "String"; return name; } };
    template<> struct DetermineTypeName<StringView> { static StringAtom Get() { static StringAtom name = "StringView"; return name; } };
    template<> struct DetermineTypeName<StringAtom> { static StringAtom Get() { static StringAtom name = "StringAtom"; return name; } };
    template<> struct DetermineTypeName<UnicodeString> { static StringAtom Get() { static StringAtom name = "UnicodeString"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FColor> { static StringAtom Get() { static StringAtom name = "Color"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::Quaternion> { static StringAtom Get() { static StringAtom name = "Quaternion"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::AABB> { static StringAtom Get() { static StringAtom name = "AABB"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::Matrix3x3> { static StringAtom Get() { static StringAtom name = "Matrix3x3"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::Matrix4x4> { static StringAtom Get() { static StringAtom name = "Matrix4x4"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FRect> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::IRect> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::URect> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::USRect> { static StringAtom Get() { static StringAtom name = "uint16"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FSize> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::USize> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::ISize> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FSize2> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::USize2> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::ISize2> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FVector2> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FVector3> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FVector4> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FVector6> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::IVector2> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::IVector3> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::IVector4> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::IVector6> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::UVector2> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::UVector3> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::UVector4> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::UVector6> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::BVector2> { static StringAtom Get() { static StringAtom name = "bool"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::BVector3> { static StringAtom Get() { static StringAtom name = "bool"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::BVector4> { static StringAtom Get() { static StringAtom name = "bool"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::BVector6> { static StringAtom Get() { static StringAtom name = "bool"; return name; } };

    TypeInfo* AllocateTypeInfo(IAllocator& allocator) {
        auto pTypeInfo = static_cast<TypeInfo*>(allocator.Allocate(sizeof(TypeInfo), alignof(TypeInfo)));
        new (pTypeInfo) TypeInfo();
        return pTypeInfo;
    }

    template<typename T> ReflectedValue ReflectedTypeInlineConstructor(IAllocator&) {
        ReflectedValue reflectedValue;
        new (&reflectedValue.inlineData) T();
        reflectedValue.storageType = ReflectedValueStorageType::Embedded;
        return reflectedValue;
    }

    template<typename T> void ReflectedTypeInlineDestructor(IAllocator&, ReflectedValue value) {
        auto pValue = reinterpret_cast<T*>(&value.inlineData);
        pValue->~T();
    }

    template<typename T> void ReflectedTypeInlineCopy(ReflectedValue from, ReflectedValue to) {
        auto pFrom = reinterpret_cast<T*>(&from.inlineData);
        auto pTo = reinterpret_cast<T*>(&to.inlineData);
        *pTo = *pFrom;
    }

    template<typename T> void ReflectedTypeInlineMove(ReflectedValue from, ReflectedValue to) {
        auto pFrom = reinterpret_cast<T*>(&from.inlineData);
        auto pTo = reinterpret_cast<T*>(&to.inlineData);
        *pTo = std::move(*pFrom);
    }

    template<typename T> ReflectedValue ReflectedTypeConstructor(IAllocator& allocator) {
        auto pValue = static_cast<T*>(allocator.Allocate(sizeof(T), alignof(T)));
        new (pValue) T();
        return ReflectedValue::MakeFromPointer(pValue, ReflectedValueStorageType::Embedded);
    }

    template<typename T> void ReflectedTypeDestructor(IAllocator& allocator, ReflectedValue value) {
        auto pValue = static_cast<T*>(value.pData);
        pValue->~T();
        allocator.Free(pValue, sizeof(T), alignof(T));
    }

    template<typename T> void ReflectedTypeCopy(ReflectedValue from, ReflectedValue to) {
        auto pFrom = static_cast<T*>(from.pData);
        auto pTo = static_cast<T*>(to.pData);
        *pTo = *pFrom;
    }

    template<typename T> void ReflectedTypeMove(ReflectedValue from, ReflectedValue to) {
        auto pFrom = static_cast<T*>(from.pData);
        auto pTo = static_cast<T*>(to.pData);
        *pTo = std::move(*pFrom);
    }

    template<typename T> SizeType ReflectedTypeSize(ReflectedValue value) {
        auto pContainer = static_cast<T*>(value.pData);
        return pContainer->size();
    }

    template<typename T> void ReflectedTypeResize(ReflectedValue value, SizeType size) {
        auto pContainer = static_cast<T*>(value.pData);
        pContainer->resize(size);
    }

    template<typename T> void ReflectedTypeReserve(ReflectedValue value, SizeType size) {
        auto pContainer = static_cast<T*>(value.pData);
        pContainer->reserve(size);
    }

    template<typename T> void ReflectedTypeClear(ReflectedValue value) {
        auto pContainer = static_cast<T*>(value.pData);
        pContainer->clear();
    }

    ReflectedValue ArithmeticTypeConstructor(IAllocator&) {
        return ReflectedValue::MakeFromNumeric(uint64_t(), ReflectedValueStorageType::Embedded);
    }

    void ArithmeticTypeCopy(ReflectedValue from, ReflectedValue to) {
        to.numeric = from.numeric;
    }

    void ArithmeticTypeMove(ReflectedValue from, ReflectedValue to) {
        to.numeric = from.numeric;
        from.numeric = uint64_t();
    }

    template<typename T> TypeInfo* DetermineTypeInfoRegistered(IAllocator& allocator) {
        auto pTypeInfo = AllocateTypeInfo(allocator);
        if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, bool>) {
            pTypeInfo->category = ReflectedCategoryType::Arithmetic;
            pTypeInfo->detailedType = DetermineTypeName<T>::Get();
            pTypeInfo->vtable.pConstructor = &ArithmeticTypeConstructor;
            pTypeInfo->vtable.pCopy = &ArithmeticTypeCopy;
            pTypeInfo->vtable.pMove = &ArithmeticTypeMove;
        }
        else if constexpr (std::is_same_v<T, SR_MATH_NS::Quaternion> || std::is_same_v<T, SR_MATH_NS::Matrix3x3> ||
            std::is_same_v<T, SR_MATH_NS::Matrix4x4> || std::is_same_v<T, SR_MATH_NS::FColor> || std::is_same_v<T, SR_MATH_NS::AABB>
        ) {
            pTypeInfo->category = ReflectedCategoryType::MathObject;
            pTypeInfo->detailedType = DetermineTypeName<T>::Get();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeInlineConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeInlineDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeInlineCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeInlineMove<T>;
        }
        else if constexpr (IsTypeMathRectTemplateV<T>) {
            pTypeInfo->category = ReflectedCategoryType::MathRect;
            pTypeInfo->detailedType = DetermineTypeName<typename T::ValueType>::Get();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeInlineConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeInlineDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeInlineCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeInlineMove<T>;
        }
        else if constexpr (IsTypeMathSizeTemplateV<T>) {
            pTypeInfo->category = ReflectedCategoryType::MathSize;
            pTypeInfo->detailedType = DetermineTypeName<typename T::ValueType>::Get();
            pTypeInfo->detailedSize = T::Dimensions();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeInlineConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeInlineDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeInlineCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeInlineMove<T>;
        }
        else if constexpr (IsTypeMathVectorTemplateV<T>) {
            pTypeInfo->category = ReflectedCategoryType::MathVector;
            pTypeInfo->detailedType = DetermineTypeName<typename T::ValueType>::Get();
            pTypeInfo->detailedSize = T::Dimensions();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeInlineConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeInlineDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeInlineCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeInlineMove<T>;
        }
        else if constexpr (std::is_same_v<T, String> || std::is_same_v<T, StringView> || std::is_same_v<T, StringAtom> || std::is_same_v<T, Path> || std::is_same_v<T, UnicodeString>) {
            pTypeInfo->category = ReflectedCategoryType::String;
            pTypeInfo->detailedType = DetermineTypeName<T>::Get();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeInlineConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeInlineDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeInlineCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeInlineMove<T>;
            pTypeInfo->vtable.pSize = &ReflectedTypeSize<T>;
            if constexpr (std::is_same_v<T, String> || std::is_same_v<T, UnicodeString>) {
                pTypeInfo->vtable.pResize = &ReflectedTypeResize<T>;
                pTypeInfo->vtable.pReserve = &ReflectedTypeReserve<T>;
            }
            pTypeInfo->vtable.pClear = &ReflectedTypeClear<T>;
        }
        else {
            static_assert(AlwaysFalseV<T>, "Type is not registered for reflection");
        }
        return pTypeInfo;
    }

    template TypeInfo* DetermineTypeInfoRegistered<int8_t>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<int16_t>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<int32_t>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<int64_t>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<uint8_t>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<uint16_t>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<uint32_t>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<uint64_t>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<float>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<double>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<bool>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<Path>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<String>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<StringView>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<StringAtom>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<UnicodeString>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FRect>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::IRect>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::URect>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::USRect>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FSize>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::USize>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::ISize>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FSize2>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::USize2>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::ISize2>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FVector2>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FVector3>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FVector4>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FVector6>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::IVector2>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::IVector3>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::IVector4>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::IVector6>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::UVector2>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::UVector3>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::UVector4>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::UVector6>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::BVector2>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::BVector3>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::BVector4>(IAllocator&);
    template TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::BVector6>(IAllocator&);

    bool Test() {
        SR_UTILS_NS::Vector<uint32_t> values = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
        SR_UTILS_NS::Optional<int> optionalValue = 42;
        SR_UTILS_NS::Reflection::ReflectedValue valuesRef {
            &values,
            SR_UTILS_NS::Reflection::ReflectedValueStorageType::Reference
        };
        SR_UTILS_NS::IAllocator* pAllocator = SR_UTILS_NS::IAllocator::GetDefaultAllocator();
        TypeInfo* types[17] {
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, values),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, optionalValue),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, 2),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, true),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, String()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, StringView()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, StringAtom()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, UnicodeString()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, Path()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, SR_MATH_NS::FSize2()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, SR_MATH_NS::FRect()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, SR_MATH_NS::Quaternion()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, SR_MATH_NS::Matrix4x4()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, SR_MATH_NS::UVector6()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, ReflectedCategoryType()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, Serializable()),
            SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, SRClass()),
        };
        types[0]->vtable.pResize(valuesRef, 4);
        return true;
    }
    bool b = Test();
}
