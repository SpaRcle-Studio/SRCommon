//
// Created by Monika on 11.08.2026.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_VTABLE_H
#define SR_ENGINE_COMMON_REFLECTION_VTABLE_H

namespace SR_UTILS_NS::Reflection {
    struct ReflectedValue;

    /// Операции над Vector. Индексы не проверяются, это делает ArrayView
    struct SR_COMMON_DLL_API ArrayVTable {
        SizeType(*pSize)(const void* pArray) = nullptr;
        void*(*pAt)(void* pArray, SizeType index) = nullptr;
        void(*pResize)(void* pArray, SizeType size) = nullptr;
        void(*pReserve)(void* pArray, SizeType capacity) = nullptr;
        void(*pInsert)(void* pArray, SizeType index, const void* pValue) = nullptr;
        void(*pErase)(void* pArray, SizeType index) = nullptr;
        void(*pClear)(void* pArray) = nullptr;
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

    struct SR_COMMON_DLL_API PairVTable {
        using GetPairValue = ReflectedValue(*)(ReflectedValue&, bool isFirst);
        GetPairValue pGetPairValue;
    };

    struct TypeInfo;

    struct SR_COMMON_DLL_API TypeInfoVTable {
        /// any type functions
        using ConstructorFn = ReflectedValue(*)(IAllocator&);
        using DestructorFn = void(*)(IAllocator&, ReflectedValue&);
        using CopyFn = void(*)(const ReflectedValue&, ReflectedValue&);
        using MoveFn = void(*)(ReflectedValue&, ReflectedValue&);
        using SizeOfAlignFn = Pair<SizeType, SizeType>(*)(const TypeInfo&);

        ConstructorFn pConstructor = nullptr;
        DestructorFn pDestructor = nullptr;
        CopyFn pCopy = nullptr;
        MoveFn pMove = nullptr;
        SizeOfAlignFn pSizeOfAlign = nullptr;

        /// for SRClass and other containers (except Vector, Map and Set)
        using GetTypeController = void*(*)(ReflectedValue&);

        ArrayVTable arrayVTable = {};
        MapVTable mapVTable = {};
        PairVTable pairVTable = {};
        GetTypeController pGetTypeController = nullptr;
    };

    struct TypeInfoKey;

    /// vtable для категорий, которые строятся без шаблона конкретного типа: enum по рефлектору, SRClass по фабрике
    extern SR_COMMON_DLL_API bool ResolveBuiltinVTable(const TypeInfoKey& key, TypeInfoVTable& vtable);
}

#endif //SR_ENGINE_COMMON_REFLECTION_VTABLE_H
