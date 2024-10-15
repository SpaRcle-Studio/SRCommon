//
// Created by Monika on 22.09.2021.
//

#include <Utils/Serialization/Serializable.h>

#include <Codegen/Serializable.generated.hpp>

namespace SR_UTILS_NS {
    void Serializable::Save(ISerializer& serializer) const {
        GetMeta()->Save(serializer);
    }

    void Serializable::Load(IDeserializer& deserializer) {
        GetMeta()->Load(deserializer);
    }

    bool Serializable::HasSerializationFlags(const ObjectSerializationFlagsFlag flags) const noexcept {
        return SR_MATH_NS::IsMaskIncludedSubMask(m_flags, flags);
    }

    SR_HTYPES_NS::Marshal::Ptr Serializable::SaveLegacy(SavableContext data) const  {
        if (HasSerializationFlags(ObjectSerializationFlags::DontSave)) {
            return nullptr;
        }

        if (data.pMarshal) {
            return data.pMarshal;
        }

        return new SR_HTYPES_NS::Marshal();
    }
}
