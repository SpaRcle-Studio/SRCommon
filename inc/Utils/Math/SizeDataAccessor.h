//
// Created by Monika on 01.12.2025.
//

#ifndef SR_ENGINE_UTILS_SIZE_DATA_ACCESSOR_H
#define SR_ENGINE_UTILS_SIZE_DATA_ACCESSOR_H

#include <Utils/Math/Size.h>
#include <Utils/Serialization/ObjectDataAccessor.h>
#include <Utils/Serialization/SerializableDataAccessor.h>

namespace SR_UTILS_NS {
    template<> struct ObjectDataAccessor<SR_MATH_NS::FSize> {
        static void Save(ISerializer& serializer, const SR_MATH_NS::FSize& value, const SerializationId& id) {
            serializer.BeginObject(id);
            Serialization::SaveCheckDefault(serializer, value.v, SerializationId::Create("v"));
            Serialization::SaveCheckDefault(serializer, value.metric, SerializationId::Create("metric"));
            serializer.EndObject();
        }

        static void Load(IDeserializer& deserializer, SR_MATH_NS::FSize& value, const SerializationId& id) {
            if (!deserializer.BeginObject(id)) {
                return;
            }

            Serialization::Load(deserializer, value.v, SerializationId::Create("v"));
            Serialization::Load(deserializer, value.metric, SerializationId::Create("metric"));

            deserializer.EndObject();
        }
    };

    template<> struct ObjectDataAccessor<SR_MATH_NS::FSize2> {
        static void Save(ISerializer& serializer, const SR_MATH_NS::FSize2& value, const SerializationId& id) {
            serializer.BeginObject(id);
            Serialization::SaveCheckDefault(serializer, value.width, SerializationId::Create("width"));
            Serialization::SaveCheckDefault(serializer, value.height, SerializationId::Create("height"));
            serializer.EndObject();
        }
        static void Load(IDeserializer& deserializer, SR_MATH_NS::FSize2& value, const SerializationId& id) {
            if (!deserializer.BeginObject(id)) {
                return;
            }
            Serialization::Load(deserializer, value.width, SerializationId::Create("width"));
            Serialization::Load(deserializer, value.height, SerializationId::Create("height"));
            deserializer.EndObject();
        }
    };
}

#endif //SR_ENGINE_UTILS_SIZE_DATA_ACCESSOR_H
