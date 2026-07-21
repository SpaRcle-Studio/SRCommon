//
// Created by Monika on 21.07.2026.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H
#define SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H

#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    template<typename T> class Vector;
    template<typename T> class Optional;
    template<typename T> class EntityRef;
    template<typename T> class ResourceRef;
}

namespace SR_UTILS_NS::Reflection {
    SR_ENUM_NS_CLASS_T(ReflectedType, uint8_t,
        Unknown,
        Object,
        SequenceContainer,
        AssociativeContainer,
        BitMap,
        SmartPtr,
        Pointer,
        String,
        StringView,
        StringAtom,
        UnicodeString,
        AABB,
        Path,
        Rect,
        Color,
        MathVector,
        Quaternion,
        MathSize,
        Arithmetic,
        Optional,
        ResourceRef,
        EntityRef,
        Enum
    );

    enum class ReflectedValueStorageType : uint8_t {
        Embedded,
        Reference,
        ConstReference,
    };

    struct SR_COMMON_DLL_API ReflectedValue {
        ReflectedValue();
        ReflectedValue(void* pData, ReflectedValueStorageType storageType);
        ReflectedValue(uint64_t rawData, ReflectedValueStorageType storageType);
        union {
            void* pData = nullptr;
            uint64_t rawData;
        }; /// всегда 8 байт, даже на x86
        ReflectedValueStorageType storageType = ReflectedValueStorageType::Embedded;
    };

    struct SR_COMMON_DLL_API TypeInfoVTable {
        /// any type functions
        using ConstructorFn = ReflectedValue(*)(IAllocator&);
        using DestructorFn = void(*)(IAllocator&, ReflectedValue);
        using CopyFn = void(*)(ReflectedValue, ReflectedValue);
        using MoveFn = void(*)(ReflectedValue, ReflectedValue);

        /// container type functions
        using SizeFn = SizeType(*)(ReflectedValue);
        using ClearFn = void(*)(ReflectedValue);
        using ReserveFn = void(*)(ReflectedValue, SizeType);
        using ResizeFn = void(*)(ReflectedValue, SizeType);
        using AccessFn = ReflectedValue(*)(ReflectedValue, ReflectedValue, SizeType);

        ConstructorFn pConstructor = nullptr;
        DestructorFn pDestructor = nullptr;
        CopyFn pCopy = nullptr;
        MoveFn pMove = nullptr;

        SizeFn pSize = nullptr;
        ClearFn pClear = nullptr;
        ReserveFn pReserve = nullptr;
        ResizeFn pResize = nullptr;
        AccessFn pAccess = nullptr;

    };

    struct SR_COMMON_DLL_API TypeInfo {
        ReflectedType type = ReflectedType::Unknown;
        StringAtom detailedType;
        TypeInfo* pNext = nullptr;
        TypeInfoVTable vtable;

        bool operator==(const TypeInfo& other) const noexcept;
        bool operator!=(const TypeInfo& other) const noexcept;
    };

    template<typename T> TypeInfo* DetermineTypeInfoRegistered(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<int8_t>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<int16_t>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<int32_t>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<int64_t>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<uint8_t>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<uint16_t>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<uint32_t>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<uint64_t>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<float>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<double>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<bool>(IAllocator&);

    extern SR_COMMON_DLL_API TypeInfo* AllocateTypeInfo(IAllocator& allocator);

    template<typename T, typename Enable = void> struct SR_MAYBE_UNUSED DetermineTypeInfoAccessor {
        static TypeInfo* Determine(IAllocator&) { static_assert(AlwaysFalseV<T>, "Unable to determine type info for type!"); }
    };

    template<typename T> TypeInfo* DetermineTypeInfo(IAllocator& allocator, const T&) {
        return DetermineTypeInfoAccessor<T>::Determine(allocator);
    }

    /// ================================================================================================================

    template<typename T> struct SR_MAYBE_UNUSED DetermineTypeInfoAccessor<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
        static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<T>(allocator); }
    };

    template<typename T> ReflectedValue ReflectedTypeTemplateConstructor(IAllocator& allocator) {
        auto pValue = static_cast<T*>(allocator.Allocate(sizeof(T), alignof(T)));
        new (pValue) T();
        return ReflectedValue{ pValue, ReflectedValueStorageType::Embedded };
    }

    template<typename T> void ReflectedTypeTemplateDestructor(IAllocator& allocator, ReflectedValue value) {
        auto pContainer = static_cast<T*>(value.pData);
        pContainer->~T();
        allocator.Free(pContainer, sizeof(T), alignof(T));
    }

    template<typename T> void ReflectedTypeTemplateCopy(ReflectedValue from, ReflectedValue to) {
        auto pFrom = static_cast<T*>(from.pData);
        auto pTo = static_cast<T*>(to.pData);
        *pTo = *pFrom;
    }

    template<typename T> void ReflectedTypeTemplateMove(ReflectedValue from, ReflectedValue to) {
        auto pFrom = static_cast<T*>(from.pData);
        auto pTo = static_cast<T*>(to.pData);
        *pTo = std::move(*pFrom);
    }

    template<typename T> struct DetermineTypeInfoAccessor<Optional<T>> {
        static TypeInfo* Determine(IAllocator& allocator) {
            auto pType = AllocateTypeInfo(allocator);
            pType->type = ReflectedType::Optional;
            pType->pNext = DetermineTypeInfoAccessor<T>::Determine(allocator);
            return pType;
        }
    };

    //template<typename T> struct SR_MAYBE_UNUSED DetermineTypeInfoMetaAccessor<EntityRef<T>> {
    //    static TypeInfoMeta* Determine(IAllocator& allocator) {
    //        auto pMeta = AllocateTypeInfoMeta(allocator);
    //        pMeta->info.type = ReflectedType::EntityRef;
    //        pMeta->info.detailedType = T::GetClassStaticName();
    //        pMeta->vtable.pConstructor = &ReflectedTypeTemplateConstructor<EntityRef<T>>;
    //        pMeta->vtable.pDestructor = &ReflectedTypeTemplateDestructor<EntityRef<T>>;
    //        pMeta->vtable.pCopy = &ReflectedTypeTemplateCopy<EntityRef<T>>;
    //        pMeta->vtable.pMove = &ReflectedTypeTemplateMove<EntityRef<T>>;
    //        return pMeta;
    //    }
    //};

    //template<typename T> struct SR_MAYBE_UNUSED DetermineTypeInfoMetaAccessor<ResourceRef<T>> {
    //    static TypeInfoMeta* Determine(IAllocator& allocator) {
    //        auto pMeta = AllocateTypeInfoMeta(allocator);
    //        pMeta->info.type = ReflectedType::ResourceRef;
    //        pMeta->info.detailedType = T::GetClassStaticName();
    //        pMeta->vtable.pConstructor = &ReflectedTypeTemplateConstructor<ResourceRef<T>>;
    //        pMeta->vtable.pDestructor = &ReflectedTypeTemplateDestructor<ResourceRef<T>>;
    //        pMeta->vtable.pCopy = &ReflectedTypeTemplateCopy<ResourceRef<T>>;
    //        pMeta->vtable.pMove = &ReflectedTypeTemplateMove<ResourceRef<T>>;
    //        return pMeta;
    //    }
    //};
}

#include <Utils/Reflection/SequenceContainerReflection.inl.h>

#endif //SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H
