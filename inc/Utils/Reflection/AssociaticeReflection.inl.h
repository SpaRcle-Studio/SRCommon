//
// Created by Monika on 01.08.2026.
//

namespace SR_UTILS_NS::Reflection {
    template<typename T> SizeType ReflectedMapSize(const TypeInfo&, const void* pMap) {
        return static_cast<const T*>(pMap)->size();
    }

    template<typename T> void ReflectedMapClear(const TypeInfo&, void* pMap) {
        static_cast<T*>(pMap)->clear();
    }

    template<typename T> bool ReflectedMapErase(const TypeInfo&, void* pMap, const void* pKey) {
        return static_cast<T*>(pMap)->erase(*static_cast<const typename T::KeyType*>(pKey));
    }

    /// ================================================================================================================

    template<typename T> void* ReflectedMapFind(const TypeInfo&, void* pMap, const void* pKey) {
        auto&& map = *static_cast<T*>(pMap);
        auto&& pIt = map.find(*static_cast<const typename T::KeyType*>(pKey));
        if (pIt == map.end()) {
            return nullptr;
        }
        return &pIt->second;
    }

    template<typename T> void* ReflectedMapFindOrAdd(const TypeInfo&, void* pMap, const void* pKey) {
        return &(*static_cast<T*>(pMap))[*static_cast<const typename T::KeyType*>(pKey)];
    }

    template<typename T> bool ReflectedMapInsertOrAssign(const TypeInfo&, void* pMap, const void* pKey, const void* pValue) {
        auto&& map = *static_cast<T*>(pMap);
        auto&& key = *static_cast<const typename T::KeyType*>(pKey);
        auto&& value = *static_cast<const typename T::MappedType*>(pValue);
        if (auto&& pIt = map.find(key); pIt != map.end()) {
            pIt->second = value;
            return false;
        }
        map[key] = value;
        return true;
    }

    template<typename T> void ReflectedMapForEach(const TypeInfo&, void* pMap, bool(*pVisit)(void* pUserData, const void* pKey, void* pValue), void* pUserData) {
        auto&& map = *static_cast<T*>(pMap);
        for (auto&& pIt = map.begin(); pIt != map.end(); ++pIt) {
            if (!pVisit(pUserData, &pIt->first, &pIt->second)) {
                return;
            }
        }
    }

    template<typename T> void FillMapVTable(TypeInfoVTable& vtable) {
        FillLifecycle<T>(vtable);
        vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<T>;
        vtable.mapVTable.pSize = &ReflectedMapSize<T>;
        vtable.mapVTable.pClear = &ReflectedMapClear<T>;
        vtable.mapVTable.pFind = &ReflectedMapFind<T>;
        vtable.mapVTable.pFindOrAdd = &ReflectedMapFindOrAdd<T>;
        vtable.mapVTable.pInsertOrAssign = &ReflectedMapInsertOrAssign<T>;
        vtable.mapVTable.pErase = &ReflectedMapErase<T>;
        vtable.mapVTable.pForEach = &ReflectedMapForEach<T>;
    }

    template<typename Key, typename Value, typename Compare> struct DetermineTypeInfoAccessor<Map<Key, Value, Compare>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            Map<Key, Value, Compare>::ValidateErasedLayout();
            key.kind = TypeKind::Map;
            key.pNext[0] = TypeOf<Pair<Key, Value>>();
            FillMapVTable<Map<Key, Value, Compare>>(vtable);
        }
    };

    template<typename Key, typename Value> struct DetermineTypeInfoAccessor<SR_HTYPES_NS::FlatHashMap<Key, Value>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            key.kind = TypeKind::FlatHashMap;
            key.pNext[0] = TypeOf<Pair<Key, Value>>();
            FillMapVTable<SR_HTYPES_NS::FlatHashMap<Key, Value>>(vtable);
        }
    };

    /// ================================================================================================================

    template<typename T, typename Key> SizeType ReflectedSetSize(const TypeInfo&, const void* pSet) {
        return static_cast<const T*>(pSet)->size();
    }

    template<typename T, typename Key> void ReflectedSetClear(const TypeInfo&, void* pSet) {
        static_cast<T*>(pSet)->clear();
    }

    template<typename T, typename Key> bool ReflectedSetErase(const TypeInfo&, void* pSet, const void* pKey) {
        return static_cast<T*>(pSet)->erase(*static_cast<const Key*>(pKey));
    }

    template<typename T, typename Key> void* ReflectedSetFind(const TypeInfo&, void* pSet, const void* pKey) {
        auto&& set = *static_cast<T*>(pSet);
        auto&& pIt = set.find(*static_cast<const Key*>(pKey));
        if (pIt == set.end()) {
            return nullptr;
        }
        return const_cast<Key*>(&*pIt);
    }

    template<typename T, typename Key> void* ReflectedSetFindOrAdd(const TypeInfo&, void* pSet, const void* pKey) {
        auto&& result = static_cast<T*>(pSet)->insert(*static_cast<const Key*>(pKey));
        return const_cast<Key*>(&*result.first);
    }

    template<typename T, typename Key> bool ReflectedSetInsertOrAssign(const TypeInfo&, void* pSet, const void* pKey, const void*) {
        return static_cast<T*>(pSet)->insert(*static_cast<const Key*>(pKey)).second;
    }

    template<typename T, typename Key> void ReflectedSetForEach(const TypeInfo&, void* pSet, bool(*pVisit)(void* pUserData, const void* pKey, void* pValue), void* pUserData) {
        auto&& set = *static_cast<T*>(pSet);
        for (auto&& pIt = set.begin(); pIt != set.end(); ++pIt) {
            if (!pVisit(pUserData, &*pIt, nullptr)) {
                return;
            }
        }
    }

    template<typename T, typename Key> void FillSetVTable(TypeInfoVTable& vtable) {
        FillLifecycle<T>(vtable);
        vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<T>;
        vtable.mapVTable.pSize = &ReflectedSetSize<T, Key>;
        vtable.mapVTable.pClear = &ReflectedSetClear<T, Key>;
        vtable.mapVTable.pFind = &ReflectedSetFind<T, Key>;
        vtable.mapVTable.pFindOrAdd = &ReflectedSetFindOrAdd<T, Key>;
        vtable.mapVTable.pInsertOrAssign = &ReflectedSetInsertOrAssign<T, Key>;
        vtable.mapVTable.pErase = &ReflectedSetErase<T, Key>;
        vtable.mapVTable.pForEach = &ReflectedSetForEach<T, Key>;
    }

    template<typename T, typename Compare> struct DetermineTypeInfoAccessor<Set<T, Compare>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            Set<T, Compare>::ValidateErasedLayout();
            key.kind = TypeKind::Set;
            key.pNext[0] = TypeOf<T>();
            FillSetVTable<Set<T, Compare>, T>(vtable);
        }
    };

    template<typename T> struct DetermineTypeInfoAccessor<SR_HTYPES_NS::FlatHashSet<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            key.kind = TypeKind::FlatHashSet;
            key.pNext[0] = TypeOf<T>();
            FillSetVTable<SR_HTYPES_NS::FlatHashSet<T>, T>(vtable);
        }
    };
}
