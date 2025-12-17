//
// Created by Monika on 11.12.2025.
//

#ifndef SR_ENGINE_UTILS_SERIALIZATION_TRAITS_H
#define SR_ENGINE_UTILS_SERIALIZATION_TRAITS_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class Serializable;

    template<class T>
    using HasMethodEmptyT = decltype(std::declval<const T&>().empty());

    template<typename T>
    struct SerializationTraits
    {
        static constexpr bool HasEmpty = IsDetectedV<HasMethodEmptyT, T>;
        static constexpr bool IsSerializable = std::is_base_of_v<SR_UTILS_NS::Serializable, T>;
    };
}

#endif //SR_ENGINE_UTILS_SERIALIZATION_TRAITS_H
