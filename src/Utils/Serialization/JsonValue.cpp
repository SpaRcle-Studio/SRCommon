//
// Created by Codex on 15.11.2026.
//

#include <Utils/Serialization/JsonValue.h>
#include <Utils/Serialization/JsonSerialization.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Resources/ResourceManager.h>
#include <Codegen/JsonValue.generated.hpp>

namespace SR_UTILS_NS {
    void JsonValue::Save(ISerializer& serializer) const {
        serializer.WriteString(ToJson(), SerializationId::Create("json"));
    }

    bool JsonValue::Load(IDeserializer& deserializer) {
        String json;
        deserializer.ReadString(json, SerializationId::Create("json"));
        return Parse(json);
    }

    JsonValue::JsonValue() {
        m_node.type = SerializationDataType::Null;
    }

    bool JsonValue::Parse(StringView json) {
        SR_TRACY_ZONE;
        JsonDeserializer deserializer;
        m_valid = deserializer.LoadValueFromStringView(json);
        if (!m_valid) {
            m_node.Clear();
            m_node.type = SerializationDataType::Null;
            SR_ERROR("JsonValue::Parse() : invalid JSON; see parser line/column diagnostics");
            return false;
        }
        m_node = deserializer.GetCurrentNode().DetachAllocator();
        return true;
    }

    bool JsonValue::LoadResource(StringView path) {
        SR_TRACY_ZONE;
        const Path resourcePath(path);
        const auto resolved = ResourceManager::Instance().GetResPath().Concat(resourcePath);
        if (!resolved.IsFile()) {
            SetNull();
            m_valid = false;
            SR_ERROR("JsonValue::LoadResource() : resource not found: {}", path);
            return false;
        }
        String text;
        if (!FileSystem::ReadFile(resolved, text)) {
            SetNull();
            m_valid = false;
            SR_ERROR("JsonValue::LoadResource() : cannot read resource: {}", path);
            return false;
        }
        if (!Parse(text)) {
            SR_ERROR("JsonValue::LoadResource() : invalid JSON resource: {}", path);
            return false;
        }
        return true;
    }

    String JsonValue::ToJson() const {
        SR_TRACY_ZONE;
        JsonSerializer serializer;
        serializer.SetPrettyPrint(false);
        auto node = m_node.DetachAllocator();
        node.id = SerializationId::CreateFromString("value");
        serializer.GetCurrentNode() = std::move(node);
        return serializer.ToJsonValue();
    }

    JsonValue JsonValue::Clone() const {
        JsonValue result;
        result.m_node = m_node.DetachAllocator();
        result.m_valid = m_valid;
        return result;
    }

    JsonValueKind JsonValue::GetKind() const {
        switch (m_node.type) {
            case SerializationDataType::Boolean: return JsonValueKind::Boolean;
            case SerializationDataType::Integer: return JsonValueKind::Integer;
            case SerializationDataType::Floating:
            case SerializationDataType::Double: return JsonValueKind::Number;
            case SerializationDataType::String: return JsonValueKind::String;
            case SerializationDataType::Array: return JsonValueKind::Array;
            case SerializationDataType::Root:
            case SerializationDataType::Item:
            case SerializationDataType::Object: return JsonValueKind::Object;
            default: return JsonValueKind::Null;
        }
    }

    bool JsonValue::Expect(JsonValueKind kind) const {
        if (!m_valid || GetKind() != kind) {
            SR_ERROR("JsonValue::Expect() : expected {}, actual {}, valid={}", static_cast<uint8_t>(kind), static_cast<uint8_t>(GetKind()), m_valid);
            return false;
        }
        return true;
    }

    uint32_t JsonValue::GetCount() const {
        return static_cast<uint32_t>(m_node.children.size());
    }

    bool JsonValue::Has(StringView key) const {
        if (!Expect(JsonValueKind::Object)) {
            return false;
        }
        for (auto&& child : m_node.children) {
            if (child.id.GetNameView() == key) {
                return true;
            }
        }
        return false;
    }

