//
// Created by Monika on 21.07.2026.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H
#define SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H

#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    class EntityRefBase;
    class OptionalBase;
    class ResourceRefBase;
    template<typename T> class Vector;
    template<typename T> class Optional;
    template<typename T> class EntityRef;
    template<typename T> class ResourceRef;
    template<typename T1, typename T2> struct Pair;
    class UnicodeString;
    class Path;
}

namespace SR_MATH_NS {
    template<typename T> struct Vector2;
    template<typename T> struct Vector3;
    template<typename T> struct Vector4;
    template<typename T> struct Vector6;
    template<typename T> struct Rect;
    template<typename T> struct Size;
    template<typename T> struct Size2;
    struct AABB;
    class Quaternion;
    class Matrix3x3;
    class Matrix4x4;
    class FColor;
}

namespace SR_HTYPES_NS {
    template<typename T> class SharedPtr;
}

namespace SR_UTILS_NS::Reflection {
    SR_ENUM_NS_CLASS_T(ReflectedCategoryType, uint8_t,
        Unknown,

        Object,
        String,
        Arithmetic,
        Enum,

        MathObject,
        MathSize,
        MathVector,
        MathRect,

        Container
    );

    enum class ReflectedValueStorageType : uint8_t {
        Embedded, Reference, ConstReference,
    };

    struct SR_COMMON_DLL_API ReflectedValue {
        struct Storage { alignas(std::max_align_t) std::byte data[64]; };
        static ReflectedValue MakeFromPointer(void* pData, ReflectedValueStorageType storageType);
        static ReflectedValue MakeFromNumeric(uint64_t numeric, ReflectedValueStorageType storageType);
        static ReflectedValue MakeFromInlineData(const Storage& inlineData, ReflectedValueStorageType storageType);

        SR_NODISCARD int64_t& GetNumeric();
        SR_NODISCARD void*& GetData();
        SR_NODISCARD const void* GetData() const;

        Storage storage = {};
        ReflectedValueStorageType storageType = ReflectedValueStorageType::Embedded;
    };

    struct SR_COMMON_DLL_API ReflectedContainerIterator {
        char data[16] = {}; /// с запасом под итераторы Map/Set, где хранится 2 указателя
    };

    struct SR_COMMON_DLL_API ContainerVTable {
        using BeginFn = ReflectedContainerIterator(*)(ReflectedValue&);
        using EndFn = ReflectedContainerIterator(*)(ReflectedValue&);
        using FindFn = ReflectedContainerIterator(*)(ReflectedValue&, const ReflectedValue&);
        using InsertFn = ReflectedContainerIterator(*)(ReflectedValue&, ReflectedContainerIterator, const ReflectedValue&, const ReflectedValue&);
        using EraseFn = ReflectedContainerIterator(*)(ReflectedValue&, ReflectedContainerIterator);
        using GetValueFn = ReflectedValue(*)(ReflectedValue&, ReflectedContainerIterator);

        using IteratorOffsetFn = ReflectedContainerIterator(*)(ReflectedContainerIterator, int64_t);
        using IteratorDistanceFn = int64_t(*)(ReflectedContainerIterator, ReflectedContainerIterator);

        using SizeFn = SizeType(*)(ReflectedValue&);
        using ClearFn = void(*)(ReflectedValue&);
        using ResizeFn = void(*)(ReflectedValue&, SizeType, bool reserve);

        BeginFn pBegin = nullptr;
        EndFn pEnd = nullptr;
        InsertFn pInsert = nullptr;
        EraseFn pErase = nullptr;
        GetValueFn pGetValue = nullptr;
        FindFn pFind = nullptr;

        SizeFn pSize = nullptr;
        ClearFn pClear = nullptr;
        ResizeFn pResize = nullptr;

    };

    struct SR_COMMON_DLL_API TypeInfoVTable {
        /// any type functions
        using ConstructorFn = ReflectedValue(*)(IAllocator&);
        using DestructorFn = void(*)(IAllocator&, ReflectedValue&);
        using CopyFn = void(*)(ReflectedValue&, ReflectedValue&);
        using MoveFn = void(*)(ReflectedValue&, ReflectedValue&);

