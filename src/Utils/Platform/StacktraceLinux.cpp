//
// Created by Monika on 02.11.2021.
//

#include <Utils/Platform/Stacktrace.h>
#include <cpptrace/cpptrace.hpp>

namespace SR_UTILS_NS {
    static bool g_stackStraceEnabled = true;

    void DisableStacktrace() {
        g_stackStraceEnabled = false;
    }

    String GetStacktrace() {
        if (!g_stackStraceEnabled) {
            return String();
        }

        auto&& stacktrace = cpptrace::generate_trace();

        return String(stacktrace.to_string(true));
    }
}


