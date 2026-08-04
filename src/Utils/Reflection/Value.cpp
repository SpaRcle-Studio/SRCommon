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

namespace SR_UTILS_NS::Reflection {
    ReflectedValue STUB_STORAGE;

    /// ----------------------------------------------------------------------------------------------------------------

    BaseContainerValueRef::BaseContainerValueRef(Value* pValue)
        : m_value(pValue)
    { }

    SizeType BaseContainerValueRef::Size() const {
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        return vtable.pSize(m_value->GetStorage());
    }

    bool BaseContainerValueRef::Empty() const {
        return Size() == 0;
    }

    void BaseContainerValueRef::Clear() {
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        vtable.pClear(m_value->GetStorage());
    }

    ReflectedContainerIterator BaseContainerValueRef::Begin() const {
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        ReflectedContainerIterator pIt = vtable.pBegin(m_value->GetStorage());
        pIt.pContainer = m_value;
        return pIt;
    }

    ReflectedContainerIterator BaseContainerValueRef::End() const {
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        ReflectedContainerIterator pIt = vtable.pEnd(m_value->GetStorage());
        pIt.pContainer = m_value;
        return pIt;
    }

    ReflectedContainerIterator BaseContainerValueRef::Erase(ReflectedContainerIterator pIt) {
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        auto newIt = vtable.pErase(m_value->GetStorage(), pIt);
        newIt.pContainer = m_value;
        return newIt;
    }

    /// ----------------------------------------------------------------------------------------------------------------

    SequenceContainerValueRef::SequenceContainerValueRef(Value* pValue)
        : BaseContainerValueRef(pValue)
    { }

    void SequenceContainerValueRef::Resize(SizeType newSize) {
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        vtable.pResize(m_value->GetStorage(), newSize, false);
    }

    Value SequenceContainerValueRef::Back() {
        if (Size() == 0) {
            SRHalt("SequenceContainerValueRef::Back() : container is empty!");
            return Value();
        }
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        auto&& iteratorVTable = m_value->GetTypeInfo().vtable.iteratorVTable;
        auto pIt = vtable.pEnd(m_value->GetStorage());
        iteratorVTable.pOffset(pIt, -1);
        auto storage = iteratorVTable.pGetValue(m_value->GetStorage(), pIt);
        Value value;
        value.m_storage = storage;
        value.m_typeInfo = CopyTypeInfo(m_value->GetTypeInfo().pNext[0]);
        value.m_allocator = m_value->GetAllocator();
        return value;
    }

    ReflectedContainerIterator SequenceContainerValueRef::Insert(ReflectedContainerIterator pIt, const Value& value) {
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        auto newIt = vtable.pInsert(m_value->GetStorage(), pIt, value.GetStorage(), STUB_STORAGE);
        newIt.pContainer = m_value;
        return newIt;
    }

    void SequenceContainerValueRef::PushBack(const Value& value) {
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        auto pIt = vtable.pEnd(m_value->GetStorage());
        vtable.pInsert(m_value->GetStorage(), pIt, value.GetStorage(), STUB_STORAGE);
    }

    void SequenceContainerValueRef::PushFront(const Value& value) {
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        auto pIt = vtable.pBegin(m_value->GetStorage());
        vtable.pInsert(m_value->GetStorage(), pIt, value.GetStorage(), STUB_STORAGE);
    }

    /// ----------------------------------------------------------------------------------------------------------------

    AssociativeContainerValueRef::AssociativeContainerValueRef(Value* pValue)
        : BaseContainerValueRef(pValue)
    { }

    ReflectedContainerIterator AssociativeContainerValueRef::Insert(const Value& key, const Value& value) {
        auto&& vtable = m_value->GetTypeInfo().vtable.containerVTable;
        auto newIt = vtable.pInsert(m_value->GetStorage(), ReflectedContainerIterator(), key.GetStorage(), value.GetStorage());
        newIt.pContainer = m_value;
        return newIt;
    }

    /// ----------------------------------------------------------------------------------------------------------------

    Value::Value() {
        m_allocator = IAllocator::GetDefaultAllocator();
    }

    Value::~Value() {
        Destroy();
    }

    SequenceContainerValueRef Value::AsSequenceContainer() {
        return SequenceContainerValueRef(this);
    }

    AssociativeContainerValueRef Value::AsAssociativeContainer() {
        return AssociativeContainerValueRef(this);
    }

