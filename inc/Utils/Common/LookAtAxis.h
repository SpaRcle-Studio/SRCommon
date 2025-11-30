//
// Created by Monika on 30.11.2025.
//

#ifndef SR_ENGINE_UTILS_LOOK_AT_AXIS_H
#define SR_ENGINE_UTILS_LOOK_AT_AXIS_H

#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    /// Ось, которая будет разворачиваться в сторону цели
    SR_ENUM_NS_CLASS_T(LookAtAxis, uint8_t,
       AxisX,
       AxisY,
       AxisZ,
       InvAxisX,
       InvAxisY,
       InvAxisZ
    );
}

#endif //SR_ENGINE_UTILS_LOOK_AT_AXIS_H
