//
// Created by Monika on 27.04.2026.
//

#ifndef SR_ENGINE_COMMON_MATH_PHYSICAL_UNIT_H
#define SR_ENGINE_COMMON_MATH_PHYSICAL_UNIT_H

#include <Utils/Common/Enumerations.h>

namespace SR_MATH_NS {
    SR_ENUM_NS_CLASS_T(PhysicalUnit, uint8_t,
        Centimeters,
        Millimeters,
        Inches,
        Points,   /// 1/72 inch
        Picas     /// 1/6 inch
    )

    SR_COMMON_DLL_API extern float_t GetPhysicalUnitScale(PhysicalUnit unit);
}

#endif //SR_ENGINE_COMMON_MATH_PHYSICAL_UNIT_H
