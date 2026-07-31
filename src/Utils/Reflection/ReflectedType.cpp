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
        value.GetData() = pData;
        value.storageType = storageType;
        return value;
    }

    ReflectedValue ReflectedValue::MakeFromNumeric(uint64_t numeric, ReflectedValueStorageType storageType) {
        ReflectedValue value;
        value.GetNumeric() = static_cast<int64_t>(numeric);
        value.storageType = storageType;
        return value;
    }

    ReflectedValue ReflectedValue::MakeFromInlineData(const ReflectedValue::Storage& inlineData, ReflectedValueStorageType storageType) {
        ReflectedValue value;
        value.storage = inlineData;
        value.storageType = storageType;
        return value;
    }

    int64_t& ReflectedValue::GetNumeric() {
        return *std::launder(reinterpret_cast<int64_t*>(storage.data));
    }

    void*& ReflectedValue::GetData() {
        return *std::launder(reinterpret_cast<void**>(storage.data));
    }

    const void* ReflectedValue::GetData() const {
        return *std::launder(reinterpret_cast<void* const*>(storage.data));
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

    TypeInfo* AllocateTypeInfo(IAllocator& allocator, uint16_t count) {
        auto pTypeInfo = static_cast<TypeInfo*>(allocator.Allocate(sizeof(TypeInfo) * count, alignof(TypeInfo)));
        for (uint16_t i = 0; i < count; ++i) {
            new (&pTypeInfo[i]) TypeInfo();
        }
        return pTypeInfo;
    }

    template<typename T> ReflectedValue ReflectedTypeInlineConstructor(IAllocator&) {
        ReflectedValue reflectedValue;
        new (&reflectedValue.storage.data) T();
        reflectedValue.storageType = ReflectedValueStorageType::Embedded;
        return reflectedValue;
    }

    template<typename T> void ReflectedTypeInlineDestructor(IAllocator&, ReflectedValue& value) {
        auto pValue = reinterpret_cast<T*>(&value.GetData());
        pValue->~T();
    }

    template<typename T> void ReflectedTypeInlineCopy(ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = reinterpret_cast<T*>(&from.GetData());
        auto pTo = reinterpret_cast<T*>(&to.GetData());
        *pTo = *pFrom;
    }

    template<typename T> void ReflectedTypeInlineMove(ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = reinterpret_cast<T*>(&from.GetData());
        auto pTo = reinterpret_cast<T*>(&to.GetData());
        *pTo = std::move(*pFrom);
    }

    template<typename T> ReflectedValue ReflectedTypeConstructor(IAllocator& allocator) {
        auto pValue = static_cast<T*>(allocator.Allocate(sizeof(T), alignof(T)));
        new (pValue) T();
        return ReflectedValue::MakeFromPointer(pValue, ReflectedValueStorageType::Embedded);
    }

    template<typename T> void ReflectedTypeDestructor(IAllocator& allocator, ReflectedValue& value) {
        auto pValue = static_cast<T*>(value.GetData());
        pValue->~T();
        allocator.Free(pValue, sizeof(T), alignof(T));
    }

    template<typename T> void ReflectedTypeCopy(ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = static_cast<T*>(from.GetData());
        auto pTo = static_cast<T*>(to.GetData());
        *pTo = *pFrom;
    }

    template<typename T> void ReflectedTypeMove(ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = static_cast<T*>(from.GetData());
        auto pTo = static_cast<T*>(to.GetData());
        *pTo = std::move(*pFrom);
    }

    template<typename T> SizeType ReflectedTypeSize(ReflectedValue& value) {
        auto pContainer = static_cast<T*>(value.GetData());
        return pContainer->size();
    }

    template<typename T> void ReflectedTypeResize(ReflectedValue& value, SizeType size, bool reserve) {
        auto pContainer = static_cast<T*>(value.GetData());
        if (reserve)  {
            pContainer->reserve(size);
        }
        else {
            pContainer->resize(size);
        }
    }

    template<typename T> void ReflectedTypeClear(ReflectedValue& value) {
        auto pContainer = static_cast<T*>(value.GetData());
        pContainer->clear();
    }

    ReflectedValue ArithmeticTypeConstructor(IAllocator&) {
        return ReflectedValue::MakeFromNumeric(uint64_t(), ReflectedValueStorageType::Embedded);
    }

    void ArithmeticTypeCopy(ReflectedValue& from, ReflectedValue& to) {
        to.GetNumeric() = from.GetNumeric();
    }

    void ArithmeticTypeMove(ReflectedValue& from, ReflectedValue& to) {
        to.GetNumeric() = from.GetNumeric();
        from.GetNumeric() = uint64_t();
    }

    template<typename T> void DetermineTypeInfoRegistered(IAllocator& allocator, TypeInfo* pTypeInfo) {
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
            pTypeInfo->vtable.containerVTable.pSize = &ReflectedTypeSize<T>;
            if constexpr (std::is_same_v<T, String> || std::is_same_v<T, UnicodeString>) {
                pTypeInfo->vtable.containerVTable.pResize = &ReflectedTypeResize<T>;
            }
            pTypeInfo->vtable.containerVTable.pClear = &ReflectedTypeClear<T>;
        }
        else {
            static_assert(AlwaysFalseV<T>, "Type is not registered for reflection");
        }
    }

    template void DetermineTypeInfoRegistered<int8_t>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<int16_t>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<int32_t>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<int64_t>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<uint8_t>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<uint16_t>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<uint32_t>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<uint64_t>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<float>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<double>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<bool>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<Path>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<String>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<StringView>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<StringAtom>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<UnicodeString>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FRect>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IRect>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::URect>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USRect>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FSize>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USize>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::ISize>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FSize2>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USize2>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::ISize2>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector2>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector3>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector4>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector6>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector2>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector3>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector4>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector6>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector2>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector3>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector4>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector6>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector2>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector3>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector4>(IAllocator&, TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector6>(IAllocator&, TypeInfo* pTypeInfo);

    bool Test() {
        Vector<uint32_t> values = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
        Optional<int> optionalValue = 42;
        auto valuesRef = Reflection::ReflectedValue::MakeFromPointer(
            &values,
            Reflection::ReflectedValueStorageType::Reference
        );
        Set<String> stringSet = { "Hello", "World", "!" };
        Map<String, int> stringMap = { {"One", 1}, {"Two", 2}, {"Three", 3} };
        auto mapRef = Reflection::ReflectedValue::MakeFromPointer(
            &stringMap,
            Reflection::ReflectedValueStorageType::Reference
        );

        String string = "Two";

        IAllocator* pAllocator = IAllocator::GetDefaultAllocator();
        TypeInfo* types[19] {
            DetermineTypeInfoAlloc(*pAllocator, values),
            DetermineTypeInfoAlloc(*pAllocator, optionalValue),
            DetermineTypeInfoAlloc(*pAllocator, 2),
            DetermineTypeInfoAlloc(*pAllocator, true),
            DetermineTypeInfoAlloc(*pAllocator, String()),
            DetermineTypeInfoAlloc(*pAllocator, StringView()),
            DetermineTypeInfoAlloc(*pAllocator, StringAtom()),
            DetermineTypeInfoAlloc(*pAllocator, UnicodeString()),
            DetermineTypeInfoAlloc(*pAllocator, Path()),
            DetermineTypeInfoAlloc(*pAllocator, SR_MATH_NS::FSize2()),
            DetermineTypeInfoAlloc(*pAllocator, SR_MATH_NS::FRect()),
            DetermineTypeInfoAlloc(*pAllocator, SR_MATH_NS::Quaternion()),
            DetermineTypeInfoAlloc(*pAllocator, SR_MATH_NS::Matrix4x4()),
            DetermineTypeInfoAlloc(*pAllocator, SR_MATH_NS::UVector6()),
            DetermineTypeInfoAlloc(*pAllocator, ReflectedCategoryType()),
            DetermineTypeInfoAlloc(*pAllocator, Serializable()),
            DetermineTypeInfoAlloc(*pAllocator, SRClass()),
            DetermineTypeInfoAlloc(*pAllocator, stringSet),
            DetermineTypeInfoAlloc(*pAllocator, stringMap)
        };
        types[0]->vtable.containerVTable.pResize(valuesRef, 4, false);
        ReflectedContainerIterator mapIt = types[18]->vtable.containerVTable.pFind(mapRef, ReflectedValue::MakeFromPointer(&string, ReflectedValueStorageType::Reference));
        ReflectedValue mapValue = types[18]->vtable.containerVTable.pGetValue(mapRef, mapIt);
        auto value = *static_cast<Pair<String, int>*>(mapValue.GetData());
        return true;
    }
    bool b = Test();
}
