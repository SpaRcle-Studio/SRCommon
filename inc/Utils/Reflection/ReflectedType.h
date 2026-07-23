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

        ConstructorFn pConstructor = nullptr;
        DestructorFn pDestructor = nullptr;
        CopyFn pCopy = nullptr;
        MoveFn pMove = nullptr;

        /// common container type functions
        using SizeFn = SizeType(*)(ReflectedValue);
        using ClearFn = void(*)(ReflectedValue);
        using ReserveFn = void(*)(ReflectedValue, SizeType);
        using ResizeFn = void(*)(ReflectedValue, SizeType);

        SizeFn pSize = nullptr;
        ClearFn pClear = nullptr;
        ReserveFn pReserve = nullptr;
        ResizeFn pResize = nullptr;

        /// for SRClass and other containers (except Vector, Map and Set)
        using GetTypeController = void*(*)(ReflectedValue);

        GetTypeController pGetTypeController = nullptr;

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
}

#include <Utils/Reflection/CommonReflection.inl.h>
#include <Utils/Reflection/VectorReflection.inl.h>

#endif //SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H
