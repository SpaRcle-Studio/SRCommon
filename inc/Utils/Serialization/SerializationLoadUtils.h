//
// Created by Monika on 11.12.2025.
//

#ifndef SR_ENGINE_UTILS_SERIALIZATION_LOAD_UTILS_H
#define SR_ENGINE_UTILS_SERIALIZATION_LOAD_UTILS_H

namespace SR_UTILS_NS {
    /// Default object makers for serialization

    #include <Utils/Serialization/DefaultObjectMakers.inl.h>

    namespace Serialization {
        template<typename T> bool Load(IDeserializer& deserializer, T& value, const SerializationId& key) {
            if (!deserializer.IsDefault(key)) {
                ObjectDataAccessor<T>::Load(deserializer, value, key);
                return true;
            }

            if constexpr (DefaultObjectMaker<T>::value) {
                if (deserializer.ShouldSetDefaults(key)) {
                    DefaultObjectMaker<T>::MakeDefault(value);
                }
            }
            return false;
        }
    }
}

#endif //SR_ENGINE_UTILS_SERIALIZATION_LOAD_UTILS_H
