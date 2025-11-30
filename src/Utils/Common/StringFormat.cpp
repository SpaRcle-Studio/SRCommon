//
// Created by Monika on 19.11.2023.
//

#include <Utils/Common/StringFormat.h>
#include <Utils/Platform/Platform.h>

namespace SR_UTILS_NS {
    namespace Details {
        void StringFormatError(const std::string& msg, bool breakPoint) {
            SR_PLATFORM_NS::WriteConsoleError(msg);
            if (breakPoint) {
                SR_UTILS_NS::Breakpoint();
            }
        }
        uint64_t StringFormatHashStrView(std::string_view sv) {
            return SR_HASH_STR_VIEW(sv);
        }
    }
}