//
// Created by Monika on 21.09.2021.
//

#ifndef SR_ENGINE_UTILS_SERIALIZABLE_H
#define SR_ENGINE_UTILS_SERIALIZABLE_H

#include <Utils/TypeTraits/SRClass.h>

namespace SR_UTILS_NS {
    class ISerializer;
    class IDeserializer;

    namespace SerializationFlagsEnumWrappper { enum SerializationFlags : uint64_t; }
    using SerializationFlags = SerializationFlagsEnumWrappper::SerializationFlags;

    class Serializable : public SRClass {
        using Super = SRClass;
        SR_CLASS()
    public:
        using OriginType = Serializable;

    public:
        Serializable();

        virtual void Save(ISerializer& serializer) const;
        virtual bool Load(IDeserializer& deserializer);

        virtual void VerifyAfterLoad(SerializableVerifyContext& context) const noexcept { }

        virtual void OnPreSave() { }
        virtual void OnPostSave() { }

        virtual void OnPreLoad() { }
        virtual void OnPostLoad() { }

        void AddSerializationFlags(SerializationFlags flags) noexcept;
        void RemoveSerializationFlags(SerializationFlags flags) noexcept;

        SR_NODISCARD bool HasSerializationFlags(SerializationFlags flags) const noexcept;

    private:
        SerializationFlags m_flags;

    };
}

#endif //SR_ENGINE_UTILS_SERIALIZABLE_H
