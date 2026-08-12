//
// Created by Monika on 10.08.2026.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_TYPE_INFO_SERIALIZATION_H
#define SR_ENGINE_COMMON_REFLECTION_TYPE_INFO_SERIALIZATION_H

#include <Utils/Types/String.h>

namespace SR_UTILS_NS {
    class ISerializer;
    class IDeserializer;
}

namespace SR_UTILS_NS::Reflection {
    struct TypeInfo;
    struct Value;

    extern SR_COMMON_DLL_API TypeInfo* LoadTypeInfo(StringView type);
    extern SR_COMMON_DLL_API void SaveTypeInfo(String& type, const TypeInfo* pTypeInfo);

    extern SR_COMMON_DLL_API void SerializeValue(const Value& value, ISerializer& serializer);
    extern SR_COMMON_DLL_API bool DeserializeValue(Value& value, IDeserializer& deserializer);
}

#endif //SR_ENGINE_COMMON_REFLECTION_TYPE_INFO_SERIALIZATION_H
