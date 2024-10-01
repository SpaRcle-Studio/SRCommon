//
// Created by Monika on 20.08.2024.
//

#ifndef SR_COMMON_STRING_ATOM_LITERALS_H
#define SR_COMMON_STRING_ATOM_LITERALS_H

#include <Utils/Common/HashManager.h>

SR_INLINE SR_UTILS_NS::StringAtom operator"" _atom(const char* str, size_t) {
    return SR_UTILS_NS::StringAtom(str);
}

SR_INLINE uint64_t operator"" _atom_hash(const char* str, size_t) {
    return SR_HASH_STR_REGISTER(str);
}

/// TODO: constexpr StringAtom support
/// SR_INLINE constexpr SR_UTILS_NS::StringAtom operator"" _atom_cexpr(const char* str, size_t) {
///     return SR_UTILS_NS::StringAtom(str);
/// }

constexpr uint64_t operator"" _atom_hash_cexpr(const char* str, size_t size) {
    const auto strView = std::string_view(str, size);
    return SR_HASH_CONSTEXPR_STR_VIEW_REGISTER(strView);
}

#endif //SR_COMMON_STRING_ATOM_LITERALS_H
