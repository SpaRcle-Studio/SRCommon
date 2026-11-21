//
// Created by Monika on 19.11.2026.
//

#include <Utils/Reflection/ValueRef.h>
#include <Utils/Reflection/Value.h>
#include <Utils/Types/Optional.h>
#include <Utils/ECS/EntityRef.h>
#include <Utils/Resources/ResourceRef.h>

namespace SR_UTILS_NS::Reflection {
    const TypeInfo& ValueRef::GetTypeInfo() const noexcept {
        return m_pTypeInfo ? *m_pTypeInfo : *GetUnknownTypeInfo();
    }

    SizeType ValueRef::SizeOf() const {
        if (!m_pTypeInfo || !m_pTypeInfo->vtable.pSizeOfAlign) {
            return 0;
        }
        return m_pTypeInfo->vtable.pSizeOfAlign(*m_pTypeInfo).first;
    }

    uint32_t ValueRef::FieldCount() const {
        if (!IsValid()) {
            return 0;
        }
        return m_pTypeInfo->vtable.pFieldCount ? m_pTypeInfo->vtable.pFieldCount(*m_pTypeInfo, m_pData) : m_pTypeInfo->fieldsCount;
    }

    StringAtom ValueRef::FieldName(uint32_t index) const {
        if (index >= FieldCount()) {
            return {};
        }
        return m_pTypeInfo->vtable.pFieldName ? m_pTypeInfo->vtable.pFieldName(*m_pTypeInfo, m_pData, index) : m_pTypeInfo->pFields[index].name;
    }

    ValueRef ValueRef::Field(uint32_t index) const {
        if (index >= FieldCount()) {
            return {};
        }
        if (m_pTypeInfo->vtable.pField) {
            return m_pTypeInfo->vtable.pField(*m_pTypeInfo, m_pData, index, m_isConst);
        }
        auto&& field = m_pTypeInfo->pFields[index];
        return ValueRef(field.pType, static_cast<uint8_t*>(m_pData) + field.offset, m_isConst);
    }

    void* ValueRef::GetTypeController() const {
        if (!IsValid() || !m_pTypeInfo->vtable.pGetTypeController) {
            return nullptr;
        }
        auto storage = ReflectedValue::MakeFromPointer(m_pData, m_isConst ? ReflectedValueStorageType::ConstReference : ReflectedValueStorageType::Reference);
        return m_pTypeInfo->vtable.pGetTypeController(storage);
    }

    SRClass* ValueRef::GetSRClass() const {
        if (!IsValid()) {
            return nullptr;
        }

        switch (m_pTypeInfo->kind) {
            case TypeKind::SharedPtr: {
                auto&& pShared = GetSharedPtrBase();
                if (!pShared || !pShared->GetPtrData()) {
                    return nullptr;
                }
                return pShared->GetSRClass();
            }
            case TypeKind::EntityRef:
                return static_cast<EntityRefBase*>(GetTypeController());
            case TypeKind::ResourceRef:
                return static_cast<ResourceRefBase*>(GetTypeController());
            case TypeKind::Object:
            case TypeKind::NativeHandle:
                return static_cast<SRClass*>(GetTypeController());
            default:
                return nullptr;
        }
    }

    SR_HTYPES_NS::SharedPtrBase* ValueRef::GetSharedPtrBase() const {
        if (!IsValid() || m_pTypeInfo->kind != TypeKind::SharedPtr) {
            return nullptr;
        }
        return static_cast<SR_HTYPES_NS::SharedPtrBase*>(GetTypeController());
    }

    OptionalBase* ValueRef::GetOptionalBase() const {
        if (!IsValid() || m_pTypeInfo->kind != TypeKind::Optional) {
            return nullptr;
        }
        return static_cast<OptionalBase*>(GetTypeController());
    }

    ArrayView ValueRef::AsArray() const {
        return ArrayView(*this);
    }

    MapView ValueRef::AsMap() const {
        return MapView(*this);
    }

    Value ValueRef::ToValue() const {
        if (!IsValid()) {
            return Value();
        }
        if (m_isConst) {
            return Value::CreateCRef(*static_cast<const char*>(m_pData), m_pTypeInfo);
        }
        return Value::CreateRef(*static_cast<char*>(m_pData), m_pTypeInfo);
    }

    Value ValueRef::Copy() const {
        if (!IsValid()) {
            return Value();
        }
        return ToValue().Copy();
    }

