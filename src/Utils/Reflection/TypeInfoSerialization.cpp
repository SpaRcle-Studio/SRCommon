//
// Created by Monika on 10.08.2026.
//

#include <Utils/Reflection/TypeInfoSerialization.h>
#include <Utils/Reflection/ReflectedType.h>
#include <Utils/Reflection/Value.h>
#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/TypeTraits/Factory.h>

#include <Enum/ReflectedCategoryType.hpp>

namespace SR_UTILS_NS::Reflection {
    /// Новый формат - каноническое имя типа: "float", "FVector3", "KeyCode", "Map<String, Vector<int32>>".
    /// Старый формат - "Category.name(size)[[arg],[arg]]", например "MathVector.float(3)", "Container.Vector[[Arithmetic.float]]".
    struct TypeParser {
        StringView source;
        SizeType pos = 0;

        void SkipSpaces() {
            while (pos < source.size() && std::isspace(static_cast<unsigned char>(source[pos]))) {
                ++pos;
            }
        }

        bool Consume(char c) {
            SkipSpaces();

            if (pos < source.size() && source[pos] == c) {
                ++pos;
                return true;
            }

            return false;
        }

        StringView ReadUntil(StringView stopChars) {
            SkipSpaces();

            const SizeType start = pos;
            while (pos < source.size() && stopChars.find(source[pos]) == StringView::npos && !std::isspace(static_cast<unsigned char>(source[pos]))) {
                ++pos;
            }

            return source.substr(start, pos - start);
        }

        SR_NODISCARD bool IsLegacyFormat() const {
            for (SizeType i = 0; i < source.size(); ++i) {
                const char c = source[i];
                if (c == '.') {
                    return true;
                }
                if (c == '<' || c == '(' || c == ',' || c == '>') {
                    return false;
                }
            }
            return false;
        }

        const TypeInfo* ParseType();
        const TypeInfo* ParseLegacyType();
    };

    const TypeInfo* TypeParser::ParseType() { /// NOLINT(misc-no-recursion)
        const StringView identifier = ReadUntil("<>,");
        if (identifier.empty()) {
            return nullptr;
        }

        if (!Consume('<')) {
            return FindTypeInfoByName(identifier);
        }

        const TypeInfo* pArguments[2] = { nullptr, nullptr };
        uint32_t count = 0;

        do {
            if (count >= 2) {
                return nullptr;
            }
            pArguments[count] = ParseType();
            if (!pArguments[count]) {
                return nullptr;
            }
            ++count;
        } while (Consume(','));

        if (!Consume('>')) {
            return nullptr;
        }

        const TypeKind kind = FindTypeKindByTemplateName(identifier);
        if (kind == TypeKind::Unknown || GetTypeKindArgumentsCount(kind) != count) {
            return nullptr;
        }

        return ResolveTemplateTypeInfo(kind, pArguments[0], pArguments[1]);
    }

    const TypeInfo* ResolveLegacyTypeInfo(ReflectedCategoryType category, StringView name, uint8_t detailedSize, const TypeInfo* const (&pChildren)[2]) {
        TypeInfoKey key;

        switch (category) {
            case ReflectedCategoryType::Unknown:
                return GetUnknownTypeInfo();
            case ReflectedCategoryType::Value:
                return TypeOf<Value>();
            case ReflectedCategoryType::Arithmetic:
            case ReflectedCategoryType::String:
                return FindTypeInfoByName(name);
            case ReflectedCategoryType::MathObject:
                return FindTypeInfoByName(name == "Color" ? StringView("FColor") : name);
            case ReflectedCategoryType::Enum:
                key.kind = TypeKind::Enum;
                key.name = name;
                return ResolveTypeInfo(key);
            case ReflectedCategoryType::Object:
                key.kind = TypeKind::Object;
                key.name = name;
                return ResolveTypeInfo(key);
            case ReflectedCategoryType::MathVector:
            case ReflectedCategoryType::MathRect:
            case ReflectedCategoryType::MathSize: {
                key.kind = category == ReflectedCategoryType::MathVector ? TypeKind::MathVector :
                    category == ReflectedCategoryType::MathRect ? TypeKind::MathRect : TypeKind::MathSize;
                key.detailedSize = detailedSize;
                key.pNext[0] = FindTypeInfoByName(name);
                return key.pNext[0] ? ResolveTypeInfo(key) : nullptr;
            }
            case ReflectedCategoryType::Container:
                /// в старом формате у Map уже записан хранимый аргумент Pair<K, V>
                key.kind = FindTypeKindByTemplateName(name);
                if (key.kind == TypeKind::Unknown) {
                    return nullptr;
                }
                key.pNext[0] = pChildren[0];
                key.pNext[1] = pChildren[1];
                return ResolveTypeInfo(key);
            default:
                return nullptr;
        }
    }

