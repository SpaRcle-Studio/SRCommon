//
// Created by Monika on 07.07.2026.
//

#include <Utils/TypeTraits/TypeTraits.h>

namespace SR_UTILS_NS {
    SerializationId SerializationId::CreateFromCStr(const char* text) noexcept {
        SerializationId id;
        SR_STRNCPY(id.name, text, MaxNameLength - 1);
        id.name[MaxNameLength - 1] = '\0';
        id.hash = ComputeHash(text);
        id.length = strlen(text);
        return id;
    }

    SerializationId SerializationId::CreateFromString(const std::string_view text) noexcept {
        SerializationId id;
        SR_STRNCPY(id.name, text.data(), SR_MIN(text.size(), MaxNameLength - 1));
        id.name[MaxNameLength - 1] = '\0';
        id.hash = ComputeHash(text);
        id.length = SR_MIN(text.size(), MaxNameLength - 1);
        return id;
    }
}