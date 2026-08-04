//
// Created by Monika on 22.07.2026.
//

namespace SR_UTILS_NS::Reflection {
    template<typename T> void DetermineTypeInfoRegistered(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int8_t>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int16_t>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int32_t>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int64_t>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint8_t>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint16_t>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint32_t>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint64_t>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<float>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<double>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<bool>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<Path>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<String>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<StringView>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<StringAtom>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<UnicodeString>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<SR_MATH_NS::Unit>>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<int32_t>>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<uint32_t>>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<uint16_t>>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FSize>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::USize>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::ISize>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FSize2>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::USize2>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::ISize2>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector2>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector3>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector4>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector6>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::SVector2>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector2>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector3>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector4>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector6>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector2>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::USVector2>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector3>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector4>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector6>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector2>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector3>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector4>(IAllocator&, TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector6>(IAllocator&, TypeInfo* pTypeInfo);

    /// ================================================================================================================

    template<typename T> void* ReflectedTypeGetControllerSRClass(ReflectedValue& value) {
        T* pValue = static_cast<T*>(value.GetData());
        return static_cast<SR_UTILS_NS::SRClass*>(pValue);
    }
    template<typename T> void* ReflectedTypeGetControllerSharedPtr(ReflectedValue& value) {
        return static_cast<SR_HTYPES_NS::SharedPtrBase*>(static_cast<SR_HTYPES_NS::SharedPtr<T>*>(value.GetData()));
    }
    template<typename T> void* ReflectedTypeGetControllerOptional(ReflectedValue& value) {
        return static_cast<OptionalBase*>(static_cast<Optional<T>*>(value.GetData()));
    }
    template<typename T> void* ReflectedTypeGetControllerEntityRef(ReflectedValue& value) {
        return static_cast<EntityRefBase*>(static_cast<EntityRef<T>*>(value.GetData()));
    }
    template<typename T> void* ReflectedTypeGetControllerResourceRef(ReflectedValue& value) {
        return static_cast<ResourceRefBase*>(static_cast<ResourceRef<T>*>(value.GetData()));
    }

    /// ================================================================================================================

    template<typename T> ReflectedValue ReflectedTypeTemplateConstructor(IAllocator& allocator) {
        if constexpr (sizeof(T) <= ReflectedValueStorageSize) {
            ReflectedValue reflectedValue;
            new (&reflectedValue.storage.data) T();
            reflectedValue.storageType = ReflectedValueStorageType::Embedded;
            return reflectedValue;
        }
        auto pValue = static_cast<T*>(allocator.Allocate(sizeof(T), alignof(T)));
        new (pValue) T();
        return ReflectedValue::MakeFromPointer(pValue, ReflectedValueStorageType::Dynamic);
    }

    template<typename T> void ReflectedTypeTemplateDestructor(IAllocator& allocator, ReflectedValue& value) {
        auto pContainer = static_cast<T*>(value.GetData());
        pContainer->~T();
        if constexpr (sizeof(T) > ReflectedValueStorageSize) {
            allocator.Free(pContainer, sizeof(T), alignof(T));
        }
    }

    template<typename T> void ReflectedTypeTemplateCopy(const ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = static_cast<const T*>(from.GetData());
        auto pTo = static_cast<T*>(to.GetData());
        *pTo = *pFrom;
    }

    template<typename T> void ReflectedTypeTemplateMove(ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = static_cast<T*>(from.GetData());
        auto pTo = static_cast<T*>(to.GetData());
        *pTo = std::move(*pFrom);
    }

    /// ================================================================================================================

    template<typename T> ReflectedValue ReflectedTypePairGetValue(ReflectedValue& value, bool first) {
        auto pPair = static_cast<T*>(value.GetData());
        ReflectedValue reflectedValue;
        if (first) {
            reflectedValue.SetData((void*)(&pPair->first));
        }
        else {
            reflectedValue.SetData((void*)(&pPair->second));
        }
        reflectedValue.storageType = value.storageType == ReflectedValueStorageType::ConstReference ?
            ReflectedValueStorageType::ConstReference :
            ReflectedValueStorageType::Reference;
        return reflectedValue;
    }

    /// ================================================================================================================

    extern SR_COMMON_DLL_API ReflectedValue ArithmeticTypeConstructor(IAllocator&);
    extern SR_COMMON_DLL_API void ArithmeticTypeCopy(const ReflectedValue& from, ReflectedValue& to);
    extern SR_COMMON_DLL_API void ArithmeticTypeMove(ReflectedValue& from, ReflectedValue& to);

    template<typename T> ReflectedValue EnumTypeConstructor(IAllocator& allocator) {
        ReflectedValue value;
        new (&value.storage.data) T();
        value.storageType = ReflectedValueStorageType::Embedded;
        return value;
    }

    template<typename T> struct DetermineTypeInfoAccessor<T, std::enable_if_t<IsSREnumV<T>>> {
        static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) {
            pTypeInfo->detailedType = GetEnumName(T());
            pTypeInfo->category = ReflectedCategoryType::Enum;
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<T>;
            pTypeInfo->vtable.pConstructor = &EnumTypeConstructor<T>;
            pTypeInfo->vtable.pCopy = &ArithmeticTypeCopy;
            pTypeInfo->vtable.pMove = &ArithmeticTypeMove;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<T, std::enable_if_t<
        IsSRClassV<T> &&
        !IsEntityRefV<T> &&
        !IsResourceRefV<T>>>
    {
        static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) {
            pTypeInfo->detailedType = T::GetClassStaticName();
            pTypeInfo->category = ReflectedCategoryType::Object;
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<T>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<T>;
            if constexpr (std::is_copy_constructible_v<T>) {
                pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<T>;
            }
            if constexpr (std::is_move_constructible_v<T>) {
                pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<T>;
            }
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerSRClass<T>;
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
        static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { DetermineTypeInfoRegistered<T>(allocator, pTypeInfo); }
    };

    template<> struct DetermineTypeInfoAccessor<Path> { static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<Path>(allocator, pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<String> { static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<String>(allocator, pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<StringView> { static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<StringView>(allocator, pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<StringAtom> { static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<StringAtom>(allocator, pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<UnicodeString> { static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<UnicodeString>(allocator, pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::AABB> { static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(allocator, pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::FColor> { static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(allocator, pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Quaternion> { static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(allocator, pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Matrix3x3> { static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(allocator, pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Matrix4x4> { static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(allocator, pTypeInfo); } };

    template<typename T> struct DetermineTypeInfoAccessor<Optional<T>> {
        static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "Optional";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(allocator, pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Optional<T>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Optional<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Optional<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Optional<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Optional<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerOptional<T>;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<EntityRef<T>> {
        static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "EntityRef";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(allocator, pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<EntityRef<T>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<EntityRef<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<EntityRef<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<EntityRef<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<EntityRef<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerEntityRef<T>;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<ResourceRef<T>> {
        static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "ResourceRef";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(allocator, pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<ResourceRef<T>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<ResourceRef<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<ResourceRef<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<ResourceRef<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<ResourceRef<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerResourceRef<T>;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<SR_HTYPES_NS::SharedPtr<T>> {
        static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "SharedPtr";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(allocator, pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerSharedPtr<T>;
        }
    };

    template<typename T1, typename T2> struct DetermineTypeInfoAccessor<Pair<T1, T2>> {
        static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "Pair";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            pTypeInfo->pNext[1] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T1>::Determine(allocator, pTypeInfo->pNext[0]);
            DetermineTypeInfoAccessor<T2>::Determine(allocator, pTypeInfo->pNext[1]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Pair<T1, T2>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Pair<T1, T2>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Pair<T1, T2>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Pair<T1, T2>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Pair<T1, T2>>;
            pTypeInfo->vtable.pairVTable.pGetPairValue = &ReflectedTypePairGetValue<Pair<T1, T2>>;
        }
    };
}
