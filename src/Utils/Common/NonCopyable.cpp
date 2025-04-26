//
// Created by Monika on 26.04.2025.
//

#include <Utils/Common/NonCopyable.h>

namespace SR_UTILS_NS {
    SR_COMMON_DLL_API NonCopyable::NonCopyable() = default;
    SR_COMMON_DLL_API NonCopyable::~NonCopyable() = default;

    SR_COMMON_DLL_API NonMovable::NonMovable() = default;
    SR_COMMON_DLL_API NonMovable::~NonMovable() = default;

    SR_COMMON_DLL_API ICloneable::ICloneable() = default;
    SR_COMMON_DLL_API ICloneable::~ICloneable() = default;
    SR_COMMON_DLL_API void ICloneable::OnCloned() { }
}