//
// Created by Monika on 12.08.2026.
//

#ifndef SR_COMMON_SERIALIZATION_JSON_SERIALIZATION_H
#define SR_COMMON_SERIALIZATION_JSON_SERIALIZATION_H

#include <Utils/Serialization/BaseSerialization.h>

namespace SR_UTILS_NS {
    /// JSON serialization / deserialization
    ///
    /// Дерево узлов отображается в JSON следующим образом:
    ///     Root / Object / Item -> { "id": value, ... }
    ///     Array                -> [ value, ... ]
    ///     String               -> "value"
    ///     Boolean              -> true / false
    ///     Integer              -> 42
    ///     Floating / Double    -> 42.0 (точка добавляется всегда, чтобы отличать числа от целых)
    ///
    /// Корневой узел оборачивается в объект с единственным ключом, чтобы сохранить его имя:
    ///     { "Root": { ... } }
    ///
    /// Элементы массива не имеют имён в JSON, поэтому идентификаторы Item-узлов не сохраняются,
    /// а JsonDeserializer::BeginItem() не сверяет их при чтении.

    class JsonISerialization : public IBaseSerialization {
    public:
        void SetPrettyPrint(const bool value) noexcept { m_isPrettyPrint = value; }
        SR_NODISCARD bool IsPrettyPrint() const noexcept { return m_isPrettyPrint; }

        SR_NODISCARD String ToStringBase() const noexcept override;

    private:
        void SerializeValue(String& result, const SerializationNode& node, uint64_t depth) const;
        void SerializeObject(String& result, const SerializationNode& node, uint64_t depth) const;
        void SerializeArray(String& result, const SerializationNode& node, uint64_t depth) const;
        void SerializeKey(String& result, StringView key) const;
        void NewLine(String& result, uint64_t depth) const;
        static void SerializeString(String& result, StringView value);
        static void SerializeFloatingPoint(String& result, StringView value);
        SR_NODISCARD StringView GenerateTabs(uint64_t depth) const;
        SR_NODISCARD uint64_t AnalyzeByteSize(const SerializationNode& node, uint64_t depth) const;

    private:
        bool m_isPrettyPrint = true;

    };

    class JsonDeserializer;

    class JsonSerializer : public IBaseSerializer, public JsonISerialization {
    public:
        JsonSerializer();

    public:
        SR_NODISCARD std::unique_ptr<IDeserializer> CreateDeserializer() const override;

    private:
        SR_NODISCARD bool IsAllowEmptyElementsInArrayImpl() const noexcept override { return IsAllowEmptyElementsInArray(); }

    };

    class JsonDeserializer : public IBaseDeserializer, public JsonISerialization {
        friend JsonSerializer;
    public:
        SR_NODISCARD bool LoadFromFile(const SR_UTILS_NS::Path& path) override;
        SR_NODISCARD bool LoadFromString(const std::string& str) override;
        SR_NODISCARD bool LoadFromStringView(StringView str);
        SR_NODISCARD bool LoadFromNode(SR_UTILS_NS::SerializationNode&& node);

        SR_NODISCARD bool ShouldSetDefaults(const SerializationId& /* name */) const noexcept override { return true; }
        SR_NODISCARD bool ShouldSetDefaults() const noexcept override { return true; }
        SR_NODISCARD bool AllowNewMapKeys() const noexcept override { return false; }
        SR_NODISCARD bool IsPreserveMode() const noexcept override { return false; }
        SR_NODISCARD bool AllowReAllocPointer(ReAllocPointerReason /* reason */) const noexcept override { return false; }

        /// В JSON элементы массива безымянные, поэтому идентификатор не сверяется, а восстанавливается
        bool BeginItem(const SerializationId& id, uint32_t index) override;

        void ReportError(const std::string& message) override;

    private:
        SR_NODISCARD bool ParseDocument();
        SR_NODISCARD bool ParseValue(SerializationNode& node, uint32_t depth);
        SR_NODISCARD bool ParseObject(SerializationNode& node, uint32_t depth);
        SR_NODISCARD bool ParseArray(SerializationNode& node, uint32_t depth);
        SR_NODISCARD bool ParseStringValue(String& result);
        SR_NODISCARD bool ParseNumber(SerializationNode& node);
        SR_NODISCARD bool ParseLiteral(StringView literal);
        SR_NODISCARD bool ParseUnicodeEscape(String& result);

        SR_NODISCARD bool HasData() const noexcept { return m_position < m_data.size(); }
        SR_NODISCARD char Peek() const noexcept { return HasData() ? m_data[m_position] : '\0'; }
        SR_NODISCARD bool Consume(char symbol);
        void SkipWhitespaces() noexcept;
        void ReportParseError(const std::string& message);

    private:
        static constexpr uint32_t MaxDepth = 512;

        StringView m_data;
        uint64_t m_position = 0;
        bool m_isFailed = false;

    };
}

#endif //SR_COMMON_SERIALIZATION_JSON_SERIALIZATION_H
