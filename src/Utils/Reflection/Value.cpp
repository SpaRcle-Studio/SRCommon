//
// Created by Monika on 20.01.2025.
//

#include <Utils/Reflection/Value.h>
#include <Utils/Reflection/ReflectedType.h>
#include <Utils/Types/Optional.h>
#include <Utils/ECS/EntityRef.h>
#include <Utils/Resources/ResourceRef.h>
#include <Utils/Math/Rect.h>
#include <Utils/Math/AABB.h>

#include <Enum/ReflectedCategoryType.hpp>

namespace SR_UTILS_NS::Reflection {
    Value::Value() = default;

    Value::~Value() {
        Destroy();
    }

    ValueRef Value::AsRef() const {
        if (!m_typeInfo) {
            return ValueRef();
        }
        return ValueRef(m_typeInfo, const_cast<void*>(m_storage.GetData()), IsConst());
    }

    ArrayView Value::AsArray() const {
        return ArrayView(AsRef());
    }

    MapView Value::AsMap() const {
        return MapView(AsRef());
    }

    bool Value::AllocateStorage(const TypeInfo* pTypeInfo) {
        if (!pTypeInfo || !pTypeInfo->IsResolved() || !pTypeInfo->vtable.lifecycle.size) {
            return false;
        }
        auto&& lifecycle = pTypeInfo->vtable.lifecycle;
        if ((lifecycle.isTriviallyRelocatable || (lifecycle.isNothrowMoveConstructible && lifecycle.pMoveConstructAt)) &&
            lifecycle.size <= ReflectedValueStorageSize &&
            lifecycle.alignment <= alignof(ReflectedValue::Storage)) {
            m_storage = {};
        }
        else {
            void* pData = IAllocator::GetDefaultAllocator()->Allocate(lifecycle.size, lifecycle.alignment);
            if (!pData) {
                return false;
            }
            m_storage = ReflectedValue::MakeFromPointer(pData, ReflectedValueStorageType::Dynamic);
        }
        m_typeInfo = pTypeInfo;
        return true;
    }

    void Value::Destroy() {
        SR_TRACY_ZONE_MAYBE_NO_INIT;
        if (m_typeInfo && !IsRef()) {
            m_typeInfo->DestructAt(m_storage.GetData());
        }
        ReleaseStorage();
    }

    void Value::ReleaseStorage() {
        if (m_typeInfo && IsDynamic()) {
            auto&& lifecycle = m_typeInfo->vtable.lifecycle;
            IAllocator::GetDefaultAllocator()->Free(m_storage.GetData(), lifecycle.size, lifecycle.alignment);
        }
        m_storage = {};
        m_typeInfo = nullptr;
    }

    Value::Value(const Value& other) {
        *this = other;
    }

    Value::Value(Value&& other) noexcept {
        *this = std::move(other);
    }

    Value& Value::operator=(const Value& other) noexcept {
        if (this != &other) {
            Destroy();
            if (other.IsRef()) {
                m_typeInfo = other.m_typeInfo;
                m_storage = other.m_storage;
            }
            else {
                *this = other.Copy();
            }
        }
        return *this;
    }

    Value& Value::operator=(Value&& other) noexcept {
        if (this != &other) {
            Destroy();
            m_typeInfo = other.m_typeInfo;
            if (other.IsEmbedded() && m_typeInfo && !m_typeInfo->vtable.lifecycle.isTriviallyRelocatable) {
                m_typeInfo->MoveConstructAt(m_storage.GetData(), other.m_storage.GetData());
                m_typeInfo->DestructAt(other.m_storage.GetData());
            }
            else {
                m_storage = other.m_storage;
            }
            other.m_storage = {};
            other.m_typeInfo = nullptr;
        }
        return *this;
    }

