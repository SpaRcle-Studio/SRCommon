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

#endif //SR_COMMON_STRING_ATOM_LITERALS_H