        ConstructorFn pConstructor = nullptr;
        DestructorFn pDestructor = nullptr;
        CopyFn pCopy = nullptr;
        MoveFn pMove = nullptr;

        /// for SRClass and other containers (except Vector, Map and Set)
        using GetTypeController = void*(*)(ReflectedValue&);

        GetTypeController pGetTypeController = nullptr;

        ContainerVTable containerVTable;

    };

    struct SR_COMMON_DLL_API TypeInfo {
        ReflectedCategoryType category = ReflectedCategoryType::Unknown;
        uint8_t detailedSize = 0;
        StringAtom detailedType;
        TypeInfo* pNext = nullptr;
        TypeInfoVTable vtable;

        bool operator==(const TypeInfo& other) const noexcept;
        bool operator!=(const TypeInfo& other) const noexcept;
    };

    extern SR_COMMON_DLL_API TypeInfo* AllocateTypeInfo(IAllocator& allocator, uint16_t count);

    template<typename T, typename Enable = void> struct DetermineTypeInfoAccessor {
        static void Determine(IAllocator&, TypeInfo*) { static_assert(AlwaysFalseV<T>, "Unable to determine type info for type!"); }
    };

    template<typename T> void DetermineTypeInfo(IAllocator& allocator, TypeInfo* pTypeInfo, const T&) {
        DetermineTypeInfoAccessor<T>::Determine(allocator, pTypeInfo);
    }

    template<typename T> TypeInfo* DetermineTypeInfoAlloc(IAllocator& allocator, const T& t) {
        auto&& pTypeInfo = AllocateTypeInfo(allocator, 1);
        DetermineTypeInfo(allocator, pTypeInfo, t);
        return pTypeInfo;
    }

    template<typename T> void ReflectedTypeContainerClear(ReflectedValue& value) {
        auto pContainer = static_cast<T*>(value.GetData());
        pContainer->clear();
    }

    template<typename T> SizeType ReflectedTypeContainerSize(ReflectedValue& value) {
        auto pContainer = static_cast<T*>(value.GetData());
        return pContainer->size();
    }

    template<typename T> ReflectedContainerIterator ReflectedTypeContainerBegin(ReflectedValue& value) {
        auto pContainer = static_cast<T*>(value.GetData());
        ReflectedContainerIterator it;
        auto tmpIt = pContainer->begin();
        memcpy(&it.data, &tmpIt, sizeof(typename T::Iterator));
        return it;
    }

    template<typename T> ReflectedContainerIterator ReflectedTypeContainerEnd(ReflectedValue& value) {
        auto pContainer = static_cast<T*>(value.GetData());
        ReflectedContainerIterator it;
        auto tmpIt = pContainer->end();
        memcpy(&it.data, &tmpIt, sizeof(typename T::Iterator));
        return it;
    }

    template<typename T> ReflectedValue ReflectedTypeContainerIteratorGetValue(ReflectedValue& value, ReflectedContainerIterator iterator) {
        typename T::Iterator it;
        memcpy(&it, &iterator.data, sizeof(typename T::Iterator));
        ReflectedValue reflectedValue;
        auto& itValue = *it;
        reflectedValue.GetData() = (void*)(&itValue);
        reflectedValue.storageType = value.storageType == ReflectedValueStorageType::ConstReference ?
            ReflectedValueStorageType::ConstReference :
            ReflectedValueStorageType::Reference;
        return reflectedValue;
    }

    template<typename T> ReflectedContainerIterator ReflectedTypeContainerErase(ReflectedValue& value, ReflectedContainerIterator iterator) {
        auto pContainer = static_cast<T*>(value.GetData());
        typename T::Iterator it;
        memcpy(&it, &iterator.data, sizeof(typename T::Iterator));
        auto newIt = pContainer->erase(it);
        ReflectedContainerIterator newIterator;
        memcpy(&newIterator.data, &newIt, sizeof(typename T::Iterator));
        return newIterator;
    }
}

#include <Utils/Reflection/CommonReflection.inl.h>
#include <Utils/Reflection/VectorReflection.inl.h>
#include <Utils/Reflection/AssociaticeReflection.inl.h>

#endif //SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H
