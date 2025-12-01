//
// Created by Monika on 02.04.2024.
//

#include <Utils/TypeTraits/SRClass.h>
#include <Utils/Serialization/Serializable.h>

#include <Codegen/SRClass.generated.hpp>

namespace SR_UTILS_NS {
    void SRClass::CloneTo(SRClass& clone) const {
        SR_TRACY_ZONE;
        GetMeta()->CloneTo(*this, clone);
    }
}