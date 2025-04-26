//
// Created by Monika on 20.01.2025.
//

#include <Utils/Reflection/Value.h>

namespace SR_UTILS_NS::Reflection {
    Value ValueSequenceContainerIterator::operator*() const {
        return Value(m_iterator->as_ref());
    }

    InputIteratorPointer<Value> ValueSequenceContainerIterator::operator->() const {
        return operator*();
    }

    void ValueSequenceContainer::Clear() {
        m_storage.clear();
    }

    void ValueSequenceContainer::Resize(uint64_t size) {
        if (static_cast<int64_t>(size) < 0) {
            SR_ERROR("ValueSequenceContainer::Resize() : size is negative!");
            return;
        }
        m_storage.resize(size);
    }

    void ValueSequenceContainer::Reserve(uint64_t size) {
        if (static_cast<int64_t>(size) < 0) {
            SR_ERROR("ValueSequenceContainer::Reserve() : size is negative!");
            return;
        }
        m_storage.reserve(size);
    }

    /// ----------------------------------------------------------------------------------------------------------------

    Value::Value(entt::meta_any&& storage)
        : m_storage(std::move(storage))
    { }

    Value::Value() = default;

    Value::Value(const Value& other) {
        if (other.IsRef()) {
            m_storage = const_cast<entt::meta_any*>(&other.m_storage)->as_ref();
        } else {
            m_storage = other.m_storage;
        }
    }

    Value& Value::operator=(const Value& other) noexcept {
        if (this != &other) {
            if (other.IsRef()) {
                m_storage = const_cast<entt::meta_any*>(&other.m_storage)->as_ref();
            } else {
                m_storage = other.m_storage;
            }
        }
        return *this;
    }

    Value& Value::operator=(Value&& other) noexcept {
        if (this != &other) {
            m_storage = other.IsRef() ? other.m_storage.as_ref() : other.m_storage;
        }
        return *this;
    }


    Value Value::Ref() {
        return Value(m_storage.as_ref());
    }

    Value Value::Copy() const {
        Value result;
        result.m_storage = m_storage;
        return result;
    }

    Value& Value::Detach() {
        if (IsRef()) {
            m_storage = entt::meta_any(m_storage);
        }
        return *this;
    }

    Value& Value::DetachIfConst() {
        if (IsConst()) {
            return Detach();
        }
        return *this;
    }

    bool Value::IsRef() const {
        return m_storage.base().policy() == entt::any_policy::cref || m_storage.base().policy() == entt::any_policy::ref;
    }

    bool Value::IsSequenceContainer() const {
        return m_storage.type().is_sequence_container();
    }

    bool Value::IsAssociativeContainer() const {
        return m_storage.type().is_associative_container();
    }

    bool Value::IsBitMap() const {
        static auto meta = entt::meta_any(std::vector<bool>());
        return GetTypeName() == meta.base().type().name();
    }

    bool Value::IsSmartPtr() const {
        if (!IsClass() || !IsTemplate()) {
            return false;
        }

        static auto meta = entt::meta_any(SR_HTYPES_NS::SharedPtr<uint64_t>());
        static const std::string_view compare = meta.base().type().name().substr(0, meta.base().type().name().find('<'));

        return GetTypeName().starts_with(compare);
    }

    bool Value::IsPointer() const {
        return m_storage.type().is_pointer();
    }

    bool Value::IsString() const {
        if (!IsClass()) {
            return false;
        }

        static const auto meta = entt::meta_any(std::string());
        static const std::string_view compare = meta.base().type().name();
        return GetTypeName() == compare;
    }

    bool Value::IsStringView() const {
        if (!IsClass()) {
            return false;
        }

        static const auto meta = entt::meta_any(std::string_view());
        static const std::string_view compare = meta.base().type().name();
        return GetTypeName() == compare;
    }

    bool Value::IsStringAtom() const {
        if (!IsClass()) {
            return false;
        }

        static const auto meta = entt::meta_any(SR_UTILS_NS::StringAtom());
        static const std::string_view compare = meta.base().type().name();
        return GetTypeName() == compare;
    }

