//
// Created by Monika on 22.07.2026.
//

namespace SR_UTILS_NS::Reflection {
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

    template<typename T> void* ReflectedTypeGetControllerSRClass(ReflectedValue value) {
        return static_cast<SRClass*>(static_cast<T*>(value.pData));
    }
    template<typename T> void* ReflectedTypeGetControllerSharedPtr(ReflectedValue value) {
        return static_cast<SR_HTYPES_NS::SharedPtrBase*>(static_cast<SR_HTYPES_NS::SharedPtr<T>*>(value.pData));
    }
    template<typename T> void* ReflectedTypeGetControllerOptional(ReflectedValue value) {
        return static_cast<OptionalBase*>(static_cast<Optional<T>*>(value.pData));
    }
    template<typename T> void* ReflectedTypeGetControllerEntityRef(ReflectedValue value) {
        return static_cast<EntityRefBase*>(static_cast<EntityRef<T>*>(value.pData));
    }
    template<typename T> void* ReflectedTypeGetControllerResourceRef(ReflectedValue value) {
        return static_cast<ResourceRefBase*>(static_cast<ResourceRef<T>*>(value.pData));
    }

    /// ================================================================================================================

    template<typename T> ReflectedValue ReflectedTypeTemplateConstructor(IAllocator& allocator) {
        auto pValue = static_cast<T*>(allocator.Allocate(sizeof(T), alignof(T)));
        new (pValue) T();
        return ReflectedValue::MakeFromPointer(pValue, ReflectedValueStorageType::Embedded);
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

    /// ================================================================================================================

    extern SR_COMMON_DLL_API ReflectedValue ArithmeticTypeConstructor(IAllocator&);
    extern SR_COMMON_DLL_API void ArithmeticTypeCopy(ReflectedValue from, ReflectedValue to);
    extern SR_COMMON_DLL_API void ArithmeticTypeMove(ReflectedValue from, ReflectedValue to);

    template<typename T> struct DetermineTypeInfoAccessor<T, std::enable_if_t<IsSREnumV<T>>> {
        static TypeInfo* Determine(IAllocator& allocator) {
            auto pTypeInfo = AllocateTypeInfo(allocator);
            pTypeInfo->detailedType = GetEnumName(T());
            pTypeInfo->category = ReflectedCategoryType::Enum;
            pTypeInfo->vtable.pConstructor = &ArithmeticTypeConstructor;
            pTypeInfo->vtable.pCopy = &ArithmeticTypeCopy;
            pTypeInfo->vtable.pMove = &ArithmeticTypeMove;
            return pTypeInfo;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<T, std::enable_if_t<IsSRClassV<T>>> {
        static TypeInfo* Determine(IAllocator& allocator) {
            auto pTypeInfo = AllocateTypeInfo(allocator);
            pTypeInfo->detailedType = T::GetClassStaticName();
            pTypeInfo->category = ReflectedCategoryType::Object;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<T>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerSRClass<T>;
            return pTypeInfo;
        }
    };

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

    template<typename T> struct DetermineTypeInfoAccessor<Optional<T>> {
        static TypeInfo* Determine(IAllocator& allocator) {
            auto pTypeInfo = AllocateTypeInfo(allocator);
            static const StringAtom detailedType = "Optional";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext = DetermineTypeInfoAccessor<T>::Determine(allocator);
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Optional<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Optional<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Optional<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Optional<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerOptional<T>;
            return pTypeInfo;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<EntityRef<T>> {
        static TypeInfo* Determine(IAllocator& allocator) {
            auto pTypeInfo = AllocateTypeInfo(allocator);
            static const StringAtom detailedType = "EntityRef";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext = DetermineTypeInfoAccessor<T>::Determine(allocator);
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<EntityRef<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<EntityRef<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<EntityRef<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<EntityRef<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerEntityRef<T>;
            return pTypeInfo;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<ResourceRef<T>> {
        static TypeInfo* Determine(IAllocator& allocator) {
            auto pTypeInfo = AllocateTypeInfo(allocator);
            static const StringAtom detailedType = "ResourceRef";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext = DetermineTypeInfoAccessor<T>::Determine(allocator);
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<ResourceRef<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<ResourceRef<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<ResourceRef<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<ResourceRef<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerResourceRef<T>;
            return pTypeInfo;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<SR_HTYPES_NS::SharedPtr<T>> {
        static TypeInfo* Determine(IAllocator& allocator) {
            auto pTypeInfo = AllocateTypeInfo(allocator);
            static const StringAtom detailedType = "SharedPtr";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext = DetermineTypeInfoAccessor<T>::Determine(allocator);
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerSharedPtr<T>;
            return pTypeInfo;
        }
    };
}
