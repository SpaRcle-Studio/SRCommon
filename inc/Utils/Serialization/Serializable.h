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
        virtual bool Load(IDeserializer& deserializer);

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

    template<typename T>
    struct ObjectDataAccessor<T, typename std::enable_if<SerializationTraits<T>::IsSerializable>::type> {
        static void Save(ISerializer& serializer, const T& value, const SerializationId& id) {
            serializer.BeginObject(id);
            static_cast<const Serializable&>(value).Save(serializer);
            serializer.EndObject();
        }

        static void Load(IDeserializer& deserializer, T& value, const SerializationId& id) {
            if (!deserializer.BeginObject(id)) {
                return;
            }
            static_cast<Serializable&>(value).Load(deserializer);
            deserializer.EndObject();
        }
    };
}

#endif //SR_ENGINE_UTILS_SERIALIZABLE_H
