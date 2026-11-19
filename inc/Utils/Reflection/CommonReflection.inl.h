//
// Created by Monika on 22.07.2026.
//

namespace SR_UTILS_NS::Reflection {
    template<typename T> void DetermineTypeInfoRegistered(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<Value>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int8_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int16_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int32_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<int64_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint8_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint16_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint32_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<uint64_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<float>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<double>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<bool>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<Path>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<String>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<StringView>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<StringAtom>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<UnicodeString>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<SR_MATH_NS::Unit>>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<int32_t>>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<uint32_t>>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::Rect<uint16_t>>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FSize>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::USize>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::ISize>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FSize2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::USize2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::ISize2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector3>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector4>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::FVector6>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::SVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector3>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector4>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::IVector6>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::USVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector3>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector4>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::UVector6>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector3>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector4>(TypeInfoKey& key, TypeInfoVTable& vtable);
    extern template SR_COMMON_DLL_API void DetermineTypeInfoRegistered<SR_MATH_NS::BVector6>(TypeInfoKey& key, TypeInfoVTable& vtable);

    /// ================================================================================================================

    template<typename T> void* ReflectedTypeGetControllerSRClass(ReflectedValue& value) {
        T* pValue = static_cast<T*>(value.GetData());
        return static_cast<SR_UTILS_NS::SRClass*>(pValue);
    }
    template<typename T> void* ReflectedTypeGetControllerSharedPtr(ReflectedValue& value) {
        auto&& pPointer = static_cast<SR_HTYPES_NS::SharedPtr<T>*>(value.GetData());
        return static_cast<SR_HTYPES_NS::SharedPtrBase*>(pPointer);
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

    template<typename T, bool AllowCopy, bool AllowMove> void FillLifecycle(TypeInfoVTable& vtable) {
        auto&& lifecycle = vtable.lifecycle;
        lifecycle = {};
        lifecycle.size = sizeof(T);
        lifecycle.alignment = alignof(T);
        lifecycle.isTriviallyCopyable = AllowCopy && std::is_trivially_copyable_v<T>;
        lifecycle.isTriviallyDestructible = std::is_trivially_destructible_v<T>;
        lifecycle.isTriviallyRelocatable = AllowMove && std::is_trivially_copyable_v<T>;
        lifecycle.isNothrowMoveConstructible = AllowMove && std::is_nothrow_move_constructible_v<T>;
        if constexpr (std::is_default_constructible_v<T>) {
            lifecycle.pConstructAt = [](const TypeInfo&, void* pDst) {
                new (pDst) T();
            };
        }
        if constexpr (AllowCopy && std::is_copy_constructible_v<T>) {
            lifecycle.pCopyConstructAt = [](const TypeInfo&, void* pDst, const void* pSrc) {
                new (pDst) T(*static_cast<const T*>(pSrc));
            };
        }
        if constexpr (AllowMove && std::is_move_constructible_v<T>) {
            lifecycle.pMoveConstructAt = [](const TypeInfo&, void* pDst, void* pSrc) {
                new (pDst) T(std::move(*static_cast<T*>(pSrc)));
            };
        }
        if constexpr (AllowCopy && std::is_copy_assignable_v<T>) {
            lifecycle.pCopyAssign = [](const TypeInfo&, void* pDst, const void* pSrc) {
                *static_cast<T*>(pDst) = *static_cast<const T*>(pSrc);
            };
        }
        if constexpr (std::is_destructible_v<T>) {
            lifecycle.pDestructAt = [](const TypeInfo&, void* pData) {
                static_cast<T*>(pData)->~T();
            };
        }
    }

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

    /// Арифметика и enum хранятся во встроенном буфере Value, обнулённом конструктором,
    /// а копируются ровно по размеру типа: Value может ссылаться на поле меньшего размера.
    extern SR_COMMON_DLL_API void FillArithmeticVTable(TypeInfoVTable& vtable, SizeType size);

    template<typename T> struct DetermineTypeInfoAccessor<T, std::enable_if_t<IsSREnumV<T>>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            key.kind = TypeKind::Enum;
            key.name = GetEnumName(T());
            FillArithmeticVTable(vtable, sizeof(T));
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<T, std::enable_if_t<
        IsSRClassV<T> &&
        !IsEntityRefV<T> &&
        !IsResourceRefV<T>>>
    {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            key.kind = TypeKind::Object;
            key.name = T::GetClassStaticName();
            vtable = T::GetMetaStatic()->GetVTable();
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
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<T>(key, vtable); }
    };

    template<> struct DetermineTypeInfoAccessor<Path> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<Path>(key, vtable); } };
    template<> struct DetermineTypeInfoAccessor<String> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<String>(key, vtable); } };
    template<> struct DetermineTypeInfoAccessor<StringView> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<StringView>(key, vtable); } };
    template<> struct DetermineTypeInfoAccessor<StringAtom> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<StringAtom>(key, vtable); } };
    template<> struct DetermineTypeInfoAccessor<UnicodeString> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<UnicodeString>(key, vtable); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::AABB> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(key, vtable); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::FColor> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(key, vtable); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Quaternion> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(key, vtable); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Matrix3x3> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(key, vtable); } };
    template<> struct DetermineTypeInfoAccessor<SR_MATH_NS::Matrix4x4> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(key, vtable); } };
    template<> struct DetermineTypeInfoAccessor<Value> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) { DetermineTypeInfoRegistered<Value>(key, vtable); } };

    template<typename T> struct DetermineTypeInfoAccessor<Optional<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            key.kind = TypeKind::Optional;
            key.pNext[0] = TypeOf<T>();
            FillLifecycle<Optional<T>>(vtable);
            vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Optional<T>>;
            vtable.pGetTypeController = &ReflectedTypeGetControllerOptional<T>;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<EntityRef<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            key.kind = TypeKind::EntityRef;
            key.pNext[0] = TypeOf<T>();
            FillLifecycle<EntityRef<T>>(vtable);
            vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<EntityRef<T>>;
            vtable.pGetTypeController = &ReflectedTypeGetControllerEntityRef<T>;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<ResourceRef<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            key.kind = TypeKind::ResourceRef;
            key.pNext[0] = TypeOf<T>();
            FillLifecycle<ResourceRef<T>>(vtable);
            vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<ResourceRef<T>>;
            vtable.pGetTypeController = &ReflectedTypeGetControllerResourceRef<T>;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<SR_HTYPES_NS::SharedPtr<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            key.kind = TypeKind::SharedPtr;
            key.pNext[0] = TypeOf<T>();
            FillLifecycle<SR_HTYPES_NS::SharedPtr<T>>(vtable);
            vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<SR_HTYPES_NS::SharedPtr<T>>;
            vtable.pGetTypeController = &ReflectedTypeGetControllerSharedPtr<T>;
        }
    };

    template<typename T1, typename T2> struct DetermineTypeInfoAccessor<Pair<T1, T2>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            key.kind = TypeKind::Pair;
            key.pNext[0] = TypeOf<T1>();
            key.pNext[1] = TypeOf<T2>();
            FillLifecycle<Pair<T1, T2>>(vtable);
            vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Pair<T1, T2>>;
            vtable.pairVTable.pGetPairValue = &ReflectedTypePairGetValue<Pair<T1, T2>>;
        }
    };
}
