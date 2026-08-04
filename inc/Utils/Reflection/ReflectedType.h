//
// Created by Monika on 21.07.2026.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H
#define SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H

#include <Utils/Common/Enumerations.h>
#include <Utils/TypeTraits/SRClass.h>
#include <Utils/TypeTraits/SRClassMeta.h>

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

    template<typename>
    inline constexpr bool IsEntityRefV = false;

    template<typename U>
    inline constexpr bool IsEntityRefV<EntityRef<U>> = true;

    template<typename>
    inline constexpr bool IsResourceRefV = false;

    template<typename U>
    inline constexpr bool IsResourceRefV<ResourceRef<U>> = true;
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
    template<typename Key> class FlatHashSet;
    template<typename Key, typename Value> class FlatHashMap;
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
        Embedded, Dynamic, Reference, ConstReference,
    };

    extern SR_COMMON_DLL_API bool IsReflectedTypeSigned(StringView type);
    extern SR_COMMON_DLL_API bool IsReflectedTypeIntegral(StringView type);
    extern SR_COMMON_DLL_API SizeType GetReflectedTypeSize(StringView type);

    constexpr inline SizeType ReflectedValueStorageSize = 64;

    struct SR_COMMON_DLL_API ReflectedValue {
        struct Storage { alignas(std::max_align_t) std::byte data[ReflectedValueStorageSize]; };
        static ReflectedValue MakeFromPointer(void* pData, ReflectedValueStorageType storageType);
        static ReflectedValue MakeFromNumeric(uint64_t numeric, ReflectedValueStorageType storageType);
        static ReflectedValue MakeFromInlineData(const Storage& inlineData, ReflectedValueStorageType storageType);

        SR_NODISCARD int64_t& GetNumeric();
        SR_NODISCARD int64_t GetNumeric() const;
        SR_NODISCARD const void* GetData() const;
        SR_NODISCARD void* GetData();

        void SetData(void* pData);

        Storage storage = {};
        ReflectedValueStorageType storageType = ReflectedValueStorageType::Embedded;
    };

    struct ReflectedContainerIterator;
    struct IteratorVTable {
        using GetValueFn = ReflectedValue(*)(ReflectedValue&, ReflectedContainerIterator);
        using OffsetFn = ReflectedContainerIterator(*)(ReflectedContainerIterator, int64_t);

        OffsetFn pOffset;
        GetValueFn pGetValue;
    };

    class Value;
    struct SR_COMMON_DLL_API ReflectedContainerIterator {
        char data[16] = {}; /// с запасом под итераторы Map/Set, где хранится 2 указателя
        Value* pContainer = nullptr;

        SR_NODISCARD Value operator*() const;
        SR_NODISCARD Value operator->() const;

        SR_NODISCARD Value First() const;
        SR_NODISCARD Value Second() const;

        ReflectedContainerIterator operator+(int64_t offset) const;
        ReflectedContainerIterator& operator++();
        ReflectedContainerIterator operator-(int64_t offset) const;
        ReflectedContainerIterator& operator--();

        SR_NODISCARD bool IsMapIterator() const;

        SR_NODISCARD bool operator==(const ReflectedContainerIterator& other) const noexcept;
        SR_NODISCARD bool operator!=(const ReflectedContainerIterator& other) const noexcept;
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

    struct SR_COMMON_DLL_API TypeInfoVTable {
        /// any type functions
        using ConstructorFn = ReflectedValue(*)(IAllocator&);
        using DestructorFn = void(*)(IAllocator&, ReflectedValue&);
        using CopyFn = void(*)(const ReflectedValue&, ReflectedValue&);
        using MoveFn = void(*)(ReflectedValue&, ReflectedValue&);
        using SizeOfAlignFn = Pair<SizeType, SizeType>(*)();

        ConstructorFn pConstructor = nullptr;
        DestructorFn pDestructor = nullptr;
        CopyFn pCopy = nullptr;
        MoveFn pMove = nullptr;
        SizeOfAlignFn pSizeOfAlign = nullptr;

        /// for SRClass and other containers (except Vector, Map and Set)
        using GetTypeController = void*(*)(ReflectedValue&);

        union {
            struct {
                ContainerVTable containerVTable;
                IteratorVTable iteratorVTable;
            };
            PairVTable pairVTable;
            GetTypeController pGetTypeController;
        };

    };

    struct SR_COMMON_DLL_API TypeInfo {
        ReflectedCategoryType category = ReflectedCategoryType::Unknown;
        uint8_t detailedSize = 0;
        StringAtom detailedType;
        TypeInfo* pNext[2] = { nullptr, nullptr };
        TypeInfoVTable vtable;

        bool operator==(const TypeInfo& other) const noexcept;
        bool operator!=(const TypeInfo& other) const noexcept;
    };

    extern SR_COMMON_DLL_API TypeInfo* AllocateTypeInfo();
    extern SR_COMMON_DLL_API TypeInfo* CopyTypeInfo(TypeInfo* pTypeInfo);
    extern SR_COMMON_DLL_API void FreeTypeInfo(TypeInfo* pTypeInfo);
    extern SR_COMMON_DLL_API void DestroyTypeInfoPool();

    template<typename T, typename Enable = void> struct DetermineTypeInfoAccessor {
        static void Determine(IAllocator&, TypeInfo*) { static_assert(AlwaysFalseV<T>, "Unable to determine type info for type!"); }
    };

    template<typename T> void DetermineTypeInfo(IAllocator& allocator, TypeInfo* pTypeInfo, const T&) {
        using Type = std::remove_cv_t<std::remove_reference_t<T>>;
        DetermineTypeInfoAccessor<Type>::Determine(allocator, pTypeInfo);
    }

    template<typename T> TypeInfo* DetermineTypeInfoAlloc(IAllocator& allocator, const T& t) {
        auto&& pTypeInfo = AllocateTypeInfo();
        DetermineTypeInfo(allocator, pTypeInfo, t);
        return pTypeInfo;
    }

    template<typename T> Pair<SizeType, SizeType> ReflectedTypeTemplateSizeOfAlign() {
        return { sizeof(T), alignof(T) };
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
        reflectedValue.SetData((void*)(&itValue));
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

    template<typename T> ReflectedContainerIterator ReflectedTypeContainerIteratorOffset(ReflectedContainerIterator iterator, int64_t offset) {
        typename T::Iterator it;
        memcpy(&it, &iterator.data, sizeof(typename T::Iterator));
        AdvanceIterator(it, offset);
        ReflectedContainerIterator newIterator;
        memcpy(&newIterator.data, &it, sizeof(typename T::Iterator));
        return newIterator;
    }
}

#include <Utils/Reflection/CommonReflection.inl.h>
#include <Utils/Reflection/VectorReflection.inl.h>
#include <Utils/Reflection/AssociaticeReflection.inl.h>

#endif //SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H
