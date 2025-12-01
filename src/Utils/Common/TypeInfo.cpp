//
// Created by Monika on 11.11.2025.
//

#include <Utils/Common/TypeInfo.h>
#include <Utils/Common/Breakpoint.h>
#include <Utils/Platform/Platform.h>

namespace SR_UTILS_NS {
    uint64_t GetTypeSize(const StandardType& type) {
        switch (type) {
            case StandardType::Bool: return sizeof(bool);
            case StandardType::Int8: return sizeof(int8_t);
            case StandardType::UInt8: return sizeof(uint8_t);
            case StandardType::Int16: return sizeof(int16_t);
            case StandardType::UInt16: return sizeof(uint16_t);
            case StandardType::Int32: return sizeof(int32_t);
            case StandardType::UInt32: return sizeof(uint32_t);
            case StandardType::Int64: return sizeof(int64_t);
            case StandardType::UInt64: return sizeof(uint64_t);
            case StandardType::Float: return sizeof(float_t);
            case StandardType::Double: return sizeof(double_t);
            default:
                SR_PLATFORM_NS::WriteConsoleError("Unknown type size!");
                SR_UTILS_NS::Breakpoint();
                return 0;
        }
    }

    bool IsNumberType(const StandardType& type) {
        switch (type) {
            case StandardType::Int8:
            case StandardType::UInt8:
            case StandardType::Int16:
            case StandardType::UInt16:
            case StandardType::Int32:
            case StandardType::UInt32:
            case StandardType::Int64:
            case StandardType::UInt64:
            case StandardType::Float:
            case StandardType::Double:
                return true;
            default:
                return false;
        }
    }
}
