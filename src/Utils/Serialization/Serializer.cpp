//
// Created by Monika on 16.10.2024.
//

#include <Utils/Serialization/Serializer.h>
#include <Utils/Common/TypeInfo.h>
#include <Utils/Debug.h>

namespace SR_UTILS_NS {
    void ISerializer::WriteAny(const std::any& value, const SerializationId& name) {
        SRHalt("ISerializer::WriteAny() : not implemented!");
        /*auto&& type = GetStandardType(value);

        BeginObject(name);

        WriteString(SR_UTILS_NS::EnumReflector::ToStringAtom(type), SerializationId::Create("type"));

        static constexpr auto&& id = SR_UTILS_NS::SerializationId::Create("any");

        switch (type) {
            case StandardType::Bool: WriteBool(std::any_cast<bool>(value), id); break;
            case StandardType::Int8: WriteInt(std::any_cast<int8_t>(value), id); break;
            case StandardType::UInt8: WriteUInt(std::any_cast<uint8_t>(value), id); break;
            case StandardType::Int16: WriteInt(std::any_cast<int16_t>(value), id); break;
            case StandardType::UInt16: WriteUInt(std::any_cast<uint16_t>(value), id); break;
            case StandardType::Int32: WriteInt(std::any_cast<int32_t>(value), id); break;
            case StandardType::UInt32: WriteUInt(std::any_cast<uint32_t>(value), id); break;
            case StandardType::Int64: WriteInt(std::any_cast<int64_t>(value), id); break;
            case StandardType::UInt64: WriteUInt(std::any_cast<uint64_t>(value), id); break;
            case StandardType::Float: WriteFloat(std::any_cast<float_t>(value), id); break;
            case StandardType::Double: WriteDouble(std::any_cast<double_t>(value), id); break;
            case StandardType::String: WriteString(std::any_cast<std::string>(value), id); break;
            default:
                SRHalt0();
                break;
        }

        EndObject();*/
    }
}