    JsonValue JsonValue::Get(StringView key) const {
        JsonValue result;
        if (Expect(JsonValueKind::Object)) {
            for (auto&& child : m_node.children) {
                if (child.id.GetNameView() == key) {
                    result.m_node = child.DetachAllocator();
                    return result;
                }
            }
        }
        result.m_valid = false;
        SR_ERROR("JsonValue::Get() : missing member {}", key);
        return result;
    }

    JsonValue JsonValue::At(uint32_t index) const {
        JsonValue result;
        if (Expect(JsonValueKind::Array) && index < m_node.children.size()) {
            result.m_node = m_node.children[index].DetachAllocator();
            return result;
        }
        result.m_valid = false;
        SR_ERROR("JsonValue::At() : invalid array index {}", index);
        return result;
    }

    String JsonValue::GetKey(uint32_t index) const {
        if (!Expect(JsonValueKind::Object) || index >= m_node.children.size()) {
            SR_ERROR("JsonValue::GetKey() : invalid object index {}", index);
            return {};
        }
        return String(m_node.children[index].id.GetNameView());
    }

    bool JsonValue::Set(StringView key, const JsonValue& value) {
        if (key.size() >= SerializationId::GetMaxNameLength()) {
            SR_ERROR("JsonValue::Set() : object key exceeds the serialization name limit");
            return false;
        }
        if (!Expect(JsonValueKind::Object) || !value.m_valid) {
            return false;
        }
        auto node = value.m_node.DetachAllocator();
        node.id = SerializationId::CreateFromString(key);
        for (auto&& child : m_node.children) {
            if (child.id.GetNameView() == key) {
                child = std::move(node);
                return true;
            }
        }
        m_node.children.emplace_back(std::move(node));
        return true;
    }

    bool JsonValue::SetAt(uint32_t index, const JsonValue& value) {
        if (!Expect(JsonValueKind::Array) || !value.m_valid || index >= m_node.children.size()) {
            SR_ERROR("JsonValue::SetAt() : invalid array index {}", index);
            return false;
        }
        m_node.children[index] = value.m_node.DetachAllocator();
        return true;
    }

    bool JsonValue::Append(const JsonValue& value) {
        if (!Expect(JsonValueKind::Array) || !value.m_valid) {
            return false;
        }
        m_node.children.emplace_back(value.m_node.DetachAllocator());
        return true;
    }

    void JsonValue::SetNull() {
        m_node.Clear();
        m_node.type = SerializationDataType::Null;
        m_valid = true;
    }

    void JsonValue::MakeObject() {
        SetNull();
        m_node.type = SerializationDataType::Object;
    }

    void JsonValue::MakeArray() {
        SetNull();
        m_node.type = SerializationDataType::Array;
    }

    void JsonValue::SetBool(bool value) {
        SetNull();
        m_node.type = SerializationDataType::Boolean;
        m_node.data.boolean = value;
    }

    void JsonValue::SetInt(int64_t value) {
        SetNull();
        m_node.type = SerializationDataType::Integer;
        m_node.data.integer = value;
    }

    void JsonValue::SetNumber(double_t value) {
        SetNull();
        if (!std::isfinite(value)) {
            m_valid = false;
            SR_ERROR("JsonValue::SetNumber() : non-finite number");
            return;
        }
        m_node.type = SerializationDataType::Double;
        m_node.data.floatingDouble = value;
    }

    void JsonValue::SetString(StringView value) {
        SetNull();
        m_node.type = SerializationDataType::String;
        m_node.string = value;
    }

    bool JsonValue::GetBool() const {
        return Expect(JsonValueKind::Boolean) && m_node.data.boolean;
    }

    int64_t JsonValue::GetInt() const {
        return Expect(JsonValueKind::Integer) ? m_node.data.integer : 0;
    }

    double_t JsonValue::GetNumber() const {
        if (m_valid && GetKind() == JsonValueKind::Integer) {
            return static_cast<double_t>(m_node.data.integer);
        }
        if (!Expect(JsonValueKind::Number)) {
            return 0.0;
        }
        return m_node.type == SerializationDataType::Floating ? m_node.data.floating : m_node.data.floatingDouble;
    }

    String JsonValue::GetString() const {
        return Expect(JsonValueKind::String) ? m_node.string : String();
    }
}
