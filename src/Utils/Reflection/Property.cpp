//
// Created by Monika on 21.01.2025.
//

#include <Utils/Reflection/Property.h>
#include <Utils/Reflection/Value.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Types/StringAtom.h>

namespace SR_UTILS_NS::Reflection {
    StringAtom MakeDisplayName(StringAtom id) {
        SR_TRACY_ZONE;
        SR_GLOBAL_LOCK;

        static SR_HTYPES_NS::FlatHashMap<StringAtom, StringAtom> cache;
        if (auto&& pIt = cache.find(id); pIt != cache.end()) {
            return pIt->second;
        }

        const auto displayName = StringAtom(StringUtils::MakeDisplayName(id));
        cache[id] = displayName;
        return displayName;
    }

    StringAtom Property::GetDisplayName() const noexcept {
        if (m_displayName.empty()) {
            SR_TRACY_ZONE;
            m_displayName = MakeDisplayName(m_name);
        }
        return m_displayName;
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
