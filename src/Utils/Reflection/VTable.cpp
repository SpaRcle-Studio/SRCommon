//
// Created by Monika on 11.08.2026.
//

#include <Utils/Reflection/VTable.h>
#include <Utils/Reflection/ArrayOps.h>
#include <Utils/Reflection/MapOps.h>
#include <Utils/Reflection/ReflectedType.h>
#include <Utils/TypeTraits/Factory.h>
#include <Utils/Common/EnumReflector.h>
#include <Utils/ECS/EntityRef.h>
#include <Utils/Resources/ResourceRef.h>

namespace SR_UTILS_NS::Reflection {
    bool HasResolvedVectorElements(const TypeInfo& type) {
        if (type.kind == TypeKind::Set) {
            auto&& pKey = type.GetArgument(0);
            return pKey && pKey->IsResolved() && HasResolvedVectorElements(*pKey);
        }
        if (type.kind == TypeKind::Map) {
            auto&& pKey = type.GetArgument(0);
            auto&& pValue = type.GetArgument(1);
            return pKey && pValue && pKey->IsResolved() && pValue->IsResolved() &&
                HasResolvedVectorElements(*pKey) && HasResolvedVectorElements(*pValue);
        }
        if (type.kind != TypeKind::Vector) {
            return true;
        }
        auto&& pElement = type.pNext[0];
        return pElement && pElement->IsResolved() && HasResolvedVectorElements(*pElement);
    }

    bool TypeInfo::CanConstruct() const {
        return IsResolved() && HasResolvedVectorElements(*this) && vtable.lifecycle.size &&
            (vtable.lifecycle.pConstructAt || IsArithmetic() || kind == TypeKind::Enum);
    }

    bool TypeInfo::CanCopy() const {
        return IsResolved() && HasResolvedVectorElements(*this) && (vtable.lifecycle.isTriviallyCopyable || vtable.lifecycle.pCopyConstructAt);
    }

    bool TypeInfo::CanAssign() const {
        return IsResolved() && HasResolvedVectorElements(*this) &&
            (vtable.lifecycle.isTriviallyCopyable || vtable.lifecycle.pCopyAssign);
    }

    void TypeInfo::ConstructAt(void* pDst) const {
        if (!CanConstruct()) {
            SRHalt("TypeInfo::ConstructAt() : type cannot be constructed!");
            return;
        }
        auto&& lifecycle = vtable.lifecycle;
        if (lifecycle.pConstructAt) {
            lifecycle.pConstructAt(*this, pDst);
        }
        else {
            memset(pDst, 0, lifecycle.size);
        }
    }

    void TypeInfo::CopyConstructAt(void* pDst, const void* pSrc) const {
        if (!CanCopy()) {
            SRHalt("TypeInfo::CopyConstructAt() : type cannot be copied!");
            return;
        }
        auto&& lifecycle = vtable.lifecycle;
        if (lifecycle.isTriviallyCopyable) {
            memcpy(pDst, pSrc, lifecycle.size);
        }
        else {
            lifecycle.pCopyConstructAt(*this, pDst, pSrc);
        }
    }

    void TypeInfo::MoveConstructAt(void* pDst, void* pSrc) const {
        if (!IsResolved()) {
            SRHalt("TypeInfo::MoveConstructAt() : type is unresolved!");
            return;
        }
        auto&& lifecycle = vtable.lifecycle;
        if (lifecycle.isTriviallyRelocatable) {
            memcpy(pDst, pSrc, lifecycle.size);
        }
        else if (lifecycle.pMoveConstructAt) {
            lifecycle.pMoveConstructAt(*this, pDst, pSrc);
        }
        else {
            CopyConstructAt(pDst, pSrc);
        }
    }

    void TypeInfo::CopyAssign(void* pDst, const void* pSrc) const {
        if (!CanAssign()) {
            SRHalt("TypeInfo::CopyAssign() : type cannot be assigned!");
            return;
        }
        if (pDst == pSrc) {
            return;
        }
        auto&& lifecycle = vtable.lifecycle;
        if (lifecycle.isTriviallyCopyable) {
            memcpy(pDst, pSrc, lifecycle.size);
        }
        else {
            lifecycle.pCopyAssign(*this, pDst, pSrc);
        }
    }