    bool ValueRef::Assign(const ValueRef& source) const {
        if (!IsValid() || !source.IsValid() || m_isConst || m_pTypeInfo != source.m_pTypeInfo) {
            return false;
        }
        if (!m_pTypeInfo->CanAssign()) {
            return false;
        }
        m_pTypeInfo->CopyAssign(m_pData, source.m_pData);
        return true;
    }

    /// ----------------------------------------------------------------------------------------------------------------

    ArrayView::ArrayView(const ValueRef& array) noexcept {
        if (!array.IsValid() || array.GetTypeInfo().kind != TypeKind::Vector) {
            return;
        }
        auto&& vtable = array.GetTypeInfo().vtable.arrayVTable;
        if (!vtable.pSize || !vtable.pAt) {
            return;
        }
        m_array = array;
        m_pVTable = &vtable;
    }

    const TypeInfo* ArrayView::GetElementType() const noexcept {
        return m_pVTable ? m_array.GetTypeInfo().pNext[0] : nullptr;
    }

    bool ArrayView::CanModify() const {
        if (!m_pVTable) {
            return false;
        }
        if (!m_array.GetTypeInfo().CanConstruct() || !GetElementType() || !GetElementType()->IsResolved()) {
            return false;
        }
        if (m_array.IsConst()) {
            SR_ERROR("ArrayView::CanModify() : array \"{}\" is const!", m_array.GetTypeInfo().name);
            return false;
        }
        return true;
    }

    SizeType ArrayView::Size() const {
        return m_pVTable ? m_pVTable->pSize(m_array.GetTypeInfo(), m_array.Data()) : 0;
    }

    ValueRef ArrayView::At(const SizeType index) const {
        if (index >= Size()) {
            SRHalt("ArrayView::At() : index {} out of range {}!", index, Size());
            return ValueRef();
        }
        return ValueRef(GetElementType(), m_pVTable->pAt(m_array.GetTypeInfo(), m_array.Data(), index), m_array.IsConst());
    }

    ValueRef ArrayView::Back() const {
        const auto size = Size();
        if (size == 0) {
            SRHalt("ArrayView::Back() : array is empty!");
            return ValueRef();
        }
        return At(size - 1);
    }

    void ArrayView::Resize(const SizeType size) const {
        if (CanModify() && m_pVTable->pResize) {
            m_pVTable->pResize(m_array.GetTypeInfo(), m_array.Data(), size);
        }
    }

    void ArrayView::Reserve(const SizeType capacity) const {
        if (CanModify() && m_pVTable->pReserve) {
            m_pVTable->pReserve(m_array.GetTypeInfo(), m_array.Data(), capacity);
        }
    }

    void ArrayView::Clear() const {
        if (CanModify() && m_pVTable->pClear) {
            m_pVTable->pClear(m_array.GetTypeInfo(), m_array.Data());
        }
    }

    bool ArrayView::Insert(const SizeType index, const ValueRef& value) const {
        if (!CanModify() || !m_pVTable->pInsert) {
            return false;
        }
        if (!value.IsValid() || &value.GetTypeInfo() != GetElementType()) {
            SR_ERROR("ArrayView::Insert() : value type \"{}\" does not match element type of \"{}\"!",
                value.GetTypeInfo().name, m_array.GetTypeInfo().name);
            return false;
        }
        if (index > Size()) {
            SRHalt("ArrayView::Insert() : index {} out of range {}!", index, Size());
            return false;
        }
        m_pVTable->pInsert(m_array.GetTypeInfo(), m_array.Data(), index, value.Data());
        return true;
    }

    bool ArrayView::PushBack(const ValueRef& value) const {
        return Insert(Size(), value);
    }

    bool ArrayView::PushFront(const ValueRef& value) const {
        return Insert(0, value);
    }

    void ArrayView::Erase(const SizeType index) const {
        if (!CanModify() || !m_pVTable->pErase) {
            return;
        }
        if (index >= Size()) {
            SRHalt("ArrayView::Erase() : index {} out of range {}!", index, Size());
            return;
        }
        m_pVTable->pErase(m_array.GetTypeInfo(), m_array.Data(), index);
    }

    void ArrayView::Move(const SizeType from, const SizeType to) const {
        if (from == to || !CanModify()) {
            return;
        }
        const auto size = Size();
        if (from >= size || to >= size) {
            SRHalt("ArrayView::Move() : index {} -> {} out of range {}!", from, to, size);
            return;
        }
        /// копия обязательна: после Erase ссылка на элемент указывает уже на соседний
        auto element = At(from).Copy();
        if (!element || !m_pVTable->pInsert || !m_pVTable->pErase) {
            return;
        }
        Erase(from);
        Insert(to, element.AsRef());
    }

