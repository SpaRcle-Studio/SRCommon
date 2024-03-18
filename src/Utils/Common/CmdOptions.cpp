//
// Created by innerviewer on 2024-03-18.
//

#include <Utils/Common/CmdOptions.h>

namespace SR_UTILS_NS {
    std::string GetCmdOption(char **begin, char **end, const std::string &option) {
        char **itr = std::find(begin, end, option);
        if (itr != end && ++itr != end) {
            return *itr;
        }
        return std::string();
    }

    bool HasCmdOption(char **begin, char **end, const std::string &option) {
        char **itr = std::find(begin, end, option);

        if (*itr == nullptr) {
            return false;
        }

        return true;
    }
}