//
// Created by Monika on 10.08.2026.
//

#include <Utils/Reflection/TypeInfoSerialization.h>
#include <Utils/Reflection/ReflectedType.h>
#include <Utils/Reflection/Value.h>
#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>

#include <Enum/ReflectedCategoryType.hpp>

namespace SR_UTILS_NS::Reflection {
    struct TypeParser {
        StringView source;
        SizeType pos = 0;

        void SkipSpaces();

        bool Consume(char c) {
            SkipSpaces();

            if (pos < source.size() && source[pos] == c) {
                ++pos;
                return true;
            }

            return false;
        }

        StringView ReadBaseType() {
            const SizeType start = pos;

            while (pos < source.size()) {
                const char c = source[pos];

                if (c == '[' || c == '(' || c == ')' || c == ']' || c == ',') {
                    break;
                }

                ++pos;
            }

            return source.substr(start, pos - start);
        }

        TypeInfo* ParseType();
    };

    void TypeParser::SkipSpaces() {
        while (pos < source.size() && std::isspace(static_cast<unsigned char>(source[pos]))) {
            ++pos;
        }
    }

    TypeInfo* TypeParser::ParseType() {
        SkipSpaces();

        TypeInfo* pTypeInfo = AllocateTypeInfo();
        StringView baseType = ReadBaseType();

        const SizeType dotPos = baseType.find('.');
        if (dotPos == StringView::npos) {
            return nullptr;
        }

        pTypeInfo->category = EnumReflector::FromString<ReflectedCategoryType>(baseType.substr(0, dotPos));
        pTypeInfo->detailedType = baseType.substr(dotPos + 1);

        if (Consume('(')) {
            SkipSpaces();
            const SizeType start = pos;
            while (pos < source.size() && std::isdigit(static_cast<unsigned char>(source[pos]))) {
                ++pos;
            }

            StringView sizeStr = source.substr(start, pos - start);
            pTypeInfo->detailedSize = static_cast<uint8_t>(std::stoull(std::string(sizeStr)));

            if (!Consume(')')) {
                return nullptr;
            }
        }

        if (Consume('[')) {
            uint32_t index = 0;

            while (true) {
                if (index >= 2) {
                    return nullptr;
                }

                pTypeInfo->pNext[index++] = ParseType();

                if (!pTypeInfo->pNext[index - 1]) {
                    return nullptr;
                }

                SkipSpaces();

                if (Consume(',')) {
                    continue;
                }

                if (Consume(']')) {
                    break;
                }

                return nullptr;
            }
        }

        return pTypeInfo;
    }

    TypeInfo* LoadTypeInfo(StringView type) {
        TypeParser parser{ type };
        return parser.ParseType();
    }

    void SaveTypeInfoInternal(String& type, const TypeInfo* pTypeInfo) { /// NOLINT(misc-no-recursion)
        if (!pTypeInfo) {
            return;
        }

        type += EnumReflector::ToStringAtom(pTypeInfo->category).ToStringView();
        type += '.';
        type += pTypeInfo->detailedType.ToStringView();

        if (pTypeInfo->detailedSize > 0) {
            type += '(';
            FormatTo(type, "{}", pTypeInfo->detailedSize);
            type += ')';
        }

        if (pTypeInfo->pNext[0] || pTypeInfo->pNext[1]) {
            type += '[';

            for (uint32_t i = 0; i < 2; ++i) {
                if (pTypeInfo->pNext[i]) {
                    type += '[';
                    SaveTypeInfoInternal(type, pTypeInfo->pNext[i]);
                    type += ']';
                }

                if (i == 0 && pTypeInfo->pNext[1]) {
                    type += ',';
                }
            }

            type += ']';
        }
    }

    void SaveTypeInfo(String& type, const TypeInfo* pTypeInfo) {
        type.clear();
        type.reserve(64);
        SaveTypeInfoInternal(type, pTypeInfo);
    }

    static SerializationId SERIALIZE_AXIS_ID[6] = {
        SerializationId::Create("x"),
        SerializationId::Create("y"),
        SerializationId::Create("z"),
        SerializationId::Create("w"),
        SerializationId::Create("u"),
        SerializationId::Create("v")
    };