    /// ----------------------------------------------------------------------------------------------------------------

    MapView::MapView(const ValueRef& map) noexcept {
        if (!map.IsValid()) {
            return;
        }
        auto&& typeInfo = map.GetTypeInfo();
        if (!typeInfo.IsMapLike() && !typeInfo.IsSetLike()) {
            return;
        }
        auto&& vtable = typeInfo.vtable.mapVTable;
        if (!vtable.pSize || !vtable.pForEach) {
            return;
        }
        auto&& pKeyType = typeInfo.GetArgument(0);
        auto&& pValueType = typeInfo.IsMapLike() ? typeInfo.GetArgument(1) : nullptr;
        if (!pKeyType || (typeInfo.IsMapLike() && !pValueType)) {
            return;
        }
        m_map = map;
        m_pVTable = &vtable;
        m_pKeyType = pKeyType;
        m_pValueType = pValueType;
    }

    bool MapView::CanModify() const {
        if (!m_pVTable) {
            return false;
        }
        if (m_map.IsConst()) {
            SR_ERROR("MapView::CanModify() : container \"{}\" is const!", m_map.GetTypeInfo().name);
            return false;
        }
        return true;
    }

    bool MapView::IsKey(const ValueRef& key) const {
        if (key.IsValid() && &key.GetTypeInfo() == m_pKeyType) {
            return true;
        }
        SR_ERROR("MapView::IsKey() : key type \"{}\" does not match key type of \"{}\"!",
            key.GetTypeInfo().name, m_map.GetTypeInfo().name);
        return false;
    }

    SizeType MapView::Size() const {
        return m_pVTable ? m_pVTable->pSize(m_map.GetTypeInfo(), m_map.Data()) : 0;
    }

    void MapView::Clear() const {
        if (CanModify() && m_pVTable->pClear) {
            m_pVTable->pClear(m_map.GetTypeInfo(), m_map.Data());
        }
    }

    bool MapView::Contains(const ValueRef& key) const {
        return Find(key).IsValid();
    }

    ValueRef MapView::Find(const ValueRef& key) const {
        if (!m_pVTable || !m_pVTable->pFind || !IsKey(key)) {
            return ValueRef();
        }
        auto&& pFound = m_pVTable->pFind(m_map.GetTypeInfo(), m_map.Data(), key.Data());
        if (!pFound) {
            return ValueRef();
        }
        if (IsSet()) {
            return ValueRef(m_pKeyType, pFound, true);
        }
        return ValueRef(m_pValueType, pFound, m_map.IsConst());
    }

    ValueRef MapView::FindOrAdd(const ValueRef& key) const {
        if (!CanModify() || !m_pVTable->pFindOrAdd || !IsKey(key)) {
            return ValueRef();
        }
        auto&& pFound = m_pVTable->pFindOrAdd(m_map.GetTypeInfo(), m_map.Data(), key.Data());
        if (IsSet()) {
            return ValueRef(m_pKeyType, pFound, true);
        }
        return ValueRef(m_pValueType, pFound, false);
    }

    bool MapView::InsertOrAssign(const ValueRef& key, const ValueRef& value) const {
        if (!CanModify() || !m_pVTable->pInsertOrAssign || !IsKey(key)) {
            return false;
        }
        if (!IsSet() && (!value.IsValid() || &value.GetTypeInfo() != m_pValueType)) {
            SR_ERROR("MapView::InsertOrAssign() : value type \"{}\" does not match value type of \"{}\"!",
                value.GetTypeInfo().name, m_map.GetTypeInfo().name);
            return false;
        }
        return m_pVTable->pInsertOrAssign(m_map.GetTypeInfo(), m_map.Data(), key.Data(), IsSet() ? nullptr : value.Data());
    }

    bool MapView::Erase(const ValueRef& key) const {
        if (!CanModify() || !m_pVTable->pErase || !IsKey(key)) {
            return false;
        }
        return m_pVTable->pErase(m_map.GetTypeInfo(), m_map.Data(), key.Data());
    }

    void MapView::ForEachRaw(bool(*pVisit)(void* pUserData, const void* pKey, void* pValue), void* pUserData) const {
        if (m_pVTable) {
            m_pVTable->pForEach(m_map.GetTypeInfo(), m_map.Data(), pVisit, pUserData);
        }
    }
}
