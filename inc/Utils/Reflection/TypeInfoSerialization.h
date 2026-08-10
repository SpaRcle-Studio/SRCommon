//
// Created by Monika on 10.08.2026.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_TYPE_INFO_SERIALIZATION_H
#define SR_ENGINE_COMMON_REFLECTION_TYPE_INFO_SERIALIZATION_H

#include <Utils/Types/String.h>

namespace SR_UTILS_NS::Reflection {
    struct TypeInfo;
    extern SR_COMMON_DLL_API TypeInfo* LoadTypeInfo(StringView type);
    extern SR_COMMON_DLL_API void SaveTypeInfo(String& type, const TypeInfo* pTypeInfo);
}

#endif //SR_ENGINE_COMMON_REFLECTION_TYPE_INFO_SERIALIZATION_H
