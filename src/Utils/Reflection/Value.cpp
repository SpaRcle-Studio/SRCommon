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
    Value::Value() {
        m_allocator = IAllocator::GetDefaultAllocator();
    }

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

    void Value::Destroy() {
        SR_TRACY_ZONE_MAYBE_NO_INIT;
        if (m_typeInfo) {
            if (m_storage.storageType == ReflectedValueStorageType::Dynamic || m_storage.storageType == ReflectedValueStorageType::Embedded) {
                if (auto&& pDestructor = m_typeInfo->vtable.pDestructor) {
                    pDestructor(*m_allocator, m_storage);
                }
            }
        }
        m_storage = {};
        m_typeInfo = nullptr;
        m_allocator = nullptr;
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
            m_typeInfo = other.m_typeInfo;
            m_allocator = other.m_allocator;

            if (other.IsRef()) {
                m_storage = other.m_storage;
            }
            else if (m_typeInfo) {
                if (m_typeInfo->vtable.pConstructor) {
                    m_storage = m_typeInfo->vtable.pConstructor(*m_allocator);
                    if (auto&& pCopy = other.GetTypeInfo().vtable.pCopy) {
                        pCopy(other.m_storage, m_storage);
                    }
                }
                else {
                    m_storage = {};
                }
            }
            else {
                m_storage = {};
            }
        }
        return *this;
    }

    Value& Value::operator=(Value&& other) noexcept {
        if (this != &other) {
            Destroy();
            m_storage = other.m_storage;
            m_typeInfo = other.m_typeInfo;
            m_allocator = other.m_allocator;
            other.m_storage = {};
            other.m_typeInfo = nullptr;
            other.m_allocator = nullptr;
        }
        return *this;
    }

    Value Value::Copy() const {
        SR_TRACY_ZONE;
        Value result;
        result.m_allocator = m_allocator;
        result.m_typeInfo = m_typeInfo;
        if (GetTypeInfo().vtable.pConstructor) {
            result.m_storage = GetTypeInfo().vtable.pConstructor(*m_allocator);
            if (auto&& pCopy = GetTypeInfo().vtable.pCopy) {
                pCopy(m_storage, result.m_storage);
            }
        }
        else {
            result.m_storage = {};
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
        return m_typeInfo ? m_typeInfo->category : ReflectedCategoryType::Unknown;
    }

    bool Value::IsValid() const {
        return m_typeInfo && m_typeInfo->category != ReflectedCategoryType::Unknown;
    }

    IAllocator* Value::GetAllocator() const {
        return m_allocator;
    }

    Value Value::CreateDefault(const TypeInfo* pTypeInfo) {
        SR_TRACY_ZONE;
        Value value;
        value.m_allocator = IAllocator::GetDefaultAllocator();
        value.m_typeInfo = pTypeInfo;
        if (pTypeInfo && pTypeInfo->vtable.pConstructor) {
            value.m_storage = pTypeInfo->vtable.pConstructor(*value.m_allocator);
        }
        return value;
    }

    Value Value::Ref() const {
        Value value;
        value.m_allocator = m_allocator;
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