    const TypeInfo* TypeParser::ParseLegacyType() { /// NOLINT(misc-no-recursion)
        SkipSpaces();

        const bool wrapped = Consume('[');

        const StringView baseType = ReadUntil("[()],");

        const SizeType dotPos = baseType.find('.');
        if (dotPos == StringView::npos) {
            return nullptr;
        }

        const auto category = EnumReflector::FromString<ReflectedCategoryType>(baseType.substr(0, dotPos));
        const StringView name = baseType.substr(dotPos + 1);
        uint8_t detailedSize = 0;
        const TypeInfo* pChildren[2] = { nullptr, nullptr };

        if (Consume('(')) {
            SkipSpaces();

            uint32_t size = 0;
            while (pos < source.size() && std::isdigit(static_cast<unsigned char>(source[pos]))) {
                size = size * 10 + static_cast<uint32_t>(source[pos] - '0');
                ++pos;
            }

            detailedSize = static_cast<uint8_t>(size);

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

                pChildren[index++] = ParseLegacyType();

                if (!pChildren[index - 1]) {
                    return nullptr;
                }

                if (Consume(',')) {
                    continue;
                }

                if (Consume(']')) {
                    break;
                }

                return nullptr;
            }
        }

        if (wrapped && !Consume(']')) {
            return nullptr;
        }

