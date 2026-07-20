//
// Created by Monika on 19.11.2023.
//

#ifndef SR_ENGINE_UTILS_TO_STRING_H
#define SR_ENGINE_UTILS_TO_STRING_H

#include <Utils/Common/Enumerations.h>
#include <Utils/Math/Mathematics.h>

namespace SR_UTILS_NS {
    static SR_INLINE StringView SerializeDouble(double_t value, char* pBuffer, uint64_t bufferSize) {
        const int32_t result = std::snprintf(pBuffer, bufferSize, "%.17g", value);
        return StringView(pBuffer, result);
    }

    static SR_INLINE StringView SerializeFloat(float_t value, char* pBuffer, uint64_t bufferSize) {
        const int32_t result = std::snprintf(pBuffer, bufferSize, "%.6g", value);
        return StringView(pBuffer, result);
    }

    static SR_INLINE StringView SerializeInt(int64_t value, char* pBuffer, uint64_t bufferSize) {
        if (bufferSize == 0) {
            return {};
        }

        const int result = std::snprintf(pBuffer, bufferSize, "%" PRId64, value);
        if (result < 0) {
            return {};
        }
        const uint64_t length = std::min<uint64_t>(static_cast<uint64_t>(result), bufferSize - 1);
        return StringView(pBuffer, length);
    }

    template<typename T> std::string ToString(const T& value) {
        if constexpr (IsLogical<T>()) {
            return value ? "true" : "false";
        }
        else if constexpr (SR_MATH_NS::IsNumber<T>()) {
            return std::to_string(value);
        }
        else if constexpr (IsString<T>()) {
            return value;
        }
        else if constexpr (std::is_enum_v<T>) {
            return SR_UTILS_NS::EnumReflector::ToStringAtom(value);
        }
        else {
            SRHalt("ToStringAtom: unsupported type!");
            return std::string(); /// NOLINT
        }
    }
}

#endif //SR_ENGINE_UTILS_TO_STRING_H
