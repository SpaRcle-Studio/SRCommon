//
// Created by Monika on 21.09.2021.
//

#ifndef SR_ENGINE_UTILS_SERIALIZABLE_H
#define SR_ENGINE_UTILS_SERIALIZABLE_H

#include <Utils/TypeTraits/SRClass.h>
#include <Utils/TypeTraits/Factory.h>

#include <Utils/Serialization/Serialization.h>

namespace SR_UTILS_NS {
    class Serializable : public SRClass {
        SR_CLASS()
    public:
        using OriginType = Serializable;

    public:
        virtual void Save(ISerializer& serializer) const;
        virtual void Load(IDeserializer& deserializer);

        virtual void VerifyAfterLoad(SerializableVerifyContext& context) const noexcept { }

        virtual void OnPreSave() { }
        virtual void OnPostSave() { }

        virtual void OnPreLoad() { }
        virtual void OnPostLoad() { }

        void AddSerializationFlags(SerializationFlags flags) noexcept { m_flags |= flags; }
        void RemoveSerializationFlags(SerializationFlags flags) noexcept { m_flags &= ~flags; }

        SR_NODISCARD bool HasSerializationFlags(SerializationFlags flags) const noexcept;

    private:
        SerializationFlags m_flags = SerializationFlags::None;

    };
}

#endif //SR_ENGINE_UTILS_SERIALIZABLE_H
