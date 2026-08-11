//
// Created by Monika on 01.08.2026.
//

namespace SR_UTILS_NS::Reflection {
    /// ================================================================================================================

    template<typename T> ReflectedContainerIterator ReflectedTypeMapFind(ReflectedValue& value, const ReflectedValue& key) {
        auto pContainer = static_cast<T*>(value.GetData());
        auto pKey = static_cast<const T::KeyType*>(key.GetData());
        auto it = pContainer->find(*pKey);
        ReflectedContainerIterator iterator;
        memcpy(&iterator.data, &it, sizeof(typename T::Iterator));
        return iterator;
    }

    template<typename T> ReflectedContainerIterator ReflectedTypeMapInsert(ReflectedValue& value, ReflectedContainerIterator, const ReflectedValue& key, const ReflectedValue& val) {
        auto pContainer = static_cast<T*>(value.GetData());
        auto pKey = static_cast<const T::KeyType*>(key.GetData());
        auto pVal = static_cast<const T::MappedType*>(val.GetData());
        auto [it, inserted] = pContainer->insert(*pKey, *pVal);
        ReflectedContainerIterator iterator;
        memcpy(&iterator.data, &it, sizeof(typename T::Iterator));
        return iterator;
    }

    template<typename Key, typename Value, typename Compare> struct DetermineTypeInfoAccessor<Map<Key, Value, Compare>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "Map";
            using Type = Map<Key, Value, Compare>;
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<Pair<Key, Value>>::Determine(pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Type>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Type>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Type>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Type>;
            pTypeInfo->vtable.containerVTable.pClear = &ReflectedTypeContainerClear<Type>;
            pTypeInfo->vtable.containerVTable.pSize = &ReflectedTypeContainerSize<Type>;
            pTypeInfo->vtable.containerVTable.pBegin = &ReflectedTypeContainerBegin<Type>;
            pTypeInfo->vtable.containerVTable.pEnd = &ReflectedTypeContainerEnd<Type>;
            pTypeInfo->vtable.containerVTable.pInsert = &ReflectedTypeMapInsert<Type>;
            pTypeInfo->vtable.containerVTable.pErase = &ReflectedTypeContainerErase<Type>;
            pTypeInfo->vtable.containerVTable.pFind = &ReflectedTypeMapFind<Type>;
            pTypeInfo->vtable.iteratorVTable.pGetValue = &ReflectedTypeContainerIteratorGetValue<Type>;
            pTypeInfo->vtable.iteratorVTable.pOffset = &ReflectedTypeContainerIteratorOffset<Type>;
        }
    };

    template<typename Key, typename Value> struct DetermineTypeInfoAccessor<SR_HTYPES_NS::FlatHashMap<Key, Value>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "FlatHashMap";
            using Type = SR_HTYPES_NS::FlatHashMap<Key, Value>;
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<Pair<Key, Value>>::Determine(pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Type>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Type>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Type>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Type>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Type>;
            pTypeInfo->vtable.containerVTable.pClear = &ReflectedTypeContainerClear<Type>;
            pTypeInfo->vtable.containerVTable.pSize = &ReflectedTypeContainerSize<Type>;
            pTypeInfo->vtable.containerVTable.pBegin = &ReflectedTypeContainerBegin<Type>;
            pTypeInfo->vtable.containerVTable.pEnd = &ReflectedTypeContainerEnd<Type>;
            pTypeInfo->vtable.containerVTable.pInsert = &ReflectedTypeMapInsert<Type>;
            pTypeInfo->vtable.containerVTable.pErase = &ReflectedTypeContainerErase<Type>;
            pTypeInfo->vtable.containerVTable.pFind = &ReflectedTypeMapFind<Type>;
            pTypeInfo->vtable.iteratorVTable.pGetValue = &ReflectedTypeContainerIteratorGetValue<Type>;
            pTypeInfo->vtable.iteratorVTable.pOffset = &ReflectedTypeContainerIteratorOffset<Type>;
        }
    };

    /// ================================================================================================================

    template<typename T> ReflectedContainerIterator ReflectedTypeSetFind(ReflectedValue& value, const ReflectedValue& key) {
        auto pContainer = static_cast<T*>(value.GetData());
        auto pKey = static_cast<const T::KeyType*>(key.GetData());
        auto it = pContainer->find(*pKey);
        ReflectedContainerIterator iterator;
        memcpy(&iterator.data, &it, sizeof(typename T::Iterator));
        return iterator;
    }

    template<typename T> ReflectedContainerIterator ReflectedTypeSetInsert(ReflectedValue& value, ReflectedContainerIterator, const ReflectedValue& key, const ReflectedValue&) {
        auto pContainer = static_cast<T*>(value.GetData());
        auto pKey = static_cast<const T::KeyType*>(key.GetData());
        auto [it, inserted] = pContainer->insert(*pKey);
        ReflectedContainerIterator iterator;
        memcpy(&iterator.data, &it, sizeof(typename T::Iterator));
        return iterator;
    }

    template<typename T, typename Compare> struct DetermineTypeInfoAccessor<Set<T, Compare>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "Set";
            using Type = Set<T, Compare>;
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Type>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Type>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Type>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Type>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Type>;
            pTypeInfo->vtable.containerVTable.pClear = &ReflectedTypeContainerClear<Type>;
            pTypeInfo->vtable.containerVTable.pSize = &ReflectedTypeContainerSize<Type>;
            pTypeInfo->vtable.containerVTable.pBegin = &ReflectedTypeContainerBegin<Type>;
            pTypeInfo->vtable.containerVTable.pEnd = &ReflectedTypeContainerEnd<Type>;
            pTypeInfo->vtable.containerVTable.pInsert = &ReflectedTypeSetInsert<Type>;
            pTypeInfo->vtable.containerVTable.pErase = &ReflectedTypeContainerErase<Type>;
            pTypeInfo->vtable.containerVTable.pFind = &ReflectedTypeSetFind<Type>;
            pTypeInfo->vtable.iteratorVTable.pGetValue = &ReflectedTypeContainerIteratorGetValue<Type>;
            pTypeInfo->vtable.iteratorVTable.pOffset = &ReflectedTypeContainerIteratorOffset<Type>;
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<SR_HTYPES_NS::FlatHashSet<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "FlatHashSet";
            using Type = SR_HTYPES_NS::FlatHashSet<T>;
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext[0] = AllocateTypeInfo();
            DetermineTypeInfoAccessor<T>::Determine(pTypeInfo->pNext[0]);
            pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Type>;
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Type>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Type>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Type>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Type>;
            pTypeInfo->vtable.containerVTable.pClear = &ReflectedTypeContainerClear<Type>;
            pTypeInfo->vtable.containerVTable.pSize = &ReflectedTypeContainerSize<Type>;
            pTypeInfo->vtable.containerVTable.pBegin = &ReflectedTypeContainerBegin<Type>;
            pTypeInfo->vtable.containerVTable.pEnd = &ReflectedTypeContainerEnd<Type>;
            pTypeInfo->vtable.containerVTable.pInsert = &ReflectedTypeSetInsert<Type>;
            pTypeInfo->vtable.containerVTable.pErase = &ReflectedTypeContainerErase<Type>;
            pTypeInfo->vtable.containerVTable.pFind = &ReflectedTypeSetFind<Type>;
            pTypeInfo->vtable.iteratorVTable.pGetValue = &ReflectedTypeContainerIteratorGetValue<Type>;
            pTypeInfo->vtable.iteratorVTable.pOffset = &ReflectedTypeContainerIteratorOffset<Type>;
        }
    };
}
