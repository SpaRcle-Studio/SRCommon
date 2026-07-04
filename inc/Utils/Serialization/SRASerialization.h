//
// Created by Monika on 17.10.2024.
//

#ifndef SR_COMMON_SERIALIZATION_SRA_SERIALIZATION_H
#define SR_COMMON_SERIALIZATION_SRA_SERIALIZATION_H

#include <Utils/Serialization/BaseSerialization.h>

namespace SR_UTILS_NS {
    /// SpaRcle Assert serialization / deserialization

    class SRAISerialization : public IBaseSerialization {
    public:
        void SetUseTabs(const bool value) noexcept { m_isNeedUseTabs = value; }
        SR_NODISCARD bool IsNeedUseTabs() const noexcept { return m_isNeedUseTabs; }

        SR_NODISCARD String ToStringBase() const noexcept override;

    private:
        void SerializeNode(String& result, const SerializationNode& node, uint64_t depth) const;
        SR_NODISCARD StringView GenerateTabs(uint64_t depth) const;
        SR_NODISCARD static StringView StringFromSmallInt(uint64_t value);
        SR_NODISCARD uint64_t AnalyzeByteSize(const SerializationNode& node, uint64_t depth) const;

    private:
        bool m_isNeedUseTabs = false;

    };

    class SRADeserializer;

    class SRASerializer : public IBaseSerializer, public SRAISerialization {
    public:
        SRASerializer();

    public:
        SR_NODISCARD std::unique_ptr<IDeserializer> CreateDeserializer() const override;

    private:
        SR_NODISCARD bool IsAllowEmptyElementsInArrayImpl() const noexcept override { return IsAllowEmptyElementsInArray(); }

    };

    class SRADeserializer : public IBaseDeserializer, public SRAISerialization {
        friend SRASerializer;
    public:
        SR_NODISCARD bool LoadFromFile(const SR_UTILS_NS::Path& path) override;
        SR_NODISCARD bool LoadFromString(const std::string& str) override;
        SR_NODISCARD bool LoadFromNode(SR_UTILS_NS::SerializationNode&& node);

        SR_NODISCARD bool ShouldSetDefaults(const SerializationId& /* name */) const noexcept override { return true; }
        SR_NODISCARD bool ShouldSetDefaults() const noexcept override { return true; }
        SR_NODISCARD bool AllowNewMapKeys() const noexcept override { return false; }
        SR_NODISCARD bool IsPreserveMode() const noexcept override { return false; }
        SR_NODISCARD bool AllowReAllocPointer(ReAllocPointerReason /* reason */) const noexcept override { return false; }

        void ReportError(const std::string& message) override;

    private:
        SR_NODISCARD bool LoadFromStringsBuffer(const std::vector<std::string_view>& lines);

    };
}

#endif //SR_COMMON_SERIALIZATION_SRA_SERIALIZATION_H
