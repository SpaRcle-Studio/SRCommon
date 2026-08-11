//
// Created by Monika on 21.07.2026.
//

namespace SR_UTILS_NS::Reflection {
    template<typename T> void ReflectedTypeVectorResize(ReflectedValue& value, SizeType size, bool reserve) {
        auto pContainer = static_cast<Vector<T>*>(value.GetData());
        if (reserve) {
            pContainer->reserve(size);
        }
        else {
            pContainer->resize(size);
        }
    }

    template<typename T> ReflectedContainerIterator ReflectedTypeVectorInsert(ReflectedValue& value, ReflectedContainerIterator iterator, const ReflectedValue& newValue, const ReflectedValue&) {
        auto pContainer = static_cast<Vector<T>*>(value.GetData());
        typename Vector<T>::Iterator it;
        memcpy(&it, &iterator.data, sizeof(typename Vector<T>::Iterator));
        auto pNewValue = static_cast<const T*>(newValue.GetData());
        auto newIt = pContainer->insert(it, *pNewValue);
        ReflectedContainerIterator newIterator;
        memcpy(&newIterator.data, &newIt, sizeof(typename Vector<T>::Iterator));
        return newIterator;
    }

    template<typename T> ReflectedContainerIterator ReflectedTypeVectorFind(ReflectedValue& value, const ReflectedValue& searchValue) {
        auto pContainer = static_cast<Vector<T>*>(value.GetData());
        auto pSearchValue = static_cast<const T*>(searchValue.GetData());
        auto it = std::find(pContainer->begin(), pContainer->end(), *pSearchValue);
        ReflectedContainerIterator reflectedIt;
        memcpy(&reflectedIt.data, &it, sizeof(typename Vector<T>::Iterator));
        return reflectedIt;
    }

    template<typename T> struct DetermineTypeInfoAccessor<Vector<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "Vector";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Vector<T>>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Vector<T>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Vector<T>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Vector<T>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Vector<T>>;
            pTypeInfo->vtable.containerVTable.pClear = &ReflectedTypeContainerClear<Vector<T>>;
            pTypeInfo->vtable.containerVTable.pSize = &ReflectedTypeContainerSize<Vector<T>>;
            pTypeInfo->vtable.containerVTable.pBegin = &ReflectedTypeContainerBegin<Vector<T>>;
            pTypeInfo->vtable.containerVTable.pEnd = &ReflectedTypeContainerEnd<Vector<T>>;
            pTypeInfo->vtable.containerVTable.pResize = &ReflectedTypeVectorResize<T>;
            pTypeInfo->vtable.containerVTable.pInsert = &ReflectedTypeVectorInsert<T>;
            pTypeInfo->vtable.containerVTable.pErase = &ReflectedTypeContainerErase<Vector<T>>;
            pTypeInfo->vtable.containerVTable.pFind = &ReflectedTypeVectorFind<T>;
            pTypeInfo->vtable.iteratorVTable.pGetValue = &ReflectedTypeContainerIteratorGetValue<Vector<T>>;
            pTypeInfo->vtable.iteratorVTable.pOffset = &ReflectedTypeContainerIteratorOffset<Vector<T>>;
        }
    };
}
