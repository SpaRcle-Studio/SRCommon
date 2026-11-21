//
// Created by Monika on 11.08.2026.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_VTABLE_H
#define SR_ENGINE_COMMON_REFLECTION_VTABLE_H

#include <Utils/Types/StringAtom.h>

namespace SR_UTILS_NS::Reflection {
    struct ReflectedValue;
    struct TypeInfo;
    struct FieldInfo;
    class ValueRef;

    struct SR_COMMON_DLL_API LifecycleVTable {
        SizeType size = 0;
        SizeType alignment = 0;
        bool isTriviallyCopyable = false;
        bool isTriviallyDestructible = false;
        bool isTriviallyRelocatable = false;
        bool isNothrowMoveConstructible = false;
        void(*pConstructAt)(const TypeInfo&, void*) = nullptr;
        void(*pCopyConstructAt)(const TypeInfo&, void*, const void*) = nullptr;
        void(*pMoveConstructAt)(const TypeInfo&, void*, void*) = nullptr;
        void(*pCopyAssign)(const TypeInfo&, void*, const void*) = nullptr;
        void(*pDestructAt)(const TypeInfo&, void*) = nullptr;
    };

    struct TypeInfoVTable;
    template<typename T, bool AllowCopy = true, bool AllowMove = true> void FillLifecycle(TypeInfoVTable& vtable);

    /// Операции над Vector. Индексы не проверяются, это делает ArrayView
    struct SR_COMMON_DLL_API ArrayVTable {
        SizeType(*pSize)(const TypeInfo& arrayType, const void* pArray) = nullptr;
        void*(*pAt)(const TypeInfo& arrayType, void* pArray, SizeType index) = nullptr;
        void(*pResize)(const TypeInfo& arrayType, void* pArray, SizeType size) = nullptr;
        void(*pReserve)(const TypeInfo& arrayType, void* pArray, SizeType capacity) = nullptr;
        void(*pInsert)(const TypeInfo& arrayType, void* pArray, SizeType index, const void* pValue) = nullptr;
        void(*pErase)(const TypeInfo& arrayType, void* pArray, SizeType index) = nullptr;
        void(*pClear)(const TypeInfo& arrayType, void* pArray) = nullptr;
    };

    /// Операции над Map, FlatHashMap, Set и FlatHashSet.
    /// У множеств нет значения: pValue в pVisit равен nullptr, pFind и pFindOrAdd возвращают сам ключ.
    struct SR_COMMON_DLL_API MapVTable {
        SizeType(*pSize)(const void* pMap) = nullptr;
        void(*pClear)(void* pMap) = nullptr;
        void*(*pFind)(void* pMap, const void* pKey) = nullptr;
        void*(*pFindOrAdd)(void* pMap, const void* pKey) = nullptr;
        /// true, если ключ добавлен; у существующего ключа значение перезаписывается
        bool(*pInsertOrAssign)(void* pMap, const void* pKey, const void* pValue) = nullptr;
        bool(*pErase)(void* pMap, const void* pKey) = nullptr;
        /// pVisit возвращает false, чтобы остановить обход
        void(*pForEach)(void* pMap, bool(*pVisit)(void* pUserData, const void* pKey, void* pValue), void* pUserData) = nullptr;
    };

    struct SR_COMMON_DLL_API TypeInfoVTable {
        /// any type functions
        using SizeOfAlignFn = Pair<SizeType, SizeType>(*)(const TypeInfo&);
        LifecycleVTable lifecycle;
        SizeOfAlignFn pSizeOfAlign = nullptr;

        /// for SRClass and other containers (except Vector, Map and Set)
        using GetTypeController = void*(*)(ReflectedValue&);

        ArrayVTable arrayVTable = {};
        MapVTable mapVTable = {};
        const FieldInfo* pFields = nullptr;
        uint32_t fieldsCount = 0;
        GetTypeController pGetTypeController = nullptr;
        void* pRuntimeData = nullptr;
        StringAtom displayName;
        uint32_t(*pEnumCount)(const TypeInfo&) = nullptr;
        StringAtom(*pEnumValue)(const TypeInfo&, uint32_t, int64_t&) = nullptr;
        uint32_t(*pFieldCount)(const TypeInfo&, const void*) = nullptr;
        StringAtom(*pFieldName)(const TypeInfo&, const void*, uint32_t) = nullptr;
        ValueRef(*pField)(const TypeInfo&, void*, uint32_t, bool) = nullptr;
        bool(*pIsNull)(const TypeInfo&, const void*) = nullptr;
        void(*pCreate)(const TypeInfo&, void*) = nullptr;
    };

    struct TypeInfoKey;

    /// vtable для категорий, которые строятся без шаблона конкретного типа: enum по рефлектору, SRClass по фабрике
    extern SR_COMMON_DLL_API bool ResolveBuiltinVTable(const TypeInfoKey& key, TypeInfoVTable& vtable);
}

#endif //SR_ENGINE_COMMON_REFLECTION_VTABLE_H
