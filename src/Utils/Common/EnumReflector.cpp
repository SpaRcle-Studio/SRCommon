//
// Created by Monika on 09.09.2022.
//

#include <Utils/Common/EnumReflector.h>
#include <Utils/Common/HashManager.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Debug.h>

namespace SR_UTILS_NS {
    EnumReflector::EnumReflector(EnumVariant enumVariant, const void* pData, uint64_t typeSize, uint64_t count, SR_UTILS_NS::StringAtom name, const char* body)
        : m_data(new Data())
    {
        m_enumVariant = enumVariant;
        m_integralTypeSize = typeSize;

        m_data->enumName = name;
        m_data->values.resize(count);
        m_data->names.resize(count);

        enum states {
            state_start, // Before identifier
            state_ident, // In identifier
            state_skip,  // Looking for separator comma
        } state = state_start;

        SRAssert(*body == '(');
        ++body;
        const char* ident_start = nullptr;
        int value_index = 0;
        int level = 0;
        for (;;) {
            SRAssert(*body);
            switch (state) {
                case state_start:
                    if (IsIdentChar(*body)) {
                        state = state_ident;
                        ident_start = body;
                    }
                    ++body;
                    break;
                case state_ident:
                    if (!IsIdentChar(*body)) {
                        state = state_skip;
                        SRAssert(value_index < count);
                        m_data->values[value_index].name = std::string(ident_start, body - ident_start);

                        switch (typeSize) {
                            case 1: m_data->values[value_index].value = static_cast<int64_t>(static_cast<const int8_t*>(pData)[value_index]); break;
                            case 2: m_data->values[value_index].value = static_cast<int64_t>(static_cast<const int16_t*>(pData)[value_index]); break;
                            case 4: m_data->values[value_index].value = static_cast<int64_t>(static_cast<const int32_t*>(pData)[value_index]); break;
                            case 8: m_data->values[value_index].value = static_cast<int64_t>(static_cast<const int64_t*>(pData)[value_index]); break;
                            default:
                                SRHalt("EnumReflector::EnumReflector() : unsupported enum size!");
                                break;
                        }

                        m_data->values[value_index].hashName = m_data->values[value_index].name.GetHash();
                        m_data->names[value_index] = m_data->values[value_index].name;
                        ++value_index;
                    }
                    else {
                        ++body;
                    }
                    break;
                case state_skip:
                    if (*body == '(') {
                        ++level;
                    }
                    else if (*body == ')') {
                        if (level == 0) {
                            SRAssert(value_index == count);
                            return;
                        }
                        --level;
                    }
                    else if (level == 0 && *body == ',') {
                        state = state_start;
                    }
                    ++body;
            }
        }
    }

    void EnumReflectorManager::RegisterReflector(EnumVariant enumVariant, const void* pData, uint64_t typeSize, uint64_t count, const char* name, const char* body) {
        SR_UTILS_NS::StringAtom nameAtom(name);
        if (m_reflectors.count(nameAtom) == 1) {
            SRHalt("EnumReflectorManager::RegisterReflector() : reflector already registered! Name: {}", name);
            return;
        }
        m_reflectors[nameAtom] = new EnumReflector(enumVariant, pData, typeSize, count, name, body);
    }

    void EnumReflectorManager::UnregisterReflector(const char* name) {
        SR_UTILS_NS::StringAtom nameAtom(name);
        if (auto&& pIt = m_reflectors.find(nameAtom); pIt != m_reflectors.end()) {
            SR_SAFE_DELETE_PTR(pIt->second)
            m_reflectors.erase(pIt);
        }
        else {
            SRHalt("EnumReflectorManager::UnregisterReflector() : reflector not found! Name: {}", name);
        }
    }

    EnumReflector* EnumReflectorManager::GetReflector(SR_UTILS_NS::StringAtom name) const {
        if (auto&& pIt = m_reflectors.find(name); pIt != m_reflectors.end()) {
            return pIt->second;
        }
        SRHalt("EnumReflectorManager::GetReflector() : reflector not found! Name: {}", name);
        return nullptr;
    }

    EnumReflector* EnumReflectorManager::TryGetReflector(SR_UTILS_NS::StringAtom name) const {
        if (auto&& pIt = m_reflectors.find(name); pIt != m_reflectors.end()) {
            return pIt->second;
        }
        return nullptr;
    }

    EnumReflectorManager& EnumReflectorManager::Instance() {
        static EnumReflectorManager instance;
        return instance;
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

    const Vector<StringAtom>& EnumReflector::GetNamesInternal() const { return m_data->names; }
    const StringAtom& EnumReflector::GetNameInternal() const { return m_data->enumName; }
    uint64_t EnumReflector::GetIntegralTypeSizeInternal() const { return m_integralTypeSize; }
    EnumVariant EnumReflector::GetEnumVariantInternal() const { return m_enumVariant; }

    std::optional<int64_t> EnumReflector::FromStringLowerCaseInternal(String value) const {
        if (!m_data) {
            std::cerr << "EnumReflector::FromStringLowerCaseInternal() : reflector is empty!\n";
            return std::nullopt;
        }

        SR_UTILS_NS::StringUtils::Instance().ToLower(value);

        String tmp;
        for (auto&& enumerator : m_data->values) {
            tmp = enumerator.name.ToStringView();
            SR_UTILS_NS::StringUtils::Instance().ToLower(tmp);
            if (tmp == value) {
                return enumerator.value;
            }
        }

        return std::nullopt;
    }

    uint64_t EnumReflector::GetAndRegisterHash(const char* name) {
        return SR_HASH_STR_REGISTER(name);
    }
}

