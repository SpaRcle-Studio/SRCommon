//
// Created by Codex on 21.11.2026.
//

#include <Utils/Reflection/MapOps.h>
#include <Utils/Reflection/ReflectedType.h>
#include <Utils/Types/Map.h>
#include <Utils/Types/Set.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS::Reflection {
    ErasedTreeMapLayout ReadTreeMap(const void* pMap) {
        ErasedTreeMapLayout layout;
        memcpy(&layout, pMap, sizeof(layout));
        return layout;
    }

    void WriteTreeMap(void* pMap, const ErasedTreeMapLayout& layout) {
        memcpy(pMap, &layout, sizeof(layout));
    }

    SizeType TreeMapNodeSize(const TypeInfo& type) {
        auto&& key = type.GetArgument(0)->vtable.lifecycle;
        if (type.kind == TypeKind::Set) {
            return CalculateSetNodeSize(key.size, key.alignment);
        }
        auto&& value = type.GetArgument(1)->vtable.lifecycle;
        return CalculateMapNodeSize(key.size, key.alignment, value.size, value.alignment);
    }

    void* TreeMapKey(const TypeInfo& type, RbTreeNodeHeader* pNode) {
        if (type.kind == TypeKind::Set) {
            return reinterpret_cast<uint8_t*>(pNode) + CalculateSetNodeValueOffset(type.GetArgument(0)->vtable.lifecycle.alignment);
        }
        return reinterpret_cast<uint8_t*>(pNode) + CalculateMapNodeKeyOffset(
            type.GetArgument(0)->vtable.lifecycle.alignment, type.GetArgument(1)->vtable.lifecycle.alignment);
    }

    void* TreeMapValue(const TypeInfo& type, RbTreeNodeHeader* pNode) {
        if (type.kind == TypeKind::Set) {
            return TreeMapKey(type, pNode);
        }
        auto&& key = type.GetArgument(0)->vtable.lifecycle;
        return reinterpret_cast<uint8_t*>(pNode) + CalculateMapNodeValueOffset(
            key.size, key.alignment, type.GetArgument(1)->vtable.lifecycle.alignment);
    }

    RbTreeNodeHeader* AllocateTreeNode(const TypeInfo& type, const ErasedTreeMapLayout& layout) {
        const auto bytes = TreeMapNodeSize(type);
        auto&& pMemory = layout.pAllocator ? layout.pAllocator->Allocate(bytes) : SRMalloc(bytes);
        if (!pMemory) {
            SRHalt("AllocateTreeNode() : allocation failed!");
            return nullptr;
        }
        return new (pMemory) RbTreeNodeHeader();
    }

    void FreeTreeNode(const TypeInfo& type, const ErasedTreeMapLayout& layout, RbTreeNodeHeader* pNode) {
        if (layout.pAllocator) {
            layout.pAllocator->Free(pNode, TreeMapNodeSize(type));
        }
        else {
            SRFree(pNode);
        }
    }

    void DestroyTreeNode(const TypeInfo& type, const ErasedTreeMapLayout& layout, RbTreeNodeHeader* pNode) {
        if (type.kind != TypeKind::Set) {
            type.GetArgument(1)->DestructAt(TreeMapValue(type, pNode));
        }
        type.GetArgument(0)->DestructAt(TreeMapKey(type, pNode));
        FreeTreeNode(type, layout, pNode);
    }

    RbTreeNodeHeader* FindTreeNode(const TypeInfo& type, const ErasedTreeMapLayout& layout, const void* pKey) {
        if (!layout.pNil) {
            return nullptr;
        }
        auto&& pLess = type.GetArgument(0)->vtable.pLess;
        auto&& pNode = MapCore::LowerBound(layout.pNil, pKey,
            [&type](RbTreeNodeHeader* pCurrent) { return TreeMapKey(type, pCurrent); }, pLess);
        return pNode != layout.pNil && !pLess(pKey, TreeMapKey(type, pNode)) ? pNode : nullptr;
    }

    SizeType TreeMapSize(const TypeInfo&, const void* pMap) {
        return ReadTreeMap(pMap).size;
    }

    void TreeMapClear(const TypeInfo& type, void* pMap) {
        auto layout = ReadTreeMap(pMap);
        if (layout.pNil) {
            MapCore::DestroySubtree(MapCore::Root(layout.pNil), layout.pNil,
                [&](RbTreeNodeHeader* pNode) { DestroyTreeNode(type, layout, pNode); });
            MapCore::InitNil(layout.pNil);
            layout.pMin = layout.pNil;
        }
        layout.size = 0;
        WriteTreeMap(pMap, layout);
    }

    void* TreeMapFind(const TypeInfo& type, void* pMap, const void* pKey) {
        auto&& pNode = FindTreeNode(type, ReadTreeMap(pMap), pKey);
        return pNode ? TreeMapValue(type, pNode) : nullptr;
    }

    void* InsertTreeValue(const TypeInfo& type, void* pMap, const void* pKey, const void* pValue) {
        auto layout = ReadTreeMap(pMap);
        if (!layout.pNil) {
            layout.pNil = AllocateTreeNode(type, layout);
            if (!layout.pNil) {
                return nullptr;
            }
            MapCore::InitNil(layout.pNil);
            layout.pMin = layout.pNil;
            WriteTreeMap(pMap, layout);
        }
        auto&& pNode = AllocateTreeNode(type, layout);
        if (!pNode) {
            return nullptr;
        }
        bool keyConstructed = false;
        SR_EXCEPTIONS_TRY {
            type.GetArgument(0)->CopyConstructAt(TreeMapKey(type, pNode), pKey);
            keyConstructed = true;
            if (type.kind != TypeKind::Set) {
                if (pValue) {
                    type.GetArgument(1)->CopyConstructAt(TreeMapValue(type, pNode), pValue);
                }
                else {
                    type.GetArgument(1)->ConstructAt(TreeMapValue(type, pNode));
                }
            }
        }
        SR_EXCEPTIONS_CATCH(...) {
            if (keyConstructed) {
                type.GetArgument(0)->DestructAt(TreeMapKey(type, pNode));
            }
            FreeTreeNode(type, layout, pNode);
            throw;
        }
        auto result = MapCore::InsertNode(layout.pMin, layout.size, pNode, layout.pNil,
            [&type](RbTreeNodeHeader* pCurrent) { return TreeMapKey(type, pCurrent); }, type.GetArgument(0)->vtable.pLess);
        if (!result.second) {
            DestroyTreeNode(type, layout, pNode);
        }
        WriteTreeMap(pMap, layout);
        return TreeMapValue(type, result.first);
    }

    void* TreeMapFindOrAdd(const TypeInfo& type, void* pMap, const void* pKey) {
        if (auto&& pFound = TreeMapFind(type, pMap, pKey)) {
            return pFound;
        }
        return InsertTreeValue(type, pMap, pKey, nullptr);
    }

    bool TreeMapInsertOrAssign(const TypeInfo& type, void* pMap, const void* pKey, const void* pValue) {
        if (auto&& pFound = TreeMapFind(type, pMap, pKey)) {
            if (type.kind != TypeKind::Set) {
                type.GetArgument(1)->CopyAssign(pFound, pValue);
            }
            return false;
        }
        return InsertTreeValue(type, pMap, pKey, pValue) != nullptr;
    }

    bool TreeMapErase(const TypeInfo& type, void* pMap, const void* pKey) {
        auto layout = ReadTreeMap(pMap);
        auto&& pNode = FindTreeNode(type, layout, pKey);
        if (!pNode) {
            return false;
        }
        MapCore::UnlinkNode(layout.pMin, pNode, layout.pNil);
        --layout.size;
        DestroyTreeNode(type, layout, pNode);
        WriteTreeMap(pMap, layout);
        return true;
    }

    void TreeMapForEach(const TypeInfo& type, void* pMap,
        bool(*pVisit)(void*, const void*, void*), void* pUserData) {
        const auto layout = ReadTreeMap(pMap);
        for (RbTreeNodeHeader* pNode = layout.pMin; pNode && pNode != layout.pNil; pNode = MapCore::Next(pNode, layout.pNil)) {
            if (!pVisit(pUserData, TreeMapKey(type, pNode), type.kind == TypeKind::Set ? nullptr : TreeMapValue(type, pNode))) {
                return;
            }
        }
    }

    void TreeMapConstruct(const TypeInfo&, void* pMap) {
        WriteTreeMap(pMap, ErasedTreeMapLayout());
    }

    void TreeMapDestruct(const TypeInfo& type, void* pMap) {
        TreeMapClear(type, pMap);
        auto layout = ReadTreeMap(pMap);
        if (layout.pNil) {
            FreeTreeNode(type, layout, layout.pNil);
        }
        WriteTreeMap(pMap, ErasedTreeMapLayout());
    }

    void TreeMapCopyConstruct(const TypeInfo& type, void* pDestination, const void* pSource) {
        SR_TRACY_ZONE;
        TreeMapConstruct(type, pDestination);
        const auto layout = ReadTreeMap(pSource);
        SR_EXCEPTIONS_TRY {
            for (RbTreeNodeHeader* pNode = layout.pMin; pNode && pNode != layout.pNil; pNode = MapCore::Next(pNode, layout.pNil)) {
                InsertTreeValue(type, pDestination, TreeMapKey(type, pNode), TreeMapValue(type, pNode));
            }
        }
        SR_EXCEPTIONS_CATCH(...) {
            TreeMapDestruct(type, pDestination);
            throw;
        }
    }

    void TreeMapMoveConstruct(const TypeInfo&, void* pDestination, void* pSource) {
        WriteTreeMap(pDestination, ReadTreeMap(pSource));
        WriteTreeMap(pSource, ErasedTreeMapLayout());
    }

    void TreeMapCopyAssign(const TypeInfo& type, void* pDestination, const void* pSource) {
        if (pDestination == pSource) {
            return;
        }
        ErasedTreeMapLayout temporary;
        TreeMapCopyConstruct(type, &temporary, pSource);
        TreeMapDestruct(type, pDestination);
        TreeMapMoveConstruct(type, pDestination, &temporary);
    }

    Pair<SizeType, SizeType> TreeMapSizeOfAlign(const TypeInfo&) {
        return { sizeof(ErasedTreeMapLayout), alignof(ErasedTreeMapLayout) };
    }

    bool FillTreeMapVTable(const TypeInfoKey& key, TypeInfoVTable& vtable) {
        Map<int32_t, int32_t>::ValidateErasedLayout();
        Set<int32_t>::ValidateErasedLayout();
        const bool set = key.kind == TypeKind::Set;
        auto&& pKey = set ? key.pNext[0] : (key.pNext[0] ? key.pNext[0]->pNext[0] : nullptr);
        auto&& pValue = !set && key.pNext[0] ? key.pNext[0]->pNext[1] : nullptr;
        if (!pKey || !pKey->vtable.pLess || !pKey->CanCopy() ||
            pKey->vtable.lifecycle.alignment > alignof(std::max_align_t)) {
            return false;
        }
        if (!set && (!pValue || !pValue->CanCopy() || !pValue->CanConstruct() || !pValue->CanAssign() ||
            pValue->vtable.lifecycle.alignment > alignof(std::max_align_t))) {
            return false;
        }
        vtable.lifecycle.size = sizeof(ErasedTreeMapLayout);
        vtable.lifecycle.alignment = alignof(ErasedTreeMapLayout);
        vtable.lifecycle.isNothrowMoveConstructible = true;
        vtable.lifecycle.pConstructAt = &TreeMapConstruct;
        vtable.lifecycle.pCopyConstructAt = &TreeMapCopyConstruct;
        vtable.lifecycle.pMoveConstructAt = &TreeMapMoveConstruct;
        vtable.lifecycle.pCopyAssign = &TreeMapCopyAssign;
        vtable.lifecycle.pDestructAt = &TreeMapDestruct;
        vtable.pSizeOfAlign = &TreeMapSizeOfAlign;
        vtable.mapVTable.pSize = &TreeMapSize;
        vtable.mapVTable.pClear = &TreeMapClear;
        vtable.mapVTable.pFind = &TreeMapFind;
        vtable.mapVTable.pFindOrAdd = &TreeMapFindOrAdd;
        vtable.mapVTable.pInsertOrAssign = &TreeMapInsertOrAssign;
        vtable.mapVTable.pErase = &TreeMapErase;
        vtable.mapVTable.pForEach = &TreeMapForEach;
        return true;
    }
}
