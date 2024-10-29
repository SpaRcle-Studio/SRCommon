//
// Created by Monika on 17.10.2024.
//

#ifndef SR_COMMON_SERIALIZATION_SRA_SERIALIZATION_H
#define SR_COMMON_SERIALIZATION_SRA_SERIALIZATION_H

#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>

namespace SR_UTILS_NS {
    /// SpaRcle Assert serialization / deserialization

    union SRATrivialDataType {
        bool boolean;
        int64_t integer;
        double_t floating;
    };

    enum class SRASerializationDataType : uint8_t {
        Unknown,
        Root,
        String,
        Boolean,
        Integer,
        Floating,
        Object,
        Item,
        Array
    };

    struct SRANode {
        SRANode() = default;
        SRANode(const SerializationId& id, SRASerializationDataType type)
            : id(id)
            , type(type)
        { }
        SerializationId id;
        SRASerializationDataType type = SRASerializationDataType::Unknown;
        std::string string;
        SRATrivialDataType data = {};
        std::vector<SRANode> children;
    };

    class SRAISerialization {
    public:
        void SetUseTabs(const bool value) noexcept { m_isNeedUseTabs = value; }
        SR_NODISCARD bool IsNeedUseTabs() const noexcept { return m_isNeedUseTabs; }

        SR_NODISCARD std::string ToString() const;

    protected:
        SR_NODISCARD virtual bool IsAllowEmptyElementsInArrayImpl() const noexcept { return true; }
        SR_NODISCARD bool SaveToFileImpl(const SR_UTILS_NS::Path& path) const;
        SR_NODISCARD SRANode& GetCurrentNode() noexcept { return GetNode(m_stack); }
        SR_NODISCARD SRANode& GetWalkNode() noexcept { return GetNode(m_walker); }
        SR_NODISCARD SRANode& GetNode(const std::vector<uint64_t>& stack) noexcept;

    protected:
        std::vector<uint64_t> m_stack;
        std::vector<uint64_t> m_walker;
        SRANode m_root;
        bool m_isNeedUseTabs = false;

    };

    class SRASerializer : public ISerializer, public SRAISerialization {
    public:
        SRASerializer();

    public:
        SR_NODISCARD bool SaveToFile(const SR_UTILS_NS::Path& path) const override { return SaveToFileImpl(path); }

        void WriteString(std::string_view value, const SerializationId& name) override;
        void WriteBool(bool value, const SerializationId& name) override;
        void WriteInt(int8_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteInt(int16_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteInt(int32_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteInt(int64_t value, const SerializationId& name) override;
        void WriteUInt(uint8_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteUInt(uint16_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteUInt(uint32_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteUInt(uint64_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteFloat(float_t value, const SerializationId& name) override { WriteDouble(static_cast<double_t>(value), name); }
        void WriteDouble(double_t value, const SerializationId& name) override;

        void BeginItem(const SerializationId& id) override;
        void EndItem() override;

        void BeginObject(const SerializationId& id) override;
        void EndObject() override;

        void BeginArray(uint64_t size, const SerializationId& id) override;
        void EndArray() override;

    private:
        bool IsAllowEmptyElementsInArrayImpl() const noexcept override { return IsAllowEmptyElementsInArray(); }

    };

    class SRADeserializer : public IDeserializer, public SRAISerialization {
    public:
        SR_NODISCARD bool SaveToFile(const SR_UTILS_NS::Path& path) const override { return SaveToFileImpl(path); }
        SR_NODISCARD bool LoadFromFile(const SR_UTILS_NS::Path& path) override;

        SR_NODISCARD bool IsDefault(const SerializationId& name) const noexcept override { return false; }
        SR_NODISCARD bool ShouldSetDefaults(const SerializationId& name) const noexcept override { return true; }
        SR_NODISCARD bool ShouldSetDefaults() const noexcept override { return true; }
        SR_NODISCARD bool AllowNewMapKeys() const noexcept override { return false; }
        SR_NODISCARD bool IsPreserveMode() const noexcept override { return false; }
        SR_NODISCARD bool AllowReAllocPointer(ReAllocPointerReason reason) const noexcept override { return false; }

        bool BeginItem(const SerializationId& id, uint32_t index) override;
        void EndItem() override;

        bool BeginObject(const SerializationId& id) override;
        void EndObject() override;

        uint64_t BeginArray(const SerializationId& id) override;
        void EndArray() override;

        void ReadString(std::string& value, const SerializationId& name) override { return ReadStringImpl(value, name); }
        void ReadString(SR_UTILS_NS::StringAtom& value, const SerializationId& name) override { return ReadStringImpl(value, name); }

        void ReadBool(bool& value, const SerializationId& name) override {
            auto&& node = GetWalkNode();
            for (auto&& child : node.children) {
                if (child.id.GetHash() == name.GetHash()) {
                    if (child.type == SRASerializationDataType::Boolean) {
                        value = child.data.boolean;
                    }
                    break;
                }
            }
        }

        void ReadInt(int8_t& value, const SerializationId& name) override { return ReadIntegerImpl(value, name); }
        void ReadInt(int16_t& value, const SerializationId& name) override { return ReadIntegerImpl(value, name); }
        void ReadInt(int32_t& value, const SerializationId& name) override { return ReadIntegerImpl(value, name); }
        void ReadInt(int64_t& value, const SerializationId& name) override { return ReadIntegerImpl(value, name); }
        void ReadUInt(uint8_t& value, const SerializationId& name) override { return ReadIntegerImpl(value, name); }
        void ReadUInt(uint16_t& value, const SerializationId& name) override { return ReadIntegerImpl(value, name); }
        void ReadUInt(uint32_t& value, const SerializationId& name) override { return ReadIntegerImpl(value, name); }
        void ReadUInt(uint64_t& value, const SerializationId& name) override { return ReadIntegerImpl(value, name); }

        void ReadFloat(float_t& value, const SerializationId& name) override { return ReadFloatingImpl(value, name); }
        void ReadDouble(double_t& value, const SerializationId& name) override { return ReadFloatingImpl(value, name); }

        void ReportError(const std::string& message) override;

    private:
        void UpdateDepth(int32_t depth, int32_t line);

        template<typename T> void ReadIntegerImpl(T& value, const SerializationId& name) {
            auto&& node = GetWalkNode();
            for (auto&& child : node.children) {
                if (child.id.GetHash() == name.GetHash()) {
                    if (child.type == SRASerializationDataType::Integer) {
                        value = static_cast<T>(child.data.integer);
                    }
                    break;
                }
            }
        }

        template<typename T> void ReadFloatingImpl(T& value, const SerializationId& name) {
            auto&& node = GetWalkNode();
            for (auto&& child : node.children) {
                if (child.id.GetHash() == name.GetHash()) {
                    if (child.type == SRASerializationDataType::Floating) {
                        value = static_cast<T>(child.data.floating);
                    }
                    break;
                }
            }
        }

        template<typename T> void ReadStringImpl(T& value, const SerializationId& name) {
            auto&& node = GetWalkNode();
            for (auto&& child : node.children) {
                if (child.id.GetHash() == name.GetHash()) {
                    if (child.type == SRASerializationDataType::String) {
                        value = child.string;
                    }
                    break;
                }
            }
        }

    private:
        struct ArrayInfo {
            SRANode* pNode = nullptr;
            enum class State : uint8_t {
                None,
                Reading,
                End
            } state = State::None;
        };
        std::vector<ArrayInfo> m_arrayStack;

    };
}

#endif //SR_COMMON_SERIALIZATION_SRA_SERIALIZATION_H
