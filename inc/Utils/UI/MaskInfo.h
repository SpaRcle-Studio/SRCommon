//
// Created by Monika on 27.01.2026.
//

#ifndef SR_ENGINE_COMMON_UI_MASK_INFO_H
#define SR_ENGINE_COMMON_UI_MASK_INFO_H

#include <Utils/Math/Rect.h>

namespace SR_UTILS_NS::UI {
    struct SR_COMMON_DLL_API MaskInfo {
        SR_MATH_NS::IRect rect;
        SR_MATH_NS::IVector2 referenceSize;
        bool hasMask : 4 = false;
        bool scissor : 4 = false;

        bool operator==(const MaskInfo& other) const {
            return rect == other.rect && hasMask == other.hasMask && scissor == other.scissor && referenceSize == other.referenceSize;
        }

        bool operator!=(const MaskInfo& other) const {
            return !(*this == other);
        }

        SR_NODISCARD static MaskInfo Combine(const MaskInfo& parent, const MaskInfo& self) {
            if (!self.hasMask) {
                return parent;
            }

            MaskInfo r;

            r.hasMask = parent.hasMask || self.hasMask;
            if (!r.hasMask) {
                return r;
            }

            r.referenceSize = self.referenceSize;

            if (parent.scissor && self.scissor) {
                r.scissor = true;
                r.rect = parent.rect.IntersectInclusive(self.rect);
            }
            else if (parent.scissor) {
                r.scissor = true;
                r.rect = parent.rect;
            }
            else if (self.scissor) {
                r.scissor = true;
                r.rect = self.rect;
            }
            else {
                r.scissor = false;
                r.rect = {};
            }

            return r;
        }

    };
}

#endif //SR_ENGINE_COMMON_UI_MASK_INFO_H
