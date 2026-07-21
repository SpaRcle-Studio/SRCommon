//
// Created by Monika on 21.07.2026.
//

#include <Utils/Reflection/ReflectedType.h>
#include <Utils/Types/Vector.h>
#include <Utils/Types/Optional.h>

namespace SR_UTILS_NS::Reflection {
    ReflectedValue::ReflectedValue() = default;

    ReflectedValue::ReflectedValue(void* pData, ReflectedValueStorageType storageType)
        : pData(pData)
        , storageType(storageType)
    { }

    ReflectedValue::ReflectedValue(uint64_t rawData, ReflectedValueStorageType storageType)
        : rawData(rawData)
        , storageType(storageType)
    { }

    bool TypeInfo::operator==(const TypeInfo& other) const noexcept {
        return type == other.type && detailedType == other.detailedType && pNext == other.pNext;
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

    TypeInfo* AllocateTypeInfo(IAllocator& allocator) {
        auto pTypeInfo = static_cast<TypeInfo*>(allocator.Allocate(sizeof(TypeInfo), alignof(TypeInfo)));
        new (pTypeInfo) TypeInfo();
        return pTypeInfo;
    }

    template<typename T> ReflectedValue ReflectedTypeConstructor(IAllocator& allocator) {
        auto pValue = static_cast<T*>(allocator.Allocate(sizeof(T), alignof(T)));
        new (pValue) T();
        return ReflectedValue(pValue, ReflectedValueStorageType::Embedded);
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

    ReflectedValue ArithmeticTypeConstructor(IAllocator&) {
        return ReflectedValue(uint64_t(), ReflectedValueStorageType::Embedded);
    }

    void ArithmeticTypeCopy(ReflectedValue from, ReflectedValue to) {
        to.rawData = from.rawData;
    }

    void ArithmeticTypeMove(ReflectedValue from, ReflectedValue to) {
        to.rawData = from.rawData;
        from.rawData = uint64_t();
    }

    template<typename T> TypeInfo* DetermineTypeInfoRegistered(IAllocator& allocator) {
        auto pTypeInfo = AllocateTypeInfo(allocator);
        if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, bool> || std::is_same_v<T, void*>) {
            pTypeInfo->type = ReflectedType::Arithmetic;
            pTypeInfo->detailedType = DetermineTypeName<T>::Get();
            pTypeInfo->vtable.pConstructor = &ArithmeticTypeConstructor;
            pTypeInfo->vtable.pDestructor = nullptr;
            pTypeInfo->vtable.pCopy = &ArithmeticTypeCopy;
            pTypeInfo->vtable.pMove = &ArithmeticTypeMove;
        }
        else {
            pTypeInfo->vtable.pConstructor = &ReflectedTypeConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeMove<T>;
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

    bool Test() {
        SR_UTILS_NS::Vector<uint32_t> values = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
        SR_UTILS_NS::Optional<int> optionalValue = 42;
        SR_UTILS_NS::Reflection::ReflectedValue valuesRef {
            &values,
            SR_UTILS_NS::Reflection::ReflectedValueStorageType::Reference
        };
        SR_UTILS_NS::IAllocator* pAllocator = SR_UTILS_NS::IAllocator::GetDefaultAllocator();
        auto&& pTypeInfo1 = SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, values);
        auto&& pTypeInfo2 = SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, optionalValue);
        auto&& pTypeInfo3 = SR_UTILS_NS::Reflection::DetermineTypeInfo(*pAllocator, 2);
        pTypeInfo1->vtable.pResize(valuesRef, 4);
        return true;
    }
    //bool b = Test();
}
