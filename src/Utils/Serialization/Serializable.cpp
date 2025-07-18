//
// Created by Monika on 22.09.2021.
//

#include <Utils/Serialization/Serializable.h>

#include <Codegen/Serializable.generated.hpp>

namespace SR_UTILS_NS {
    void Serializable::Save(ISerializer& serializer) const {
        const_cast<Serializable&>(static_cast<const Serializable&>(*this)).OnPreSave();
        GetMeta()->Save(serializer, *this);
        const_cast<Serializable&>(static_cast<const Serializable&>(*this)).OnPostSave();
    }

    bool Serializable::Load(IDeserializer& deserializer) {
        const_cast<Serializable&>(static_cast<const Serializable&>(*this)).OnPreLoad();
        if (!GetMeta()->Load(deserializer, *this)) {
            return false;
        }
        const_cast<Serializable&>(static_cast<const Serializable&>(*this)).OnPostLoad();
        return true;
    }

    bool Serializable::HasSerializationFlags(const SerializationFlags flags) const noexcept {
        return SR_MATH_NS::IsMaskIncludedSubMask(m_flags, flags);
    }
}
