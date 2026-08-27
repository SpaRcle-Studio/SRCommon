//
// Created by Monika on 11.08.2026.
//

#include <Utils/Reflection/VTable.h>
#include <Utils/Reflection/ReflectedType.h>
#include <Utils/TypeTraits/Factory.h>

namespace SR_UTILS_NS::Reflection {
    struct TypeInfoVTableCounter {
        TypeInfoVTable vtable;
        uint32_t count = 0;
    };
    SR_HTYPES_NS::FlatHashMap<SRHashType, TypeInfoVTableCounter> g_typeInfoVTableMap;

    void RegisterVTable(TypeInfo& typeInfo) {
        const auto hash = typeInfo.GetHash();
        auto&& table = g_typeInfoVTableMap[hash];
        if (table.count == 0) {
            table.vtable = typeInfo.vtable;
        }
        table.count++;
    }

    void UnregisterVTable(TypeInfo& typeInfo) {
        const auto hash = typeInfo.GetHash();
        auto&& pIt = g_typeInfoVTableMap.find(hash);
        if (pIt == g_typeInfoVTableMap.end()) {
            SRHalt("UnregisterVTable() : TypeInfoVTable for type '{}' is not registered!", typeInfo.detailedType);
            return;
        }
        auto&& table = pIt->second;
        if (--table.count == 0) {
            g_typeInfoVTableMap.erase(pIt);
        }
    }

    Pair<SizeType, SizeType> ReflectedTypeTemplateSizeOfAlignEnum(const TypeInfo& typeInfo) {
        auto&& pReflector = SR_UTILS_NS::EnumReflectorManager::Instance().GetReflector(typeInfo.detailedType);
        if (!pReflector) {
            SRHalt("ReflectedTypeTemplateSizeOfAlignEnum() : Enum reflector for type '{}' is not registered!", typeInfo.detailedType);
            return { 0, 0 };
        }
        switch (pReflector->GetIntegralTypeSizeInternal()) {
            case 1: return { sizeof(uint8_t), alignof(uint8_t) };
            case 2: return { sizeof(uint16_t), alignof(uint16_t) };
            case 4: return { sizeof(uint32_t), alignof(uint32_t) };
            case 8: return { sizeof(uint64_t), alignof(uint64_t) };
            default:
                SRHalt("ReflectedTypeTemplateSizeOfAlignEnum() : Enum reflector for type '{}' has unknown integral type size {}!", typeInfo.detailedType, pReflector->GetIntegralTypeSizeInternal());
                return { 0, 0 };
        }
    }

    using DetermineFn = void(*)(TypeInfo*);
    Map<StringAtom, DetermineFn> g_determineTypeInfoMap = {
        { "Path", &DetermineTypeInfo<Path> },
        { "String", &DetermineTypeInfo<String> },
        { "StringView", &DetermineTypeInfo<StringView> },
        { "StringAtom", &DetermineTypeInfo<StringAtom> },
        { "UnicodeString", &DetermineTypeInfo<UnicodeString> }
    };

    using ArithmeticSizeOfAlignFn = Pair<SizeType, SizeType>(*)(const TypeInfo&);
    Map<StringAtom, ArithmeticSizeOfAlignFn> g_arithmeticSizeOfAlignMap = {
        { "int8", &ReflectedTypeTemplateSizeOfAlign<int8_t> },
        { "int16", &ReflectedTypeTemplateSizeOfAlign<int16_t> },
        { "int32", &ReflectedTypeTemplateSizeOfAlign<int32_t> },
        { "int64", &ReflectedTypeTemplateSizeOfAlign<int64_t> },
        { "uint8", &ReflectedTypeTemplateSizeOfAlign<uint8_t> },
        { "uint16", &ReflectedTypeTemplateSizeOfAlign<uint16_t> },
        { "uint32", &ReflectedTypeTemplateSizeOfAlign<uint32_t> },
        { "uint64", &ReflectedTypeTemplateSizeOfAlign<uint64_t> },
        { "float", &ReflectedTypeTemplateSizeOfAlign<float> },
        { "double", &ReflectedTypeTemplateSizeOfAlign<double> },
        { "bool", &ReflectedTypeTemplateSizeOfAlign<bool> }
    };

    bool FindVTable(TypeInfo& typeInfo) {
        if (typeInfo.category == ReflectedCategoryType::Enum || typeInfo.category == ReflectedCategoryType::Arithmetic) {
            TypeInfoVTable table;
            table.pCopy = &ArithmeticTypeCopy;
            table.pMove = &ArithmeticTypeMove;
            table.pConstructor = &ArithmeticTypeConstructor;
            if (typeInfo.category == ReflectedCategoryType::Enum) {
                table.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlignEnum;
            }
            else {
                if (auto pIt = g_arithmeticSizeOfAlignMap.find(typeInfo.detailedType); pIt != g_arithmeticSizeOfAlignMap.end()) {
                    table.pSizeOfAlign = pIt->second;
                }
                else {
                    return false;
                }
            }
            typeInfo.vtable = table;
            return true;
        }
        if (typeInfo.category == ReflectedCategoryType::Object) {
            if (auto&& pType = Factory::Instance().GetType(typeInfo.detailedType)) {
                typeInfo.vtable = pType->GetVTable();
                return true;
            }
            return false;
        }
        if (typeInfo.category == ReflectedCategoryType::String) {
            auto&& pTypeInfo = AllocateTypeInfo();
            if (auto pIt = g_determineTypeInfoMap.find(typeInfo.detailedType); pIt != g_determineTypeInfoMap.end()) {
                pIt->second(pTypeInfo);
            }
            else {
                FreeTypeInfo(pTypeInfo);
                return false;
            }
            typeInfo.vtable = pTypeInfo->vtable;
            FreeTypeInfo(pTypeInfo);
            return true;
        }
        const auto hash = typeInfo.GetHash();
        auto pIt = g_typeInfoVTableMap.find(hash);
        if (pIt == g_typeInfoVTableMap.end()) {
            return false;
        }
        typeInfo.vtable = pIt->second.vtable;
        return true;
    }

    uint64_t GetVTableSize() {
        return g_typeInfoVTableMap.size();
    }
}