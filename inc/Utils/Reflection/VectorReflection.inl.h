//
// Created by Monika on 21.07.2026.
//

namespace SR_UTILS_NS::Reflection {
    template<typename T> void ReflectedTypeVectorResize(ReflectedValue value, SizeType size) {
        auto pContainer = static_cast<Vector<T>*>(value.pData);
        pContainer->resize(size);
    }

    template<typename T> void ReflectedTypeVectorReserve(ReflectedValue value, SizeType size) {
        auto pContainer = static_cast<Vector<T>*>(value.pData);
        pContainer->reserve(size);
    }

    template<typename T> void ReflectedTypeVectorClear(ReflectedValue value) {
        auto pContainer = static_cast<Vector<T>*>(value.pData);
        pContainer->clear();
    }

    template<typename T> SizeType ReflectedTypeVectorSize(ReflectedValue value) {
        auto pContainer = static_cast<Vector<T>*>(value.pData);
        return pContainer->size();
    }

    template<typename T> ReflectedValue ReflectedTypeVectorAccess(ReflectedValue value, ReflectedValue, SizeType index) {
        auto pContainer = static_cast<Vector<T>*>(value.pData);
        return ReflectedValue{ &(*pContainer)[index], value.storageType == ReflectedValueStorageType::ConstReference ?
            ReflectedValueStorageType::ConstReference :
            ReflectedValueStorageType::Reference
        };
    }

    template<typename T> struct DetermineTypeInfoAccessor<Vector<T>> {
        static TypeInfo* Determine(IAllocator& allocator) {
            auto pTypeInfo = AllocateTypeInfo(allocator);
            static const StringAtom detailedType = "Vector";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext = DetermineTypeInfoAccessor<T>::Determine(allocator);
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Vector<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Vector<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Vector<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Vector<T>>;
            pTypeInfo->vtable.pResize = &ReflectedTypeVectorResize<T>;
            pTypeInfo->vtable.pReserve = &ReflectedTypeVectorReserve<T>;
            pTypeInfo->vtable.pClear = &ReflectedTypeVectorClear<T>;
            pTypeInfo->vtable.pSize = &ReflectedTypeVectorSize<T>;
            //pTypeInfo->vtable.pAccess = &ReflectedTypeVectorAccess<T>;
            return pTypeInfo;
        }
    };
}