        return ResolveLegacyTypeInfo(category, name, detailedSize, pChildren);
    }

    const TypeInfo* LoadTypeInfo(StringView type) {
        TypeParser parser{ type };
        return parser.IsLegacyFormat() ? parser.ParseLegacyType() : parser.ParseType();
    }

    void SaveTypeInfo(String& type, const TypeInfo* pTypeInfo) {
        type.clear();
        if (pTypeInfo) {
            type += pTypeInfo->name.ToStringView();
        }
    }

    static SerializationId SERIALIZE_AXIS_ID[6] = {
        SerializationId::Create("x"),
        SerializationId::Create("y"),
        SerializationId::Create("z"),
        SerializationId::Create("w"),
        SerializationId::Create("u"),
        SerializationId::Create("v")
    };

    /// Целые и компоненты векторов хранятся как int64 независимо от размера типа,
    /// поэтому при чтении и записи используется ровно size байт поля, без выхода за его границы.
    int64_t ReadIntegralBits(const void* pData, const SizeType size, const bool isSigned) {
        switch (size) {
            case 1: return isSigned ? static_cast<int64_t>(*static_cast<const int8_t*>(pData)) : static_cast<int64_t>(*static_cast<const uint8_t*>(pData));
            case 2: return isSigned ? static_cast<int64_t>(*static_cast<const int16_t*>(pData)) : static_cast<int64_t>(*static_cast<const uint16_t*>(pData));
            case 4: return isSigned ? static_cast<int64_t>(*static_cast<const int32_t*>(pData)) : static_cast<int64_t>(*static_cast<const uint32_t*>(pData));
            case 8: return *static_cast<const int64_t*>(pData);
            default:
                SRHalt("ReadIntegralBits() : unsupported size {}!", size);
                return 0;
        }
    }

    void WriteIntegralBits(void* pData, const SizeType size, const int64_t value) {
        switch (size) {
            case 1: *static_cast<uint8_t*>(pData) = static_cast<uint8_t>(value); break;
            case 2: *static_cast<uint16_t*>(pData) = static_cast<uint16_t>(value); break;
            case 4: *static_cast<uint32_t*>(pData) = static_cast<uint32_t>(value); break;
            case 8: *static_cast<int64_t*>(pData) = value; break;
            default:
                SRHalt("WriteIntegralBits() : unsupported size {}!", size);
                break;
        }
    }

    void SerializeValue(const Value& value, ISerializer& serializer) { /// NOLINT(misc-no-recursion)
        auto&& typeInfo = value.GetTypeInfo();

        if (typeInfo.IsArithmetic()) {
            switch (typeInfo.kind) {
                case TypeKind::Bool:
                    serializer.WriteBool(*value.Cast<bool>(), SerializationId::Create("bool"));
                    break;
                case TypeKind::Float:
                    serializer.WriteFloat(*value.Cast<float_t>(), SerializationId::Create("float"));
                    break;
                case TypeKind::Double:
                    serializer.WriteDouble(*value.Cast<double_t>(), SerializationId::Create("double"));
                    break;
                default:
                    serializer.WriteInt(ReadIntegralBits(value.Data(), typeInfo.GetScalarSize(), typeInfo.IsSigned()), SerializationId::Create("int"));
                    break;
            }
            return;
        }

        switch (typeInfo.kind) {
            case TypeKind::Value: {
                auto&& pValue = value.Cast<Value>();
                if (pValue && pValue->IsValid()) {
                    SerializeValue(*pValue, serializer);
                }
                return;
            }
            case TypeKind::Enum: {
                auto&& pReflector = EnumReflectorManager::Instance().TryGetReflector(typeInfo.name);
                if (!pReflector) {
                    SR_WARN("SerializeValue() : unknown enum type: {}", typeInfo.name);
                    return;
                }
                const int64_t enumValue = pReflector->ReadEnumValueFromPointerInternal(value.Data());
                if (pReflector->GetEnumVariantInternal() == EnumVariant::Flags) {
                    /// save names separated by '|'
                    SR_THREAD_LOCAL static String enumStr;
                    enumStr.clear();
                    for (uint64_t i = 0; i < 64; ++i) {
                        if ((enumValue & (1LL << i)) != 0) {
                            if (!enumStr.empty()) {
                                enumStr += '|';
                            }
                            enumStr += pReflector->ToStringInternal(1LL << i).value().ToStringView();
                        }
                    }
                    serializer.WriteString(enumStr, SerializationId::Create("enum"));
                }
                else {
                    serializer.WriteString(pReflector->ToStringInternal(enumValue).value(), SerializationId::Create("enum"));
                }
                break;
            }
            case TypeKind::Vector: {
                const auto array = value.AsArray();
                const auto size = array.Size();
                serializer.BeginArray(size, SerializationId::Create("vector"));
                for (SizeType i = 0; i < size; ++i) {
                    serializer.BeginItem(SerializationId::Create("item"));
                    SerializeValue(array.At(i).ToValue(), serializer);
                    serializer.EndItem();
                }
                serializer.EndArray();
                break;
            }
            case TypeKind::Map:
            case TypeKind::FlatHashMap: {
                const auto map = value.AsMap();
                serializer.BeginArray(map.Size(), SerializationId::Create("map"));
                map.ForEach([&serializer](const ValueRef& key, const ValueRef& mapped) {
                    serializer.BeginItem(SerializationId::Create("item"));
                    serializer.BeginObject(SerializationId::Create("key"));
                    SerializeValue(key.ToValue(), serializer);
                    serializer.EndObject();
                    serializer.BeginObject(SerializationId::Create("value"));
                    SerializeValue(mapped.ToValue(), serializer);
                    serializer.EndObject();
                    serializer.EndItem();
                });
                serializer.EndArray();
                break;
            }
            case TypeKind::Set:
            case TypeKind::FlatHashSet: {
                const auto set = value.AsMap();
                serializer.BeginArray(set.Size(), SerializationId::Create("set"));
                set.ForEach([&serializer](const ValueRef& key, const ValueRef&) {
                    serializer.BeginItem(SerializationId::Create("item"));
                    SerializeValue(key.ToValue(), serializer);
                    serializer.EndItem();
                });
                serializer.EndArray();
                break;
            }
            case TypeKind::Path:
                serializer.WriteString(value.Cast<Path>()->View(), SerializationId::Create("path"));
                break;
            case TypeKind::StringView:
                serializer.WriteString(*value.Cast<StringView>(), SerializationId::Create("view"));
                break;
            case TypeKind::StringAtom:
                serializer.WriteString(*value.Cast<StringAtom>(), SerializationId::Create("atom"));
                break;
            case TypeKind::UnicodeString:
                serializer.WriteString(*value.Cast<UnicodeString>(), SerializationId::Create("unicode"));
                break;
            case TypeKind::String:
                serializer.WriteString(*value.Cast<String>(), SerializationId::Create("string"));
                break;
            case TypeKind::MathVector: {
                const auto componentSize = typeInfo.pNext[0] ? typeInfo.pNext[0]->GetScalarSize() : 0;
                if (componentSize == 0) {
                    SR_WARN("SerializeValue() : unknown component type of {}", typeInfo.name);
                    return;
                }
                auto&& pData = static_cast<const uint8_t*>(value.Data());
                for (uint8_t i = 0; i < typeInfo.detailedSize; ++i) {
                    serializer.WriteInt(ReadIntegralBits(pData + i * componentSize, componentSize, false), SERIALIZE_AXIS_ID[i]);
                }
                break;
            }
            case TypeKind::Object:
                if (auto&& pClass = value.GetSRClass()) {
                    pClass->GetMeta()->Save(serializer, *dynamic_cast<Serializable*>(pClass));
                }
                break;
            case TypeKind::Quaternion: {
                auto&& pQuat = value.Cast<SR_MATH_NS::Quaternion>();
                serializer.WriteFloat(pQuat->x, SerializationId::Create("x"));
                serializer.WriteFloat(pQuat->y, SerializationId::Create("y"));
                serializer.WriteFloat(pQuat->z, SerializationId::Create("z"));
                serializer.WriteFloat(pQuat->w, SerializationId::Create("w"));
                break;
            }
            case TypeKind::Color: {
                auto&& pColor = value.Cast<SR_MATH_NS::FColor>();
                serializer.WriteFloat(pColor->r, SerializationId::Create("r"));
                serializer.WriteFloat(pColor->g, SerializationId::Create("g"));
                serializer.WriteFloat(pColor->b, SerializationId::Create("b"));
                serializer.WriteFloat(pColor->a, SerializationId::Create("a"));
                break;
            }
            default:
                SRHalt("SerializeValue() : unsupported reflected type: {}", typeInfo.name);
                return;
        }
    }

    bool DeserializeValue(Value& value, IDeserializer& deserializer) { /// NOLINT(misc-no-recursion)
        auto&& typeInfo = value.GetTypeInfo();

        if (typeInfo.IsArithmetic()) {
            switch (typeInfo.kind) {
                case TypeKind::Bool:
                    deserializer.ReadBool(*value.Cast<bool>(), SerializationId::Create("bool"));
                    break;
                case TypeKind::Float:
                    deserializer.ReadFloat(*value.Cast<float_t>(), SerializationId::Create("float"));
                    break;
                case TypeKind::Double:
                    deserializer.ReadDouble(*value.Cast<double_t>(), SerializationId::Create("double"));
                    break;
                default: {
                    int64_t integral = 0;
                    deserializer.ReadInt(integral, SerializationId::Create("int"));
                    WriteIntegralBits(value.Data(), typeInfo.GetScalarSize(), integral);
                    break;
                }
            }
            return true;
        }

        switch (typeInfo.kind) {
            case TypeKind::Value: {
                auto&& pValue = value.Cast<Value>();
                if (pValue && pValue->IsValid()) {
                    return DeserializeValue(*pValue, deserializer);
                }
                return false;
            }
            case TypeKind::Enum: {
                auto&& pReflector = EnumReflectorManager::Instance().TryGetReflector(typeInfo.name);
                if (!pReflector) {
                    SR_WARN("DeserializeValue() : unknown enum type: {}", typeInfo.name);
                    return false;
                }
                SR_THREAD_LOCAL static String enumStr;
                enumStr.clear();
                deserializer.ReadString(enumStr, SerializationId::Create("enum"));
                int64_t enumValue = 0;
                if (pReflector->GetEnumVariantInternal() == EnumVariant::Flags) {
                    SR_THREAD_LOCAL static Vector<StringView> parts;
                    parts.clear();
                    StringUtils::Instance().SplitView(enumStr, "|", parts);
                    for (const auto& part : parts) {
                        enumValue |= pReflector->FromStringInternal(part).value_or(0);
                    }
                }
                else {
                    enumValue = pReflector->FromStringInternal(enumStr).value_or(0);
                }
                pReflector->WriteEnumValueToPointerInternal(value.Data(), enumValue);
                return true;
            }
            case TypeKind::SharedPtr: {
                if (!typeInfo.vtable.pGetTypeController || !typeInfo.pNext[0]) {
                    SRHalt("DeserializeValue() : SharedPtr type is not resolved: {}", typeInfo.name);
                    return false;
                }
                auto&& pSharedPtr = (SR_HTYPES_NS::SharedPtrBase*)typeInfo.vtable.pGetTypeController(value.GetStorage());
                auto&& pNewObject = Factory::Instance().CreateBase(typeInfo.pNext[0]->name);
                if (!pNewObject) {
                    SRHalt("DeserializeValue() : failed to create object of type: {}", typeInfo.pNext[0]->name);
                    return false;
                }
                auto&& pNewObjectShared = dynamic_cast<SR_HTYPES_NS::SharedPtrBase*>(pNewObject);
                if (!pNewObjectShared) {
                    SRHalt("DeserializeValue() : failed to cast object of type: {} to SharedPtrBase", typeInfo.pNext[0]->name);
                    delete pNewObject;
                    return false;
                }
                pSharedPtr->SetPointerFromBase(pNewObjectShared);
                if (auto&& pSerializable = dynamic_cast<Serializable*>(pNewObjectShared)) {
                    return pSerializable->Load(deserializer);
                }
                return true;
            }
            case TypeKind::Map:
            case TypeKind::FlatHashMap: {
                auto&& pKeyType = typeInfo.GetArgument(0);
                auto&& pMappedType = typeInfo.GetArgument(1);
                if (!pKeyType || !pMappedType) {
                    SR_WARN("DeserializeValue() : map element type is not resolved: {}", typeInfo.name);
                    return false;
                }
                const auto map = value.AsMap();
                map.Clear();
                if (uint64_t size = deserializer.BeginArray(SerializationId::Create("map")); size > 0) {
                    uint64_t index = 0;
                    while (deserializer.BeginItem(SerializationId::Create("item"), index)) {
                        Value key = Value::CreateDefault(pKeyType);
                        if (deserializer.BeginObject(SerializationId::Create("key"))) {
                            DeserializeValue(key, deserializer);
                            deserializer.EndObject();
                        }
                        if (deserializer.BeginObject(SerializationId::Create("value"))) {
                            Value mapped = map.FindOrAdd(key.AsRef()).ToValue();
                            DeserializeValue(mapped, deserializer);
                            deserializer.EndObject();
                        }
                        else {
                            map.FindOrAdd(key.AsRef());
                        }
                        deserializer.EndItem();
                        index++;
                    }
                    deserializer.EndArray();
                }
                return true;
            }
            case TypeKind::Set:
            case TypeKind::FlatHashSet: {
                if (!typeInfo.pNext[0]) {
                    return false;
                }
                const auto set = value.AsMap();
                set.Clear();
                if (uint64_t size = deserializer.BeginArray(SerializationId::Create("set")); size > 0) {
                    uint64_t index = 0;
                    while (deserializer.BeginItem(SerializationId::Create("item"), index)) {
                        Value key = Value::CreateDefault(typeInfo.pNext[0]);
                        DeserializeValue(key, deserializer);
                        set.InsertOrAssign(key.AsRef(), ValueRef());
                        deserializer.EndItem();
                        index++;
                    }
                    deserializer.EndArray();
                }
                return true;
            }
            case TypeKind::Vector: {
                if (!typeInfo.pNext[0]) {
                    return false;
                }
                const auto array = value.AsArray();
                array.Clear();
                if (uint64_t size = deserializer.BeginArray(SerializationId::Create("vector")); size > 0) {
                    array.Reserve(size);
                    uint64_t index = 0;
                    while (deserializer.BeginItem(SerializationId::Create("item"), index)) {
                        /// элемент читается на месте, без временного Value и копирования
                        array.Resize(index + 1);
                        Value item = array.At(index).ToValue();
                        DeserializeValue(item, deserializer);
                        deserializer.EndItem();
                        index++;
                    }
                    deserializer.EndArray();
                }
                return true;
            }
            case TypeKind::Path:
                deserializer.ReadString(*value.Cast<Path>(), SerializationId::Create("path"));
                return true;
            case TypeKind::StringView:
                deserializer.ReadString(*value.Cast<String>(), SerializationId::Create("view"));
                return true;
            case TypeKind::StringAtom:
                deserializer.ReadString(*value.Cast<StringAtom>(), SerializationId::Create("atom"));
                return true;
            case TypeKind::UnicodeString:
                deserializer.ReadString(*value.Cast<UnicodeString>(), SerializationId::Create("unicode"));
                return true;
            case TypeKind::String:
                deserializer.ReadString(*value.Cast<String>(), SerializationId::Create("string"));
                return true;
            case TypeKind::MathVector: {
                const auto componentSize = typeInfo.pNext[0] ? typeInfo.pNext[0]->GetScalarSize() : 0;
                if (componentSize == 0) {
                    SR_WARN("DeserializeValue() : unknown component type of {}", typeInfo.name);
                    return false;
                }
                auto&& pData = static_cast<uint8_t*>(value.Data());
                for (uint8_t i = 0; i < typeInfo.detailedSize; ++i) {
                    int64_t component = 0;
                    deserializer.ReadInt(component, SERIALIZE_AXIS_ID[i]);
                    WriteIntegralBits(pData + i * componentSize, componentSize, component);
                }
                return true;
            }
            case TypeKind::Object:
                if (auto&& pClass = value.GetSRClass()) {
                    pClass->GetMeta()->Load(deserializer, *dynamic_cast<Serializable*>(pClass));
                    return true;
                }
                break;
            case TypeKind::Quaternion: {
                auto&& pQuat = value.Cast<SR_MATH_NS::Quaternion>();
                deserializer.ReadFloat(pQuat->x, SerializationId::Create("x"));
                deserializer.ReadFloat(pQuat->y, SerializationId::Create("y"));
                deserializer.ReadFloat(pQuat->z, SerializationId::Create("z"));
                deserializer.ReadFloat(pQuat->w, SerializationId::Create("w"));
                return true;
            }
            case TypeKind::Color: {
                auto&& pColor = value.Cast<SR_MATH_NS::FColor>();
                deserializer.ReadFloat(pColor->r, SerializationId::Create("r"));
                deserializer.ReadFloat(pColor->g, SerializationId::Create("g"));
                deserializer.ReadFloat(pColor->b, SerializationId::Create("b"));
                deserializer.ReadFloat(pColor->a, SerializationId::Create("a"));
                return true;
            }
            default:
                break;
        }
        SR_WARN("DeserializeValue() : unsupported reflected type: {}", typeInfo.name);
        return false;
    }
}
