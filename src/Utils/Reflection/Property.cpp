//
// Created by Monika on 21.01.2025.
//

#include <Utils/Reflection/Property.h>
#include <Utils/Reflection/Value.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS::Reflection {
    std::string_view MakeSerializePropertyName(std::string_view id) {
        SR_TRACY_ZONE;
        if (id.rfind("m_", 0) == 0) {
            id = id.substr(2);
        }
        if (!id.empty() && id[0] == '_') {
            id = id.substr(1);
        }
        return id; /// NOLINT
    }

    SR_UTILS_NS::StringAtom MakeDisplayName(SR_UTILS_NS::StringAtom id) {
        SR_TRACY_ZONE;
        SR_GLOBAL_LOCK;

        static SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, SR_UTILS_NS::StringAtom> cache;
        if (auto&& pIt = cache.find(id); pIt != cache.end()) {
            return pIt->second;
        }

        static const std::regex underscoreRegex("_+");
        static const std::regex camelCaseRegex("([a-z])([A-Z])");

        static const std::string replacement = " ";
        static const std::string camelCaseReplacement = "$1 $2";

        static std::string serializeId;
        static std::string result;

        serializeId.clear();
        serializeId.reserve(128);

        serializeId = MakeSerializePropertyName(id);
        serializeId = std::regex_replace(serializeId, underscoreRegex, replacement);
        serializeId = std::regex_replace(serializeId, camelCaseRegex, camelCaseReplacement);

        result.clear();
        result.reserve(serializeId.size());

        bool capitalize = true;

        for (const char ch : serializeId) {
            if (capitalize && std::isalpha(ch)) {
                result += static_cast<char>(std::toupper(ch));
                capitalize = false;
            } else {
                result += ch;
            }

            if (ch == ' ') {
                capitalize = true;
            }
        }

        cache[id] = result;
        return result;
    }

    Value Property::Get(SRClass* pOwner) const noexcept {
        return m_getCallback(pOwner);
    }

    Property& Property::SetDefaultValue(Value&& value) noexcept {
        m_defaultValue = new Value(value);
        return *this;
    }

    Property& Property::SetResetValue(Value&& value) noexcept {
        m_resetValue = new Value(value);
        return *this;
    }

    const Value& Property::GetResetValue() const noexcept {
        if (!m_resetValue) {
            m_resetValue = new Value();
        }
        return *m_resetValue;
    }

    const Value &Property::GetDefaultValue() const noexcept {
        if (!m_defaultValue) {
            m_defaultValue = new Value();
        }
        return *m_defaultValue;
    }
}
