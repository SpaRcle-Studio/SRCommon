//
// Created by Monika on 01.12.2025.
//

#ifndef SR_ENGINE_UTILS_PLATFORM_TYPE_H
#define SR_ENGINE_UTILS_PLATFORM_TYPE_H

#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(PlatformType, uint8_t,
        Unknown, Windows, Linux, Android, MacOS, Emscripten, IOS
    );
}

#endif //SR_ENGINE_UTILS_PLATFORM_TYPE_H
