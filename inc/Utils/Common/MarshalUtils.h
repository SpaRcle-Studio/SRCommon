//
// Created by Monika on 30.03.2022.
//

#ifndef SR_ENGINE_MARSHALUTILS_H
#define SR_ENGINE_MARSHALUTILS_H

#include <Utils/Common/StringFormat.h>
#include <Utils/Math/Vector3.h>
#include <Utils/Math/Vector4.h>
#include <Utils/Common/TypeInfo.h>
#include <Utils/Common/AnyMarshal.h>
#include <Utils/Common/BaseMarshal.h>

namespace SR_UTILS_NS {
    namespace MarshalUtils {
        SR_MAYBE_UNUSED void Encode(SR_HTYPES_NS::Stream& stream, const std::string& str, StandardType type);
    }

    enum class MarshalSaveMode {
        Binary,
        Json
    };
}

#endif //SR_ENGINE_MARSHALUTILS_H