    Value Value::Copy() const {
        SR_TRACY_ZONE;
        Value result;
        if (m_typeInfo && m_typeInfo->CanCopy() && result.AllocateStorage(m_typeInfo)) {
            SR_EXCEPTIONS_TRY {
                m_typeInfo->CopyConstructAt(result.Data(), Data());
            }
            SR_EXCEPTIONS_CATCH(...) {
                result.ReleaseStorage();
                throw;
            }
        }
        return result;
    }

    bool Value::IsRef() const {
        return m_storage.storageType == ReflectedValueStorageType::Reference || m_storage.storageType == ReflectedValueStorageType::ConstReference;
    }

    bool Value::IsConst() const {
        return m_storage.storageType == ReflectedValueStorageType::ConstReference;
    }

    bool Value::IsDynamic() const {
        return m_storage.storageType == ReflectedValueStorageType::Dynamic;
    }

    bool Value::IsEmbedded() const {
        return m_storage.storageType == ReflectedValueStorageType::Embedded;
    }

    ReflectedValue& Value::GetStorage() {
        return m_storage;
    }

    const ReflectedValue& Value::GetStorage() const {
        return m_storage;
    }

    Value::operator bool() const noexcept {
        return m_typeInfo;
    }

    SizeType Value::SizeOf() const {
        if (!m_typeInfo) {
            return 0;
        }
        if (!m_typeInfo->vtable.pSizeOfAlign) {
            return 0;
        }
        return GetTypeInfo().vtable.pSizeOfAlign(GetTypeInfo()).first;
    }

    bool Value::IsSigned() const {
        return GetTypeInfo().IsSigned();
    }

    bool Value::IsIntegral() const {
        return GetTypeInfo().IsIntegral();
    }

    bool Value::IsSharedPtr() const {
        return GetTypeInfo().kind == TypeKind::SharedPtr;
    }

    const void* Value::Data() const {
        return m_storage.GetData();
    }

    void* Value::Data() {
        return m_storage.GetData();
    }

    SRClass* Value::GetSRClass() const {
        return AsRef().GetSRClass();
    }

    StringAtom Value::GetEnumType() const {
        return GetTypeInfo().name;
    }

    SR_HTYPES_NS::SharedPtrBase* Value::GetSharedPtrBase() const {
        return AsRef().GetSharedPtrBase();
    }

    SR_UTILS_NS::OptionalBase* Value::GetOptionalBase() const {
        return AsRef().GetOptionalBase();
    }

    const TypeInfo& Value::GetTypeInfo() const {
        return m_typeInfo ? *m_typeInfo : *GetUnknownTypeInfo();
    }

    ReflectedCategoryType Value::GetType() const {
        SR_TRACY_ZONE;
        return m_typeInfo ? SR_UTILS_NS::Reflection::GetTypeKindCategory(m_typeInfo->kind) : ReflectedCategoryType::Unknown;
    }

    bool Value::IsValid() const {
        return m_typeInfo && m_typeInfo->kind != TypeKind::Unknown;
    }

    IAllocator* Value::GetAllocator() const {
        return IAllocator::GetDefaultAllocator();
    }

    Value Value::CreateDefault(const TypeInfo* pTypeInfo) {
        SR_TRACY_ZONE;
        Value value;
        if (pTypeInfo && pTypeInfo->CanConstruct() && value.AllocateStorage(pTypeInfo)) {
            SR_EXCEPTIONS_TRY {
                pTypeInfo->ConstructAt(value.Data());
            }
            SR_EXCEPTIONS_CATCH(...) {
                value.ReleaseStorage();
                throw;
            }
        }
        return value;
    }

    Value Value::Ref() const {
        Value value;
        value.m_typeInfo = m_typeInfo;
        if (IsRef()) {
            value.m_storage = m_storage;
        }
        else {
            value.m_storage.SetData(const_cast<void*>(m_storage.GetData()));
            value.m_storage.storageType = IsConst() ? ReflectedValueStorageType::ConstReference : ReflectedValueStorageType::Reference;
        }
        return value;
    }
}
