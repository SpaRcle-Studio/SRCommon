//
// Created by Monika on 01.12.2025.
//

#ifndef SR_ENGINE_UTILS_SERIALIZATION_FLAGS_H
#define SR_ENGINE_UTILS_SERIALIZATION_FLAGS_H

#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    /// Флаги для сериализатора объектов
    SR_ENUM_NS_STRUCT_T(SerializationFlags, uint8_t,
        None     = 1 << 0,
        Compress = 1 << 1,
        NoUID    = 1 << 2,
        DontSave = 1 << 3
    )
}

#endif //SR_ENGINE_UTILS_SERIALIZATION_FLAGS_H
