//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_JSON_VALUE_H
#define SR_ENGINE_UTILS_JSON_VALUE_H

#include <Utils/Serialization/Serializable.h>
#include <Utils/Serialization/BaseSerialization.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(JsonValueKind, uint8_t, Null, Boolean, Integer, Number, String, Array, Object);

    class SR_COMMON_DLL_API JsonValue : public Serializable {
        SR_CLASS()
    public:
        JsonValue();
        void Save(ISerializer& serializer) const override;
        bool Load(IDeserializer& deserializer) override;
        /// @method
        bool Parse(StringView json);
        /// @method
        bool LoadResource(StringView path);
        /// @method @evaluate
        String ToJson() const;
        /// @method @evaluate
        JsonValue Clone() const;
        /// @method @evaluate
        JsonValueKind GetKind() const;
        /// @method @evaluate
        uint32_t GetCount() const;
        /// @method @evaluate
        bool Has(StringView key) const;
        /// @method @evaluate
        JsonValue Get(StringView key) const;
        /// @method @evaluate
        JsonValue At(uint32_t index) const;
        /// @method @evaluate
        String GetKey(uint32_t index) const;
        /// @method
        bool Set(StringView key, const JsonValue& value);
        /// @method
        bool SetAt(uint32_t index, const JsonValue& value);
        /// @method
        bool Append(const JsonValue& value);
        /// @method
        void MakeObject();
        /// @method
        void MakeArray();
        /// @method
        void SetNull();
        /// @method
        void SetBool(bool value);
        /// @method
        void SetInt(int64_t value);
        /// @method
        void SetNumber(double_t value);
        /// @method
        void SetString(StringView value);
        /// @method @evaluate
        bool GetBool() const;
        /// @method @evaluate
        int64_t GetInt() const;
        /// @method @evaluate
        double_t GetNumber() const;
        /// @method @evaluate
        String GetString() const;
        /// @method @evaluate
        bool IsValid() const noexcept { return m_valid; }

    private:
        bool Expect(JsonValueKind kind) const;
        SerializationNode m_node;
        bool m_valid = true;
    };
}

#endif // SR_ENGINE_UTILS_JSON_VALUE_H