    void TypeInfo::DestructAt(void* pData) const {
        auto&& lifecycle = vtable.lifecycle;
        if (IsResolved() && !lifecycle.isTriviallyDestructible && lifecycle.pDestructAt) {
            lifecycle.pDestructAt(*this, pData);
        }
    }

    template<SizeType Size> Pair<SizeType, SizeType> ArithmeticTypeSizeOfAlign(const TypeInfo&) {
        return { Size, Size };
    }

    void FillArithmeticVTable(TypeInfoVTable& vtable, const SizeType size) {
        vtable.lifecycle = {};
        vtable.lifecycle.size = size;
        vtable.lifecycle.alignment = size;
        vtable.lifecycle.isTriviallyCopyable = true;
        vtable.lifecycle.isTriviallyDestructible = true;
        vtable.lifecycle.isTriviallyRelocatable = true;
        vtable.lifecycle.isNothrowMoveConstructible = true;

        switch (size) {
            case 1:
                vtable.pSizeOfAlign = &ArithmeticTypeSizeOfAlign<1>;
                break;
            case 2:
                vtable.pSizeOfAlign = &ArithmeticTypeSizeOfAlign<2>;
                break;
            case 4:
                vtable.pSizeOfAlign = &ArithmeticTypeSizeOfAlign<4>;
                break;
            case 8:
                vtable.pSizeOfAlign = &ArithmeticTypeSizeOfAlign<8>;
                break;
            default:
                SRHalt("FillArithmeticVTable() : unsupported arithmetic size {}!", size);
                break;
        }
    }

    bool ResolveBuiltinVTable(const TypeInfoKey& key, TypeInfoVTable& vtable) {
        if (key.kind == TypeKind::ResourceRef && key.pNext[0] && key.pNext[0]->kind == TypeKind::Object) {
            auto&& pMeta = Factory::Instance().GetType(key.pNext[0]->name);
            if (!pMeta || !pMeta->IsSameOrInherited("IResource")) {
                return false;
            }
            FillLifecycle<ResourceRefDynamic>(vtable);
            vtable.pSizeOfAlign = [](const TypeInfo&) -> Pair<SizeType, SizeType> {
                return { sizeof(ResourceRefDynamic), alignof(ResourceRefDynamic) };
            };
            vtable.lifecycle.pConstructAt = [](const TypeInfo& type, void* pDst) {
                new(pDst) ResourceRefDynamic(type.pNext[0]->name);
            };
            vtable.pGetTypeController = [](ReflectedValue& value) -> void* {
                return static_cast<ResourceRefBase*>(static_cast<ResourceRefDynamic*>(value.GetData()));
            };
            return true;
        }
        if (key.kind == TypeKind::EntityRef && key.pNext[0] && key.pNext[0]->kind == TypeKind::Object) {
            auto&& pMeta = Factory::Instance().GetType(key.pNext[0]->name);
            if (!pMeta || !pMeta->IsSameOrInherited("Entity")) {
                return false;
            }
            FillLifecycle<EntityRefDynamic>(vtable);
            vtable.pSizeOfAlign = [](const TypeInfo&) -> Pair<SizeType, SizeType> {
                return { sizeof(EntityRefDynamic), alignof(EntityRefDynamic) };
            };
            vtable.lifecycle.pConstructAt = [](const TypeInfo& type, void* pDst) {
                auto&& pReference = new(pDst) EntityRefDynamic();
                pReference->SetTypeName(type.pNext[0]->name);
            };
            vtable.pGetTypeController = [](ReflectedValue& value) -> void* {
                return static_cast<EntityRefBase*>(static_cast<EntityRefDynamic*>(value.GetData()));
            };
            return true;
        }
        if (key.kind == TypeKind::Map || key.kind == TypeKind::Set) {
            return FillTreeMapVTable(key, vtable);
        }
        if (key.kind == TypeKind::Vector) {
            return FillVectorVTable(key, vtable);
        }
        if (key.kind == TypeKind::Enum) {
            auto&& pReflector = EnumReflectorManager::Instance().TryGetReflector(key.name);
            if (!pReflector) {
                return false;
            }
            FillArithmeticVTable(vtable, pReflector->GetIntegralTypeSizeInternal());
            return true;
        }

        if (key.kind == TypeKind::Object) {
            if (auto&& pMeta = Factory::Instance().GetType(key.name)) {
                vtable = pMeta->GetVTable();
                return true;
            }
            return false;
        }

        return false;
    }
}
