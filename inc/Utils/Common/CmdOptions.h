//
// Created by Monika on 22.11.2021.
//

#ifndef SR_ENGINE_CMDOPTIONS_H
#define SR_ENGINE_CMDOPTIONS_H

#include <Utils/Debug.h>

namespace SR_UTILS_NS {
    extern String GetCmdOption(char **begin, char **end, StringView option);
    extern bool HasCmdOption(char **begin, char **end, StringView option);
}

#endif //SR_ENGINE_CMDOPTIONS_H
