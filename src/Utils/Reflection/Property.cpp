//
// Created by Monika on 21.01.2025.
//

#include <Utils/Reflection/Property.h>

namespace SR_UTILS_NS::Reflection {
    SR_UTILS_NS::StringAtom MakeSerializePropertyName(std::string_view id) {
        if (id.rfind("m_", 0) == 0) {
            id = id.substr(2);
        }
        if (!id.empty() && id[0] == '_') {
            id = id.substr(1);
        }
        return id;
    }

    SR_UTILS_NS::StringAtom MakeDisplayName(const std::string_view id) {
        std::string serializeId = MakeSerializePropertyName(id);

        serializeId = std::regex_replace(serializeId, std::regex("_+"), " ");
        serializeId = std::regex_replace(serializeId, std::regex("([a-z])([A-Z])"), "$1 $2");

        std::string result;
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

        return result;
    }
}
