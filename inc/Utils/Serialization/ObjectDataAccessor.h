//
// Created by Monika on 12.02.2025.
//

#ifndef SR_COMMON_OBJECT_DATA_ACCESSOR_H
#define SR_COMMON_OBJECT_DATA_ACCESSOR_H

#include <Utils/TypeTraits/TypeTraits.h>

namespace SR_UTILS_NS {
    class ISerializer;
    class IDeserializer;

    /// Specialization for serializable types

    template<typename T, typename Enable = void> struct ObjectDataAccessor {
        static void Save(ISerializer&, const T&, const SerializationId&) {
            static_assert(AlwaysFalseV<T>, "Unable to save! Need to inherit from Serializable!");
        }

        static void Load(IDeserializer&, T&, const SerializationId&) {
            static_assert(AlwaysFalseV<T>, "Unable to load! Need to inherit from Serializable!");
        }
    };
}

#endif //SR_COMMON_OBJECT_DATA_ACCESSOR_H
