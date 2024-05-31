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

    std::string GetStacktrace() {
        if (!g_stackStraceEnabled) {
            return std::string();
        }

        auto&& stacktrace = cpptrace::generate_trace();
        auto&& result = stacktrace.to_string(true);

        return result;
    }
}


