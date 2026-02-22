//
// Created by Monika on 22.02.2026.
//

#ifndef SR_ENGINE_COMMON_RESOURCE_LOAD_MODE_H
#define SR_ENGINE_COMMON_RESOURCE_LOAD_MODE_H

#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(ResourceLoadMode, uint8_t,
        Async, Sync
    );
}

#endif //SR_ENGINE_COMMON_RESOURCE_LOAD_MODE_H
