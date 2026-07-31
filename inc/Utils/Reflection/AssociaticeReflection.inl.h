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
        static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "Map";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext = AllocateTypeInfo(allocator, 2);
            DetermineTypeInfoAccessor<Key>::Determine(allocator, &pTypeInfo->pNext[0]);
            DetermineTypeInfoAccessor<Value>::Determine(allocator, &pTypeInfo->pNext[1]);
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.containerVTable.pClear = &ReflectedTypeContainerClear<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.containerVTable.pSize = &ReflectedTypeContainerSize<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.containerVTable.pBegin = &ReflectedTypeContainerBegin<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.containerVTable.pEnd = &ReflectedTypeContainerEnd<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.containerVTable.pGetValue = &ReflectedTypeContainerIteratorGetValue<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.containerVTable.pInsert = &ReflectedTypeMapInsert<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.containerVTable.pErase = &ReflectedTypeContainerErase<Map<Key, Value, Compare>>;
            pTypeInfo->vtable.containerVTable.pFind = &ReflectedTypeMapFind<Map<Key, Value, Compare>>;
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
        static void Determine(IAllocator& allocator, TypeInfo* pTypeInfo) {
            static const StringAtom detailedType = "Set";
            pTypeInfo->detailedType = detailedType;
            pTypeInfo->category = ReflectedCategoryType::Container;
            pTypeInfo->pNext = AllocateTypeInfo(allocator, 1);
            DetermineTypeInfoAccessor<T>::Determine(allocator, pTypeInfo->pNext);
            pTypeInfo->vtable.pConstructor = &ReflectedTypeTemplateConstructor<Set<T, Compare>>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeTemplateDestructor<Set<T, Compare>>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeTemplateCopy<Set<T, Compare>>;
            pTypeInfo->vtable.pMove = &ReflectedTypeTemplateMove<Set<T, Compare>>;
            pTypeInfo->vtable.containerVTable.pClear = &ReflectedTypeContainerClear<Set<T, Compare>>;
            pTypeInfo->vtable.containerVTable.pSize = &ReflectedTypeContainerSize<Set<T, Compare>>;
            pTypeInfo->vtable.containerVTable.pBegin = &ReflectedTypeContainerBegin<Set<T, Compare>>;
            pTypeInfo->vtable.containerVTable.pEnd = &ReflectedTypeContainerEnd<Set<T, Compare>>;
            pTypeInfo->vtable.containerVTable.pGetValue = &ReflectedTypeContainerIteratorGetValue<Set<T, Compare>>;
            pTypeInfo->vtable.containerVTable.pInsert = &ReflectedTypeSetInsert<Set<T, Compare>>;
            pTypeInfo->vtable.containerVTable.pErase = &ReflectedTypeContainerErase<Set<T, Compare>>;
            pTypeInfo->vtable.containerVTable.pFind = &ReflectedTypeSetFind<Set<T, Compare>>;
        }
    };
}
