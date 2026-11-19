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

    /// Читает каноническое имя типа и старый формат "Category.name[...]".
    /// nullptr при ошибке разбора или неизвестном имени; шаблон с незарегистрированной комбинацией аргументов
    /// возвращается нерешённым, см. TypeInfo::IsResolved()
    extern SR_COMMON_DLL_API const TypeInfo* LoadTypeInfo(StringView type);
    extern SR_COMMON_DLL_API void SaveTypeInfo(String& type, const TypeInfo* pTypeInfo);

    extern SR_COMMON_DLL_API void SerializeValue(const Value& value, ISerializer& serializer);
    extern SR_COMMON_DLL_API bool DeserializeValue(Value& value, IDeserializer& deserializer);
}

#endif //SR_ENGINE_COMMON_REFLECTION_TYPE_INFO_SERIALIZATION_H