    void Value::Destroy() {
        if (m_typeInfo) {
            if (m_storage.storageType == ReflectedValueStorageType::Dynamic || m_storage.storageType == ReflectedValueStorageType::Embedded) {
                if (auto&& pDestructor = m_typeInfo->vtable.pDestructor) {
                    pDestructor(*m_allocator, m_storage);
                }
            }
        }
        FreeTypeInfo(m_typeInfo);
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
            m_typeInfo = CopyTypeInfo(other.m_typeInfo);
            m_allocator = other.m_allocator;

            if (other.IsRef()) {
                m_storage = other.m_storage;
            }
            else if (m_typeInfo) {
                m_storage = m_typeInfo->vtable.pConstructor(*m_allocator);
                if (auto&& pCopy = other.GetTypeInfo().vtable.pCopy) {
                    pCopy(other.m_storage, m_storage);
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
        Value result;
        result.m_allocator = m_allocator;
        result.m_typeInfo = CopyTypeInfo(m_typeInfo);
        if (IsEmbedded()) {
            result.m_storage = m_storage;
        }
        else {
            result.m_storage = GetTypeInfo().vtable.pConstructor(*m_allocator);
            if (auto&& pCopy = GetTypeInfo().vtable.pCopy) {
                pCopy(m_storage, result.m_storage);
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
        return GetTypeInfo().vtable.pSizeOfAlign().first;
    }

    bool Value::IsSigned() const {
        return IsReflectedTypeSigned(GetTypeInfo().detailedType);
    }

    bool Value::IsIntegral() const {
        return IsReflectedTypeIntegral(GetTypeInfo().detailedType);
    }

    bool Value::IsSharedPtr() const {
        auto&& typeInfo = GetTypeInfo();
        return typeInfo.category == ReflectedCategoryType::Container && typeInfo.detailedType == "SharedPtr";
    }

    const void* Value::Data() const {
        return m_storage.GetData();
    }

    void* Value::Data() {
        return m_storage.GetData();
    }

    SRClass* Value::GetSRClass() const {
        if (SR_HTYPES_NS::SharedPtrBase* pShared = GetSharedPtrBase()) {
            return pShared->GetSRClass();
        }
        auto&& typeInfo = GetTypeInfo();
        if (typeInfo.category == ReflectedCategoryType::Container) {
            if (typeInfo.detailedType == "EntityRef") {
                auto&& pController = GetTypeInfo().vtable.pGetTypeController(const_cast<ReflectedValue&>(m_storage));
                return static_cast<EntityRefBase*>(pController);
            }
            if (typeInfo.detailedType == "ResourceRef") {
                auto&& pController = GetTypeInfo().vtable.pGetTypeController(const_cast<ReflectedValue&>(m_storage));
                return static_cast<ResourceRefBase*>(pController);
            }
        }
        if (typeInfo.category != ReflectedCategoryType::Object) {
            return nullptr;
        }
        void* pController = GetTypeInfo().vtable.pGetTypeController(const_cast<ReflectedValue&>(m_storage));
        return static_cast<SRClass*>(pController);
    }

    StringAtom Value::GetEnumType() const {
        return GetTypeInfo().detailedType;
    }

    SR_HTYPES_NS::SharedPtrBase* Value::GetSharedPtrBase() const {
        auto&& typeInfo = GetTypeInfo();
        if (typeInfo.category != ReflectedCategoryType::Container || typeInfo.detailedType != "SharedPtr") {
            return nullptr;
        }
        auto&& pController = GetTypeInfo().vtable.pGetTypeController(const_cast<ReflectedValue&>(m_storage));
        return static_cast<SR_HTYPES_NS::SharedPtrBase*>(pController);
    }

    SR_UTILS_NS::OptionalBase* Value::GetOptionalBase() const {
        auto&& typeInfo = GetTypeInfo();
        if (typeInfo.category != ReflectedCategoryType::Container || typeInfo.detailedType != "Optional") {
            return nullptr;
        }
        auto&& pController = GetTypeInfo().vtable.pGetTypeController(const_cast<ReflectedValue&>(m_storage));
        return static_cast<SR_UTILS_NS::OptionalBase*>(pController);
    }

    const TypeInfo& Value::GetTypeInfo() const {
        static TypeInfo unknownTypeInfo;
        return m_typeInfo ? *m_typeInfo : unknownTypeInfo;
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

    Value Value::CreateDefault(TypeInfo* pTypeInfo) {
        Value value;
        value.m_allocator = IAllocator::GetDefaultAllocator();
        value.m_typeInfo = CopyTypeInfo(pTypeInfo);
        if (pTypeInfo) {
            value.m_storage = pTypeInfo->vtable.pConstructor(*value.m_allocator);
        }
        return value;
    }

    Value Value::Ref() const {
        Value value;
        value.m_allocator = m_allocator;
        value.m_typeInfo = CopyTypeInfo(m_typeInfo);
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