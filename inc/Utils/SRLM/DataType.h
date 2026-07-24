//
// Created by Monika on 25.08.2023.
//

#ifndef SR_ENGINE_DATATYPE_H
#define SR_ENGINE_DATATYPE_H

#include <Utils/SRLM/Utils.h>
#include <Utils/Resources/Xml.h>

namespace SR_UTILS_NS {
    class EnumReflector;
}

namespace SR_SRLM_NS {
    SR_ENUM_NS_CLASS_T(DataTypeClass, uint8_t,
        None, Flow, Enum, Struct, Array, Map, Set,
        Bool, Float, Double, String,
        Int8, Int16, Int32, Int64,
        UInt8, UInt16, UInt32, UInt64
    );
}

#endif //SR_ENGINE_DATATYPE_H
