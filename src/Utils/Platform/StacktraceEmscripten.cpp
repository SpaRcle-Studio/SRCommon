//
// Created by Monika on 01.03.2026.
//

#include <Utils/Platform/Stacktrace.h>

namespace SR_UTILS_NS {
    void DisableStacktrace() {

    }

    std::string GetStacktrace() {
        return "Emscripten not support stack trace!";
    }
}