    bool Value::IsUnicodeString() const {
        if (!IsClass()) {
            return false;
        }

        static const auto meta = entt::meta_any(SR_HTYPES_NS::UnicodeString());
        static const std::string_view compare = meta.base().type().name();
        return GetTypeName() == compare;
    }

    bool Value::IsPath() const {
        if (!IsClass()) {
            return false;
        }

        static const auto meta = entt::meta_any(SR_UTILS_NS::Path());
        static const std::string_view compare = meta.base().type().name();
        return GetTypeName() == compare;
    }

    bool Value::IsMathVector() const {
        if (!IsClass() || !IsTemplate()) {
            return false;
        }

        static const auto meta = entt::meta_any(SR_MATH_NS::FVector3());
        static const std::string_view compare = meta.base().type().name().substr(0, meta.base().type().name().find('<') - 1);

        return GetTypeName().starts_with(compare);
    }

    bool Value::IsMathSize() const {
        if (!IsClass() || !IsTemplate()) {
            return false;
        }

        static const auto meta = entt::meta_any(SR_MATH_NS::FSize2());
        static const std::string_view compare = meta.base().type().name().substr(0, meta.base().type().name().find('<') - 1);

        return GetTypeName().starts_with(compare);
    }

    bool Value::IsBool() const {
        return m_storage.type().is_arithmetic() && GetTypeName() == "bool";
    }

    bool Value::IsArithmetic() const {
        return m_storage.type().is_arithmetic();
    }

    bool Value::IsClass() const {
        return m_storage.type().is_class();
    }

    bool Value::IsTemplate() const {
        return m_storage.type().is_template_specialization();
    }

    bool Value::IsConst() const {
        return m_storage.base().policy() == entt::any_policy::cref;
    }

    std::string_view Value::GetTypeName() const {
        return m_storage.base().type().name();
    }

    std::string_view Value::GetSharedPtrType() const {
        SRAssert2(IsSmartPtr(), "Value::GetSharedPtrType() : value is not a smart pointer!");

        std::string_view type = GetTypeName();

        const uint64_t start = type.find('<');
        const uint64_t end = type.rfind('>');

        if (start == std::string_view::npos || end == std::string_view::npos) {
            SRHalt("Value::GetSharedPtrType() : failed to find type!");
            return {};
        }

        return type.substr(start + 1, end - start - 1);
    }

    Value::operator bool() const noexcept {
        return static_cast<bool>(m_storage);
    }

    uint64_t Value::SizeOf() const {
        return m_storage.type().size_of();
    }

    const void* Value::Data() const {
        return m_storage.base().data();
    }

    void* Value::Data() {
        return m_storage.base().data();
    }

    bool Value::IsSigned() const {
        return m_storage.type().is_signed();
    }

    bool Value::IsEnum() const {
        return m_storage.type().is_enum();
    }

    bool Value::IsIntegral() const {
        return m_storage.type().is_integral();
    }

    std::string_view Value::GetEnumType() const {
        std::string_view type = GetTypeName();

        /// format is: enum Namespace::second_namespace::EnumName or enum EnumName

        auto pos = type.rfind(':');
        if (pos == std::string_view::npos) {
            pos = type.find(' ');
        }
        if (pos == std::string_view::npos) {
            return {};
        }
        return type.substr(pos + 1, type.size() - pos - 1);
    }

    SRClass* Value::GetSRClass() const {
        if (SR_HTYPES_NS::SharedPtrBase* pShared = GetSharedPtrBase()) {
            return pShared->GetSRClass();
        }
        return const_cast<SRClass*>(static_cast<const SRClass*>(Data()));
    }

    SR_HTYPES_NS::SharedPtrBase* Value::GetSharedPtrBase() const {
        if (IsSmartPtr()) {
            return const_cast<SR_HTYPES_NS::SharedPtrBase*>(static_cast<const SR_HTYPES_NS::SharedPtrBase*>(Data()));
        }
        return nullptr;
    }

    ValueSequenceContainer Value::AsSequenceContainer() {
        return ValueSequenceContainer(m_storage.as_sequence_container());
    }

    ValueSequenceContainer Value::AsSequenceContainer() const {
        return ValueSequenceContainer(m_storage.as_sequence_container());
    }

    Value::~Value() = default;
}
