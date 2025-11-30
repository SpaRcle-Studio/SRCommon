//
// Created by Monika on 30.11.2025.
//

#include <Utils/Common/StringAtomLiterals.h>
#include <Utils/Common/HashManager.h>
#include <Utils/Types/StringAtom.h>

uint64_t operator ""_atom_hash(const char *str, size_t) {
    return SR_HASH_STR_REGISTER(str);
}

SR_UTILS_NS::StringAtom operator ""_atom(const char *str, size_t) {
    return SR_UTILS_NS::StringAtom(str);
}

SR_UTILS_NS::StringAtom SpaRcle::Utils::Details::CreateStringAtomFromStdString(const std::string& str) noexcept {
    return SR_UTILS_NS::StringAtom(str);
}
