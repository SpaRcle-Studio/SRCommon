//
// Created by Monika on 06.03.2026.
//

#ifndef SR_ENGINE_COMMON_ENC_METHOD_TYPE_H
#define SR_ENGINE_COMMON_ENC_METHOD_TYPE_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS::Localization {
    enum class EncMethodType {
        Skip = 0,    ///< Skip illegal/unconvertable characters
        Stop = 1,    ///< Stop conversion and throw conversion_error
        Default = Skip  ///< Default method - skip
    };
}

#endif //SR_ENGINE_COMMON_ENC_METHOD_TYPE_H
