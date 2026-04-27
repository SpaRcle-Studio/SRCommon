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

    SR_FORCE_INLINE float_t GetPhysicalUnitScale(PhysicalUnit unit) {
        switch (unit) {
            case PhysicalUnit::Inches:      return 1.f;
            case PhysicalUnit::Centimeters: return 2.54f;
            case PhysicalUnit::Millimeters: return 25.4f;
            case PhysicalUnit::Points:      return 72.f;
            case PhysicalUnit::Picas:       return 6.f;
            default:
                break;
        }
        return 1.f;
    }
}

#endif //SR_ENGINE_COMMON_MATH_PHYSICAL_UNIT_H
