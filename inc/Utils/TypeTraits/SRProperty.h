//
// Created by Monika on 14.10.2024.
//

#ifndef SR_ENGINE_UTILS_TYPE_TRAITS_SR_PROPERTY_H
#define SR_ENGINE_UTILS_TYPE_TRAITS_SR_PROPERTY_H

#include <Utils/Serialization/Serializable.h>
#include <Utils/Types/StringAtom.h>

namespace SR_UTILS_NS {
    class SRPropertyVariant {
        ~SRPropertyVariant() = default;

        SR_ENUM_CLASS_T(Type, uint8_t,
            Float, Double, String, StringAtom, Bool,
            Int8, Int16, Int32, Int64,
            UInt8, UInt16, UInt32, UInt64,
            Unknown
        );

        struct Object {
            StringAtom type;
            SR_UTILS_NS::Serializable value;
        };

        /*union SRPropertyData {
            float m_float;
            double m_double;
            std::string m_string;
            StringAtom m_stringAtom;
            bool m_bool;
            int8_t m_int8;
            int16_t m_int16;
            int32_t m_int32;
            int64_t m_int64;
            uint8_t m_uint8;
            uint16_t m_uint16;
            uint32_t m_uint32;
            uint64_t m_uint64;
        };*/
    };

    class SRProperty {
    private:
        StringAtom m_name;
        StringAtom m_type;
        StringAtom m_description;

    };
}

#endif //SR_ENGINE_UTILS_TYPE_TRAITS_SR_PROPERTY_H
