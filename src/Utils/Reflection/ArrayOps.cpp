//
// Created by Codex on 19.11.2026.
//

#include <Utils/Reflection/ArrayOps.h>
#include <Utils/Reflection/ReflectedType.h>
#include <Utils/Reflection/Value.h>
#include <Utils/Types/Vector.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS::Reflection {
    // Copy the representation to avoid aliasing Vector<T> through an unrelated struct.
    ErasedVectorLayout ReadVector(const void* pArray) {
        ErasedVectorLayout result;
        memcpy(&result, pArray, sizeof(result));
        return result;
    }

    void WriteVector(void* pArray, const ErasedVectorLayout& layout) {
        memcpy(pArray, &layout, sizeof(layout));
    }

    bool VectorElementReady(const TypeInfo& type) {
        auto&& pElement = type.pNext[0];
        if (!pElement || !pElement->IsResolved()) {
            return false;
        }
        if (pElement->kind == TypeKind::Vector) {
            return VectorElementReady(*pElement);
        }
        return true;
    }

    void* VectorElement(const ErasedVectorLayout& layout, SizeType size, SizeType index) {
        return static_cast<uint8_t*>(layout.pData) + size * index;
    }

    void FreeVectorBuffer(const ErasedVectorLayout& layout, SizeType elementSize) {
        if (!layout.pData) {
            return;
        }
        if (layout.pAllocator) {
            layout.pAllocator->Free(layout.pData, layout.capacity * elementSize);
        }
        else {
            SRFree(layout.pData);
        }
    }

    SizeType VectorSize(const TypeInfo&, const void* pArray) {
        return ReadVector(pArray).size;
    }

    void* VectorAt(const TypeInfo& type, void* pArray, SizeType index) {
        if (!VectorElementReady(type)) {
            return nullptr;
        }
        return VectorElement(ReadVector(pArray), type.pNext[0]->vtable.lifecycle.size, index);
    }

    void VectorReserve(const TypeInfo& type, void* pArray, SizeType capacity) {
        SR_TRACY_ZONE;
        if (!VectorElementReady(type)) {
            return;
        }
        auto layout = ReadVector(pArray);
        if (capacity <= layout.capacity) {
            return;
        }
        auto&& element = *type.pNext[0];
        auto&& lifecycle = element.vtable.lifecycle;
        if (capacity > SizeType(-1) / lifecycle.size) {
            SRHalt("VectorReserve() : capacity overflow!");
            return;
        }
        const auto bytes = capacity * lifecycle.size;
        void* pNewData = layout.pAllocator ? layout.pAllocator->Allocate(bytes) : SRMalloc(bytes);
        if (!pNewData) {
            SRHalt("VectorReserve() : failed to allocate memory!");
            return;
        }

        SizeType constructed = 0;
        SR_EXCEPTIONS_TRY {
            if (lifecycle.isTriviallyRelocatable) {
                if (layout.size) {
                    memcpy(pNewData, layout.pData, layout.size * lifecycle.size);
                }
            }
            else {
                for (; constructed < layout.size; ++constructed) {
                    void* pDst = static_cast<uint8_t*>(pNewData) + constructed * lifecycle.size;
                    void* pSrc = VectorElement(layout, lifecycle.size, constructed);
                    if (lifecycle.isNothrowMoveConstructible && lifecycle.pMoveConstructAt) {
                        element.MoveConstructAt(pDst, pSrc);
                    }
                    else {
                        element.CopyConstructAt(pDst, pSrc);
                    }
                }
            }
        }
        SR_EXCEPTIONS_CATCH(...) {
            const ErasedVectorLayout failed = { pNewData, constructed, capacity, layout.pAllocator };
            for (SizeType i = 0; i < constructed; ++i) {
                element.DestructAt(VectorElement(failed, lifecycle.size, i));
            }
            FreeVectorBuffer(failed, lifecycle.size);
            throw;
        }

        if (!lifecycle.isTriviallyRelocatable) {
            for (SizeType i = 0; i < layout.size; ++i) {
                element.DestructAt(VectorElement(layout, lifecycle.size, i));
            }
        }
        FreeVectorBuffer(layout, lifecycle.size);
        layout.pData = pNewData;
        layout.capacity = capacity;
        WriteVector(pArray, layout);
    }

    void VectorResize(const TypeInfo& type, void* pArray, SizeType size) {
        if (!VectorElementReady(type)) {
            return;
        }
        auto&& element = *type.pNext[0];
        auto layout = ReadVector(pArray);
        if (size > layout.size && !element.CanConstruct()) {
            return;
        }
        VectorReserve(type, pArray, size);
        layout = ReadVector(pArray);
        if (size > layout.capacity) {
            return;
        }
        const auto elementSize = element.vtable.lifecycle.size;
        while (layout.size > size) {
            element.DestructAt(VectorElement(layout, elementSize, --layout.size));
        }

        SR_EXCEPTIONS_TRY {
            while (layout.size < size) {
                element.ConstructAt(VectorElement(layout, elementSize, layout.size));
                ++layout.size;
            }
        }
        SR_EXCEPTIONS_CATCH(...) {
            WriteVector(pArray, layout);
            throw;
        }

        WriteVector(pArray, layout);
    }

    void VectorClear(const TypeInfo& type, void* pArray) {
        VectorResize(type, pArray, 0);
    }

    void VectorConstruct(const TypeInfo&, void* pDst) {
        WriteVector(pDst, ErasedVectorLayout());
    }

    void VectorDestruct(const TypeInfo& type, void* pArray) {
        auto layout = ReadVector(pArray);
        if (VectorElementReady(type)) {
            VectorClear(type, pArray);
        }
        else if (layout.size) {
            SR_ERROR("VectorDestruct() : element type is unresolved; element destructors cannot be called!");
        }
        FreeVectorBuffer(layout, type.pNext[0]->vtable.lifecycle.size);
        WriteVector(pArray, ErasedVectorLayout());
    }

    void VectorCopyConstruct(const TypeInfo& type, void* pDst, const void* pSrc) {
        VectorConstruct(type, pDst);
        if (!VectorElementReady(type)) {
            return;
        }
        const auto source = ReadVector(pSrc);
        ErasedVectorLayout destination;
        destination.pAllocator = source.pAllocator;
        WriteVector(pDst, destination);
        VectorReserve(type, pDst, source.size);
        destination = ReadVector(pDst);
        if (destination.capacity < source.size) {
            return;
        }

        auto&& element = *type.pNext[0];
        SR_EXCEPTIONS_TRY {
            for (; destination.size < source.size; ++destination.size) {
                element.CopyConstructAt(VectorElement(destination, element.vtable.lifecycle.size, destination.size),
                    VectorElement(source, element.vtable.lifecycle.size, destination.size));
            }
        }
        SR_EXCEPTIONS_CATCH(...) {
            WriteVector(pDst, destination);
            VectorDestruct(type, pDst);
            throw;
        }

        WriteVector(pDst, destination);
    }

    void VectorMoveConstruct(const TypeInfo&, void* pDst, void* pSrc) {
        WriteVector(pDst, ReadVector(pSrc));
        WriteVector(pSrc, ErasedVectorLayout());
    }

    void VectorCopyAssign(const TypeInfo& type, void* pDst, const void* pSrc) {
        if (pDst == pSrc || !VectorElementReady(type)) {
            return;
        }
        ErasedVectorLayout temporary;
        VectorCopyConstruct(type, &temporary, pSrc);
        VectorDestruct(type, pDst);
        VectorMoveConstruct(type, pDst, &temporary);
    }

    void VectorInsert(const TypeInfo& type, void* pArray, SizeType index, const void* pValue) {
        if (!VectorElementReady(type)) {
            return;
        }
        auto source = ReadVector(pArray);
        if (index > source.size || source.size == SizeType(-1)) {
            return;
        }
        auto&& element = *type.pNext[0];
        const auto elementSize = element.vtable.lifecycle.size;
        auto&& lifecycle = element.vtable.lifecycle;
        if (lifecycle.isTriviallyRelocatable || (lifecycle.isNothrowMoveConstructible && lifecycle.pMoveConstructAt)) {
            // Preserve aliased values before reserve or shifting invalidates their addresses.
            auto value = ValueRef(&element, const_cast<void*>(pValue), true).Copy();
            if (!value) {
                return;
            }
            SizeType capacity = source.capacity;
            if (source.size == capacity) {
                capacity = capacity && capacity <= SizeType(-1) / SR_VECTOR_REALLOCATE_MULTIPLIER ?
                    capacity * SR_VECTOR_REALLOCATE_MULTIPLIER : source.size + 1;
                if (capacity < SR_VECTOR_INITIAL_CAPACITY) {
                    capacity = SR_VECTOR_INITIAL_CAPACITY;
                }
            }
            VectorReserve(type, pArray, capacity);
            source = ReadVector(pArray);
            if (source.capacity <= source.size) {
                return;
            }
            if (lifecycle.isTriviallyRelocatable) {
                memmove(VectorElement(source, elementSize, index + 1),
                    VectorElement(source, elementSize, index), (source.size - index) * elementSize);
                memcpy(VectorElement(source, elementSize, index), value.Data(), elementSize);
            }
            else {
                for (SizeType i = source.size; i > index; --i) {
                    element.MoveConstructAt(VectorElement(source, elementSize, i), VectorElement(source, elementSize, i - 1));
                    element.DestructAt(VectorElement(source, elementSize, i - 1));
                }
                element.MoveConstructAt(VectorElement(source, elementSize, index), value.Data());
            }
            ++source.size;
            WriteVector(pArray, source);
            return;
        }
        ErasedVectorLayout result;
        result.pAllocator = source.pAllocator;
        VectorReserve(type, &result, source.capacity > source.size ? source.capacity : source.size + 1);
        if (result.capacity <= source.size) {
            return;
        }

        SR_EXCEPTIONS_TRY {
            for (; result.size <= source.size; ++result.size) {
                const void* pSrc = result.size == index ? pValue :
                    VectorElement(source, elementSize, result.size - (result.size > index ? 1 : 0));
                element.CopyConstructAt(VectorElement(result, elementSize, result.size), pSrc);
            }
        }
        SR_EXCEPTIONS_CATCH(...) {
            VectorDestruct(type, &result);
            throw;
        }

        VectorDestruct(type, pArray);
        WriteVector(pArray, result);
    }

    void VectorErase(const TypeInfo& type, void* pArray, SizeType index) {
        if (!VectorElementReady(type)) {
            return;
        }
        auto source = ReadVector(pArray);
        if (index >= source.size) {
            return;
        }
        auto&& element = *type.pNext[0];
        const auto elementSize = element.vtable.lifecycle.size;
        auto&& lifecycle = element.vtable.lifecycle;
        if (lifecycle.isTriviallyRelocatable || (lifecycle.isNothrowMoveConstructible && lifecycle.pMoveConstructAt)) {
            element.DestructAt(VectorElement(source, elementSize, index));
            if (lifecycle.isTriviallyRelocatable) {
                memmove(VectorElement(source, elementSize, index), VectorElement(source, elementSize, index + 1),
                    (source.size - index - 1) * elementSize);
            }
            else {
                for (SizeType i = index; i + 1 < source.size; ++i) {
                    element.MoveConstructAt(VectorElement(source, elementSize, i), VectorElement(source, elementSize, i + 1));
                    element.DestructAt(VectorElement(source, elementSize, i + 1));
                }
            }
            --source.size;
            WriteVector(pArray, source);
            return;
        }
        ErasedVectorLayout result;
        result.pAllocator = source.pAllocator;
        VectorReserve(type, &result, source.capacity);
        if (result.capacity < source.capacity) {
            return;
        }

        SR_EXCEPTIONS_TRY {
            for (; result.size + 1 < source.size; ++result.size) {
                element.CopyConstructAt(VectorElement(result, elementSize, result.size),
                    VectorElement(source, elementSize, result.size + (result.size >= index ? 1 : 0)));
            }
        }
        SR_EXCEPTIONS_CATCH(...) {
            VectorDestruct(type, &result);
            throw;
        }

        VectorDestruct(type, pArray);
        WriteVector(pArray, result);
    }

    Pair<SizeType, SizeType> VectorSizeOfAlign(const TypeInfo&) {
        return { sizeof(ErasedVectorLayout), alignof(ErasedVectorLayout) };
    }

    bool FillVectorVTable(const TypeInfoKey& key, TypeInfoVTable& vtable) {
        Vector<uint8_t>::ValidateErasedLayout();
        auto&& pElement = key.pNext[0];
        if (!pElement || !pElement->IsResolved()) {
            return false;
        }
        auto&& element = pElement->vtable.lifecycle;
        if (!element.size || !element.alignment || element.alignment > alignof(std::max_align_t) ||
            (!element.isTriviallyDestructible && !element.pDestructAt) ||
            (!element.isTriviallyRelocatable && !element.pCopyConstructAt &&
                !(element.isNothrowMoveConstructible && element.pMoveConstructAt))) {
            return false;
        }
        auto&& lifecycle = vtable.lifecycle;
        lifecycle.size = sizeof(ErasedVectorLayout);
        lifecycle.alignment = alignof(ErasedVectorLayout);
        lifecycle.isNothrowMoveConstructible = true;
        lifecycle.pConstructAt = &VectorConstruct;
        lifecycle.pMoveConstructAt = &VectorMoveConstruct;
        lifecycle.pDestructAt = &VectorDestruct;
        if (pElement->CanCopy()) {
            lifecycle.pCopyConstructAt = &VectorCopyConstruct;
            lifecycle.pCopyAssign = &VectorCopyAssign;
            vtable.arrayVTable.pInsert = &VectorInsert;
        }
        vtable.arrayVTable.pErase = &VectorErase;
        vtable.pSizeOfAlign = &VectorSizeOfAlign;
        vtable.arrayVTable.pSize = &VectorSize;
        vtable.arrayVTable.pAt = &VectorAt;
        vtable.arrayVTable.pReserve = &VectorReserve;
        vtable.arrayVTable.pResize = &VectorResize;
        vtable.arrayVTable.pClear = &VectorClear;
        return true;
    }
}