    void SerializeValue(const Value& value, ISerializer& serializer) {
        auto&& typeInfo = value.GetTypeInfo();
        switch (typeInfo.category) {
            case ReflectedCategoryType::Enum: {
                auto&& pReflector = EnumReflectorManager::Instance().TryGetReflector(typeInfo.detailedType);
                if (!pReflector) {
                    SR_WARN("SerializeValue() : unknown enum type: {}", typeInfo.detailedType);
                    return;
                }
                serializer.WriteString(pReflector->ToStringInternal(*value.Cast<int64_t>()).value(), SerializationId::Create("enum"));
                break;
            }
            case ReflectedCategoryType::String: {
                if (typeInfo.detailedType == "Path") {
                    serializer.WriteString(value.Cast<Path>()->View(), SerializationId::Create("path"));
                }
                else if (typeInfo.detailedType == "StringView") {
                    serializer.WriteString(*value.Cast<StringView>(), SerializationId::Create("view"));
                }
                else if (typeInfo.detailedType == "StringAtom") {
                    serializer.WriteString(*value.Cast<StringAtom>(), SerializationId::Create("atom"));
                }
                else if (typeInfo.detailedType == "UnicodeString") {
                    serializer.WriteString(*value.Cast<UnicodeString>(), SerializationId::Create("unicode"));
                }
                else if (typeInfo.detailedType == "String") {
                    serializer.WriteString(*value.Cast<String>(), SerializationId::Create("string"));
                }
                else {
                    SR_WARN("SerializeValue() : unknown string type: {}", typeInfo.detailedType);
                }
                break;
            }
            case ReflectedCategoryType::Arithmetic:
                if (typeInfo.detailedType == "bool") {
                    serializer.WriteBool(*value.Cast<bool>(), SerializationId::Create("bool"));
                }
                else if (typeInfo.detailedType == "float") {
                    serializer.WriteFloat(*value.Cast<float>(), SerializationId::Create("float"));
                }
                else if (typeInfo.detailedType == "double") {
                    serializer.WriteDouble(*value.Cast<double>(), SerializationId::Create("double"));
                }
                else {
                    serializer.WriteInt(*value.Cast<int64_t>(), SerializationId::Create("int"));
                }
                break;
            case ReflectedCategoryType::MathVector: {
                for (uint8_t i = 0; i < typeInfo.detailedSize; ++i) {
                    serializer.WriteInt(value.Cast<int64_t>()[i], SERIALIZE_AXIS_ID[i]);
                }
                break;
            }
            default:
                SR_WARN("SerializeValue() : unknown reflected type: {}", typeInfo.category);
                return;
        }
    }

    bool DeserializeValue(Value& value, IDeserializer& deserializer) {
        auto&& typeInfo = value.GetTypeInfo();
        switch (typeInfo.category) {
            case ReflectedCategoryType::Enum: {
                auto&& pReflector = EnumReflectorManager::Instance().GetReflector(typeInfo.detailedType);
                if (!pReflector) {
                    SR_WARN("DeserializeValue() : unknown enum type: {}", typeInfo.detailedType);
                    return false;
                }
                String enumStr;
                deserializer.ReadString(enumStr, SerializationId::Create("enum"));
                *value.Cast<int64_t>() = pReflector->FromStringInternal(enumStr).value_or(0);
                return true;
            }
            case ReflectedCategoryType::String: {
                if (typeInfo.detailedType == "Path") {
                    Path& path = *value.Cast<Path>();
                    deserializer.ReadString(path, SerializationId::Create("path"));
                }
                else if (typeInfo.detailedType == "StringView") {
                    String& str = *value.Cast<String>();
                    deserializer.ReadString(str, SerializationId::Create("view"));
                }
                else if (typeInfo.detailedType == "StringAtom") {
                    String& str = *value.Cast<String>();
                    deserializer.ReadString(str, SerializationId::Create("atom"));
                }
                else if (typeInfo.detailedType == "UnicodeString") {
                    UnicodeString& str = *value.Cast<UnicodeString>();
                    deserializer.ReadString(str, SerializationId::Create("unicode"));
                }
                else if (typeInfo.detailedType == "String") {
                    String& str = *value.Cast<String>();
                    deserializer.ReadString(str, SerializationId::Create("string"));
                }
                else {
                    SRHalt("DeserializeValue() : unknown string type: {}", typeInfo.detailedType);
                }
                return true;
            }
            case ReflectedCategoryType::Arithmetic: {
                if (typeInfo.detailedType == "bool") {
                    deserializer.ReadBool(*value.Cast<bool>(), SerializationId::Create("bool"));
                }
                else if (typeInfo.detailedType == "float") {
                    deserializer.ReadFloat(*value.Cast<float>(), SerializationId::Create("float"));
                }
                else if (typeInfo.detailedType == "double") {
                    deserializer.ReadDouble(*value.Cast<double>(), SerializationId::Create("double"));
                }
                else {
                    deserializer.ReadInt(*value.Cast<int64_t>(), SerializationId::Create("int"));
                }
                return true;
            }
            case ReflectedCategoryType::MathVector: {
                for (uint8_t i = 0; i < typeInfo.detailedSize; ++i) {
                    deserializer.ReadInt(value.Cast<int64_t>()[i], SERIALIZE_AXIS_ID[i]);
                }
                return true;
            }
            default:
                break;
        }
        SR_WARN("DeserializeValue() : unknown reflected type: {}", typeInfo.category);
        return false;
    }
}