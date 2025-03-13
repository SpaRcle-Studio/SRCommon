//
// Created by Monika on 16.10.2024.
//

#include <Utils/Serialization/Deserializer.h>

namespace SR_UTILS_NS {
    void IDeserializer::ReadAny(std::any& value, const SerializationId& name) {
        BeginObject(name);

        std::string typeStr;
        ReadString(typeStr, SerializationId::Create("type"));

        const auto&& type = SR_UTILS_NS::EnumReflector::FromString<SR_UTILS_NS::StandardType>(typeStr);

        static constexpr auto&& id = SerializationId::Create("any");

        switch (type) {
            case StandardType::Bool: {
                bool v = false;
                ReadBool(v, id);
                value = v;
                break;
            }
            case StandardType::Int8: {
                int8_t v = 0;
                ReadInt(v, id);
                value = v;
                break;
            }
            case StandardType::UInt8: {
                uint8_t v = 0;
                ReadUInt(v, id);
                value = v;
                break;
            }
            case StandardType::Int16: {
                int16_t v = 0;
                ReadInt(v, id);
                value = v;
                break;
            }
            case StandardType::UInt16: {
                uint16_t v = 0;
                ReadUInt(v, id);
                value = v;
                break;
            }
            case StandardType::Int32: {
                int32_t v = 0;
                ReadInt(v, id);
                value = v;
                break;
            }
            case StandardType::UInt32: {
                uint32_t v = 0;
                ReadUInt(v, id);
                value = v;
                break;
            }
            case StandardType::Int64: {
                int64_t v = 0;
                ReadInt(v, id);
                value = v;
                break;
            }
            case StandardType::UInt64: {
                uint64_t v = 0;
                ReadUInt(v, id);
                value = v;
                break;
            }
            case StandardType::Float: {
                float_t v = 0.f;
                ReadFloat(v, id);
                value = v;
                break;
            }
            case StandardType::Double: {
                double_t v = 0.0;
                ReadDouble(v, id);
                value = v;
                break;
            }
            case StandardType::String: {
                std::string v;
                ReadString(v, id);
                value = v;
                break;
            }
            default:
                SRHalt0();
                break;
        }
    }
}
