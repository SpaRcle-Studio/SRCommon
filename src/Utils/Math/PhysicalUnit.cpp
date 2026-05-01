//
// Created by Monika on 02.05.2026.
//

#include <Utils/Math/PhysicalUnit.h>

namespace SR_MATH_NS {
    float_t GetPhysicalUnitScale(PhysicalUnit unit) {
        switch (unit) {
            case PhysicalUnit::Inches:      return 1.f;
            case PhysicalUnit::Centimeters: return 2.54f;
            case PhysicalUnit::Millimeters: return 25.4f;
            case PhysicalUnit::Points:      return 72.f;
            case PhysicalUnit::Picas:       return 6.f;
            default: break;
        }
        return 1.f;
    }
}