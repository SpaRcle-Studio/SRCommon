//
// Created by Monika on 22.07.2026.
//

namespace SR_UTILS_NS::Reflection {
    template<typename T> void DetermineTypeInfoRegistered(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int8_t>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int16_t>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int32_t>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int64_t>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint8_t>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint16_t>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint32_t>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint64_t>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<float>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<double>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<bool>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<Path>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<String>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<StringView>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<StringAtom>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<UnicodeString>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<SR_MATH_NS::Unit>>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<int32_t>>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<uint32_t>>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<uint16_t>>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FSize>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::USize>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::ISize>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FSize2>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::USize2>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::ISize2>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector2>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector3>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector4>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector6>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::SVector2>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector2>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector3>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector4>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector6>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector2>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::USVector2>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector3>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector4>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector6>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector2>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector3>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector4>(TypeInfo* pTypeInfo);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector6>(TypeInfo* pTypeInfo);

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

    template<typename T> struct DetermineTypeInfoAccessor<T, std::enable_if_t<IsSREnumV<T>>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            pTypeInfo->detailedType = GetEnumName(T());
            pTypeInfo->category = ReflectedCategoryType::Enum;
            pTypeInfo->vtable.pConstructor = &ArithmeticTypeConstructor;
            pTypeInfo->vtable.pCopy = &ArithmeticTypeCopy;
            pTypeInfo->vtable.pMove = &ArithmeticTypeMove;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<T, std::enable_if_t<
        IsSRClassV<T> &&
        !IsEntityRefV<T> &&
        !IsResourceRefV<T>>>
    {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            pTypeInfo->detailedType = T::GetClassStaticName();
            pTypeInfo->category = ReflectedCategoryType::Object;
            pTypeInfo->vtable = T::GetMetaStatic()->GetVTable();
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
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { DetermineTypeInfoRegistered<T>(pTypeInfo); }
    };

    template<> struct DetermineTypeInfoAccessor<Path> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<Path>(pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<String> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<String>(pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<StringView> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<StringView>(pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<StringAtom> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<StringAtom>(pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<UnicodeString> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<UnicodeString>(pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::AABB> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::FColor> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Quaternion> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Matrix3x3> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(pTypeInfo); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Matrix4x4> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) { return DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(pTypeInfo); } };

    template<typename T> struct DetermineTypeInfoAccessor<Optional<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "Optional";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Optional<T>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Optional<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Optional<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Optional<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Optional<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerOptional<T>;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<EntityRef<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "EntityRef";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<EntityRef<T>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<EntityRef<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<EntityRef<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<EntityRef<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<EntityRef<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerEntityRef<T>;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<ResourceRef<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "ResourceRef";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<ResourceRef<T>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<ResourceRef<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<ResourceRef<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<ResourceRef<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<ResourceRef<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerResourceRef<T>;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<SR_HTYPES_NS::SharedPtr<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "SharedPtr";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<SR_HTYPES_NS::SharedPtr<T>>;
            pTypeInfo->vtable.pGetTypeController = &ReflectedTypeGetControllerSharedPtr<T>;
        }
    };

    template<typename T1, typename T2> struct DetermineTypeInfoAccessor<Pair<T1, T2>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "Pair";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            pTypeInfo->pNext[1] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T1>::Determine(pTypeInfo->pNext[0]);
            DetermineTypeInfoAccessor<T2>::Determine(pTypeInfo->pNext[1]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Pair<T1, T2>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Pair<T1, T2>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Pair<T1, T2>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Pair<T1, T2>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Pair<T1, T2>>;
            pTypeInfo->vtable.pairVTable.pGetPairValue = &ReflectedTypePairGetValue<Pair<T1, T2>>;
        }
    };
}
