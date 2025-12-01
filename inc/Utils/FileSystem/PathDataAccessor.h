//
// Created by Monika on 01.12.2025.
//

#ifndef SR_ENGINE_UTILS_PATH_DATA_ACCESSOR_H
#define SR_ENGINE_UTILS_PATH_DATA_ACCESSOR_H

#include <Utils/Serialization/ObjectDataAccessor.h>

namespace SR_UTILS_NS {
    class ISerializer;
    class IDeserializer;
    class Path;

    template<> struct ObjectDataAccessor<SR_UTILS_NS::Path> {
        static void Save(ISerializer& serializer, const SR_UTILS_NS::Path& value, const SerializationId& id) {
            value.Save(serializer, id);
        }

        static void Load(IDeserializer& deserializer, SR_UTILS_NS::Path& value, const SerializationId& id) {
            value.Load(deserializer, id);
        }
    };
}

#endif //SR_ENGINE_UTILS_PATH_DATA_ACCESSOR_H
