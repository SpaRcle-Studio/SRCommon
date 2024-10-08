//
// Created by Monika on 22.09.2021.
//

#include <Utils/Serialization/ISerializable.h>

namespace SR_UTILS_NS {
    bool ISerializable::HasSerializationFlags(ObjectSerializationFlagsFlag flags) const noexcept {
        return SR_MATH_NS::IsMaskIncludedSubMask(m_flags, flags);
    }

    SR_HTYPES_NS::Marshal::Ptr ISerializable::SaveLegacy(SavableContext data) const  {
        if (HasSerializationFlags(ObjectSerializationFlags::DontSave)) {
            return nullptr;
        }

        if (data.pMarshal) {
            return data.pMarshal;
        }

        return new SR_HTYPES_NS::Marshal();
    }
}
