//
// Created by Monika on 09.09.2022.
//

#include <Utils/Common/EnumReflector.h>
#include <Utils/Common/Hashes.h>
#include <Utils/Debug.h>

namespace SR_UTILS_NS {
    bool EnumReflectorManager::RegisterReflector(EnumReflector* pReflector) {
        if (m_reflectors.count(pReflector->GetHashNameInternal()) == 1) {
            return true;
        }
        m_reflectors[pReflector->GetHashNameInternal()] = pReflector;
        return false;
    }

    EnumReflector* EnumReflectorManager::GetReflector(const SR_UTILS_NS::StringAtom& name) const {
        return GetReflector(name.GetHash());
    }

    EnumReflector* EnumReflectorManager::GetReflector(uint64_t hashName) const {
        if (auto&& pIt = m_reflectors.find(hashName); pIt != m_reflectors.end()) {
            return pIt->second;
        }

        return nullptr;
    }

    EnumReflector::~EnumReflector() {
        SR_SAFE_DELETE_PTR(m_data)
    }

    int64_t EnumReflector::ReadEnumValueFromPointerInternal(const void *pEnum) const {
        switch (GetIntegralTypeSizeInternal()) {
            case 1: return *static_cast<const int8_t*>(pEnum);
            case 2: return *static_cast<const int16_t*>(pEnum);
            case 4: return *static_cast<const int32_t*>(pEnum);
            case 8: return *static_cast<const int64_t*>(pEnum);
            default:
                SRHalt("EnumReflector::ReadEnumValueFromPointerInternal() : unsupported enum size!");
                return 0;
        }
    }

    void EnumReflector::WriteEnumValueToPointerInternal(void* pEnum, int64_t value) const {
        switch (GetIntegralTypeSizeInternal()) {
            case 1: *static_cast<int8_t*>(pEnum) = static_cast<int8_t>(value); break;
            case 2: *static_cast<int16_t*>(pEnum) = static_cast<int16_t>(value); break;
            case 4: *static_cast<int32_t*>(pEnum) = static_cast<int32_t>(value); break;
            case 8: *static_cast<int64_t*>(pEnum) = value; break;
            default:
                SRHalt("EnumReflector::WriteEnumValueToPointerInternal() : unsupported enum size!");
                break;
        }
    }

    bool EnumReflector::IsIdentChar(char c) {
        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               (c == '_');
    }

    void EnumReflector::ErrorInternal(const std::string& msg) {
        SRHalt(msg);
    }

    std::optional<SR_UTILS_NS::StringAtom> EnumReflector::ToStringInternal(int64_t value) const {
        if (!m_data) {
            std::cerr << "EnumReflector::ToStringInternal() : reflector is empty!\n";
            return std::nullopt;
        }

        for (auto&& enumerator : m_data->values) {
            if (enumerator.value == value) {
                return enumerator.name;
            }
        }

        return std::nullopt;
    }

    std::optional<int64_t> EnumReflector::FromStringInternal(const SR_UTILS_NS::StringAtom& name) const {
        if (!m_data) {
            std::cerr << "EnumReflector::ToStringInternal() : reflector is empty!\n";
            return std::nullopt;
        }

        for (auto&& enumerator : m_data->values) {
            if (enumerator.hashName == name.GetHash()) {
                return enumerator.value;
            }
        }

        return std::nullopt;
    }

    std::optional<int64_t> EnumReflector::GetIndexInternal(int64_t value) const {
        int64_t index = 0;

        for (auto&& enumerator : m_data->values) {
            if (enumerator.value == value) {
                return index;
            }
            ++index;
        }

        return std::nullopt;
    }

    std::optional<int64_t> EnumReflector::AtInternal(uint64_t index) const {
        if (static_cast<uint64_t>(index) < m_data->values.size()) {
            return m_data->values[index].value;
        }

        return std::nullopt;
    }

    const std::vector<SR_UTILS_NS::StringAtom>& EnumReflector::GetNamesInternal() const { return m_data->names; }
    const SR_UTILS_NS::StringAtom& EnumReflector::GetNameInternal() const { return m_data->enumName; }
    uint64_t EnumReflector::GetIntegralTypeSizeInternal() const { return m_integralTypeSize; }
    uint64_t EnumReflector::GetHashNameInternal() const { return m_data->hashName; }
    EnumVariant EnumReflector::GetEnumVariantInternal() const { return m_enumVariant; }

    std::optional<int64_t> EnumReflector::FromStringLowerCaseInternal(const std::string& value) const {
        if (!m_data) {
            std::cerr << "EnumReflector::FromStringLowerCaseInternal() : reflector is empty!\n";
            return std::nullopt;
        }

        const std::string lower = SR_UTILS_NS::StringUtils::ToLower(value);

        for (auto&& enumerator : m_data->values) {
            if (SR_UTILS_NS::StringUtils::ToLower(enumerator.name) == lower) {
                return enumerator.value;
            }
        }

        return std::nullopt;
    }
}

