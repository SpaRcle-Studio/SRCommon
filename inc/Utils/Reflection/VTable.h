//
// Created by Monika on 11.08.2026.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_VTABLE_H
#define SR_ENGINE_COMMON_REFLECTION_VTABLE_H

namespace SR_UTILS_NS::Reflection {
    struct ReflectedValue;
    struct ReflectedContainerIterator;

    struct IteratorVTable {
        using GetValueFn = ReflectedValue(*)(ReflectedValue&, ReflectedContainerIterator);
        using OffsetFn = ReflectedContainerIterator(*)(ReflectedContainerIterator, int64_t);

        OffsetFn pOffset;
        GetValueFn pGetValue;
    };

    struct SR_COMMON_DLL_API ContainerVTable {
        using BeginFn = ReflectedContainerIterator(*)(ReflectedValue&);
        using EndFn = ReflectedContainerIterator(*)(ReflectedValue&);
        using FindFn = ReflectedContainerIterator(*)(ReflectedValue&, const ReflectedValue&);
        using InsertFn = ReflectedContainerIterator(*)(ReflectedValue&, ReflectedContainerIterator, const ReflectedValue&, const ReflectedValue&);
        using EraseFn = ReflectedContainerIterator(*)(ReflectedValue&, ReflectedContainerIterator);

        using SizeFn = SizeType(*)(ReflectedValue&);
        using ClearFn = void(*)(ReflectedValue&);
        using ResizeFn = void(*)(ReflectedValue&, SizeType, bool reserve);

        BeginFn pBegin;
        EndFn pEnd;
        InsertFn pInsert;
        EraseFn pErase;
        FindFn pFind;

        SizeFn pSize;
        ClearFn pClear;
        ResizeFn pResize;

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

        ContainerVTable containerVTable;
        IteratorVTable iteratorVTable;
        PairVTable pairVTable;
        GetTypeController pGetTypeController;
    };

    extern SR_COMMON_DLL_API void RegisterVTable(TypeInfo& typeInfo);
    extern SR_COMMON_DLL_API void UnregisterVTable(TypeInfo& typeInfo);
    extern SR_COMMON_DLL_API bool FindVTable(TypeInfo& typeInfo);
    extern SR_COMMON_DLL_API uint64_t GetVTableSize();
}

#endif //SR_ENGINE_COMMON_REFLECTION_VTABLE_H
