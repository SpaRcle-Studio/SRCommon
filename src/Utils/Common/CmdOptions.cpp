//
// Created by innerviewer on 2024-03-18.
//

#include <Utils/Common/CmdOptions.h>

namespace SR_UTILS_NS {
    String GetCmdOption(char **begin, char **end, StringView option) {
        char **itr = std::find_if(begin, end, [&option](const char* pArgument) { return option == pArgument; });
        if (itr != end && ++itr != end) {
            return *itr;
        }
        return String();
    }

    bool HasCmdOption(char **begin, char **end, StringView option) {
        char **itr = std::find_if(begin, end, [&option](const char* pArgument) { return option == pArgument; });

        if (*itr == nullptr) {
            return false;
        }

        return true;
    }
}