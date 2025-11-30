//
// Created by Monika on 30.11.2025.
//

#include <Utils/Common/MarshalUtils.h>
#include <Utils/Common/LexicalCast.h>

namespace SR_UTILS_NS::MarshalUtils {
    void Encode(SR_HTYPES_NS::Stream& stream, const std::string& str, StandardType type) {
        switch (type) {
            case StandardType::Bool: SaveValue(stream, LexicalCast<bool>(str)); break;
            case StandardType::Int8: SaveValue(stream, LexicalCast<int8_t>(str)); break;
            case StandardType::UInt8: SaveValue(stream, LexicalCast<uint8_t>(str)); break;
            case StandardType::Int16: SaveValue(stream, LexicalCast<int16_t>(str)); break;
            case StandardType::UInt16: SaveValue(stream, LexicalCast<uint16_t>(str)); break;
            case StandardType::Int32: SaveValue(stream, LexicalCast<int32_t>(str)); break;
            case StandardType::UInt32: SaveValue(stream, LexicalCast<uint32_t>(str)); break;
            case StandardType::Int64: SaveValue(stream, LexicalCast<int64_t>(str)); break;
            case StandardType::UInt64: SaveValue(stream, LexicalCast<uint64_t>(str)); break;
            case StandardType::Float: SaveValue(stream, LexicalCast<float_t>(str)); break;
            case StandardType::Double: SaveValue(stream, LexicalCast<double_t>(str)); break;
            case StandardType::String: SaveString(stream, str); break;
            default:
                SRHalt0();
                break;
        }
    }
}
