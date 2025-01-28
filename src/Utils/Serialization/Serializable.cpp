//
// Created by Monika on 22.09.2021.
//

#include <Utils/Serialization/Serializable.h>

#include <Codegen/Serializable.generated.hpp>

namespace SR_UTILS_NS {
    void Serializable::Save(ISerializer& serializer) const {
        SR_TRACY_ZONE;
        GetMeta()->Save(serializer, *this);
    }

    void Serializable::Load(IDeserializer& deserializer) {
        SR_TRACY_ZONE;
        GetMeta()->Load(deserializer, *this);
    }

    bool Serializable::HasSerializationFlags(const SerializationFlags flags) const noexcept {
        return SR_MATH_NS::IsMaskIncludedSubMask(m_flags, flags);
    }

    SR_HTYPES_NS::Marshal::Ptr Serializable::SaveLegacy(SavableContext data) const  {
        SR_TRACY_ZONE;

        if (HasSerializationFlags(SerializationFlags::DontSave)) {
            return nullptr;
        }

        if (data.pMarshal) {
            return data.pMarshal;
        }

        return new SR_HTYPES_NS::Marshal();
    }
}
