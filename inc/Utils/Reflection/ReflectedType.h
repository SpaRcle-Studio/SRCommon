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
        struct InlineData { char data[64]; };
        static ReflectedValue MakeFromPointer(void* pData, ReflectedValueStorageType storageType);
        static ReflectedValue MakeFromNumeric(uint64_t numeric, ReflectedValueStorageType storageType);
        static ReflectedValue MakeFromInlineData(const InlineData& inlineData, ReflectedValueStorageType storageType);
        union {
            void* pData = nullptr;
            uint64_t numeric;
            InlineData inlineData;
        };
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
        ReflectedCategoryType category = ReflectedCategoryType::Unknown;
        uint8_t detailedSize = 0;
        StringAtom detailedType;
        TypeInfo* pNext = nullptr;
        TypeInfoVTable vtable;

        bool operator==(const TypeInfo& other) const noexcept;
        bool operator!=(const TypeInfo& other) const noexcept;
    };

    extern SR_COMMON_DLL_API TypeInfo* AllocateTypeInfo(IAllocator& allocator);

    template<typename T, typename Enable = void> struct DetermineTypeInfoAccessor {
        static TypeInfo* Determine(IAllocator&) { static_assert(AlwaysFalseV<T>, "Unable to determine type info for type!"); }
    };

    template<typename T> TypeInfo* DetermineTypeInfo(IAllocator& allocator, const T&) {
        return DetermineTypeInfoAccessor<T>::Determine(allocator);
    }

    /// ================================================================================================================

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
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<Path>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<String>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<StringView>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<StringAtom>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<UnicodeString>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FRect>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::IRect>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::URect>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::USRect>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FSize>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::USize>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::ISize>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FSize2>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::USize2>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::ISize2>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FVector2>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FVector3>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FVector4>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::FVector6>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::IVector2>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::IVector3>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::IVector4>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::IVector6>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::UVector2>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::UVector3>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::UVector4>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::UVector6>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::BVector2>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::BVector3>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::BVector4>(IAllocator&);
    extern template SR_COMMON_DLL_API TypeInfo* DetermineTypeInfoRegistered<SR_MATH_NS::BVector6>(IAllocator&);

    /// ================================================================================================================

    template<typename T> struct IsTypeMathVectorTemplate : std::false_type {};
    template<typename T> struct IsTypeMathVectorTemplate<SR_MATH_NS::Vector2<T>> : std::true_type {};
    template<typename T> struct IsTypeMathVectorTemplate<SR_MATH_NS::Vector3<T>> : std::true_type {};
    template<typename T> struct IsTypeMathVectorTemplate<SR_MATH_NS::Vector4<T>> : std::true_type {};
    template<typename T> struct IsTypeMathVectorTemplate<SR_MATH_NS::Vector6<T>> : std::true_type {};
    template<typename T> constexpr bool IsTypeMathVectorTemplateV = IsTypeMathVectorTemplate<T>::value;

    template<typename T> struct IsTypeMathRectTemplate : std::false_type {};
    template<typename T> struct IsTypeMathRectTemplate<SR_MATH_NS::Rect<T>> : std::true_type {};
    template<typename T> constexpr bool IsTypeMathRectTemplateV = IsTypeMathRectTemplate<T>::value;

    template<typename T> struct IsTypeMathSizeTemplate : std::false_type {};
    template<typename T> struct IsTypeMathSizeTemplate<SR_MATH_NS::Size<T>> : std::true_type {};
    template<typename T> struct IsTypeMathSizeTemplate<SR_MATH_NS::Size2<T>> : std::true_type {};
    template<typename T> constexpr bool IsTypeMathSizeTemplateV = IsTypeMathSizeTemplate<T>::value;

    template<typename T> struct DetermineTypeInfoAccessor<T, std::enable_if_t<
        std::is_arithmetic_v<T> || IsTypeMathVectorTemplateV<T> || IsTypeMathRectTemplateV<T> || IsTypeMathSizeTemplateV<T>
    >> {
        static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<T>(allocator); }
    };

    template<> struct DetermineTypeInfoAccessor<Path> { static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<Path>(allocator); } };
    template<> struct DetermineTypeInfoAccessor<String> { static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<String>(allocator); } };
    template<> struct DetermineTypeInfoAccessor<StringView> { static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<StringView>(allocator); } };
    template<> struct DetermineTypeInfoAccessor<StringAtom> { static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<StringAtom>(allocator); } };
    template<> struct DetermineTypeInfoAccessor<UnicodeString> { static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<UnicodeString>(allocator); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::AABB> { static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(allocator); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::FColor> { static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(allocator); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Quaternion> { static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(allocator); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Matrix3x3> { static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(allocator); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Matrix4x4> { static TypeInfo* Determine(IAllocator& allocator) { return DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(allocator); } };

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
            auto pTypeInfo = AllocateTypeInfo(allocator);
            static const StringAtom detailedType = "Optional";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext = DetermineTypeInfoAccessor<T>::Determine(allocator);
            return pTypeInfo;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<EntityRef<T>> {
        static TypeInfo* Determine(IAllocator& allocator) {
            auto pTypeInfo = AllocateTypeInfo(allocator);
            static const StringAtom detailedType = "EntityRef";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->detailedType = T::GetClassStaticName();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<EntityRef<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<EntityRef<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<EntityRef<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<EntityRef<T>>;
            return pTypeInfo;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<ResourceRef<T>> {
        static TypeInfo* Determine(IAllocator& allocator) {
            auto pTypeInfo = AllocateTypeInfo(allocator);
            static const StringAtom detailedType = "ResourceRef";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->detailedType = T::GetClassStaticName();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<ResourceRef<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<ResourceRef<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<ResourceRef<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<ResourceRef<T>>;
            return pTypeInfo;
        }
    };
}

#include <Utils/Reflection/SequenceContainerReflection.inl.h>

#endif //SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H
