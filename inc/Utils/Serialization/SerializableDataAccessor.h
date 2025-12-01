//
// Created by Monika on 01.12.2025.
//

#ifndef SR_ENGINE_UTILS_SERIALIZABLE_DATA_ACCESSOR_H
#define SR_ENGINE_UTILS_SERIALIZABLE_DATA_ACCESSOR_H

#include <Utils/Serialization/Serializable.h>
#include <Utils/Serialization/SerializationFlags.h>
#include <Utils/Serialization/Serialization.h>

namespace SR_UTILS_NS {
    class Serializable;

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

#endif //SR_ENGINE_UTILS_SERIALIZABLE_DATA_ACCESSOR_H
