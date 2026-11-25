//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_JSON_VALUE_H
#define SR_ENGINE_UTILS_JSON_VALUE_H

#include <Utils/Serialization/Serializable.h>
#include <Utils/Types/Optional.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(JsonValueKind, uint8_t, Null, Boolean, Integer, Number, String, Array, Object);

    class JsonIterator;
    class JsonValueView;

    class SR_COMMON_DLL_API JsonValue : public Serializable {
        SR_CLASS()
    public:
        JsonValue();
        JsonValue(const JsonValue& other);
        JsonValue(JsonValue&& other) noexcept;
        ~JsonValue() override;
        JsonValue& operator=(const JsonValue& other);
        JsonValue& operator=(JsonValue&& other) noexcept;

        void Save(ISerializer& serializer) const override;
        bool Load(IDeserializer& deserializer) override;

        SR_NODISCARD static Optional<JsonValue> ParseString(StringView data);

        /// @method
        bool Parse(StringView json);
        /// @method
        bool LoadResource(StringView path);
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
        SR_NODISCARD JsonValueView View() const;

    private:
        friend class JsonValueView;

        struct Impl;
        Impl& Data();
        SR_NODISCARD const Impl& Data() const;
        alignas(8) uint8_t m_storage[32]{};
        bool m_valid = true;
    };

    /// Non-owning, read-only view. The document must outlive the view and must not
    /// be replaced or mutated while its views are in use. Use Clone() to retain a subtree.
    class SR_COMMON_DLL_API JsonValueView : public SRClass {
        SR_CLASS()
    public:
        JsonValueView() = default;
        explicit JsonValueView(void* pValue) noexcept
            : m_value(pValue)
        { }

        explicit operator bool() const noexcept { return IsValid(); }

        /// @method @evaluate
        SR_NODISCARD bool IsValid() const noexcept { return m_value != nullptr; }
        /// @method @evaluate
        SR_NODISCARD JsonValueKind GetKind() const;
        /// @method @evaluate
        SR_NODISCARD uint32_t GetCount() const;
        /// @method @evaluate
        SR_NODISCARD bool Has(StringView key) const;
        /// @method @evaluate
        SR_NODISCARD JsonValueView Get(StringView key) const;
        /// @method @evaluate
        SR_NODISCARD JsonValueView At(uint32_t index) const;
        /// @method @evaluate
        SR_NODISCARD String GetKey(uint32_t index) const;
        /// @method @evaluate
        SR_NODISCARD bool GetBool() const;
        /// @method @evaluate
        SR_NODISCARD int64_t GetInt() const;
        /// @method @evaluate
        SR_NODISCARD double_t GetNumber() const;
        /// @method @evaluate
        SR_NODISCARD StringView GetString() const noexcept;
        /// @method @evaluate
        SR_NODISCARD String ToJson() const;
        /// @method @evaluate
        SR_NODISCARD JsonValue Clone() const;
        /// @method @evaluate
        SR_NODISCARD bool Equals(const JsonValueView& other) const;

        SR_NODISCARD bool IsArray() const noexcept;
        SR_NODISCARD bool IsNull() const noexcept;
        SR_NODISCARD bool Contains(StringView key) const noexcept;
        JsonValueView operator[](StringView key) const noexcept;
        SR_NODISCARD JsonIterator begin() const noexcept;
        SR_NODISCARD JsonIterator end() const noexcept;

    private:
        friend class JsonValue;
        SR_NODISCARD bool Expect(JsonValueKind kind) const;
        SR_NODISCARD bool EqualsInternal(const JsonValueView& other, uint32_t depth) const;
        void* m_value = nullptr;
    };

    class JsonIterator {
    public:
        JsonIterator() = default;

        explicit JsonIterator(const void* pValue) noexcept;

        JsonIterator& operator++() noexcept;
        JsonValueView operator*() const noexcept;

        bool operator==(const JsonIterator& other) const noexcept {
            return m_current == other.m_current;
        }

        bool operator!=(const JsonIterator& other) const noexcept {
            return m_current != other.m_current;
        }

        SR_NODISCARD StringView key() const noexcept;

    private:
        alignas(8) uint8_t m_storage[40]{};
        void* m_current = nullptr;
        void* m_key = nullptr;
        bool m_isObject = false;
    };
}

#endif // SR_ENGINE_UTILS_JSON_VALUE_H
