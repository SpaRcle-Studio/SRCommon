//
// Created by Monika on 09.03.2025.
//

#ifndef SR_ENGINE_BASE_SERIALIZATION_H
#define SR_ENGINE_BASE_SERIALIZATION_H

#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>
#include <Utils/Localization/Encoding.h>
#include <Utils/Types/FastMemoryArray.h>

namespace SR_UTILS_NS {
    union SerializationTrivialDataType {
        bool boolean;
        int64_t integer;
        double_t floatingDouble;
        float_t floating;
    };

    enum class SerializationDataType : uint8_t {
        Unknown,
        Root,
        Object,
        Item,
        Array,
        String,
        Boolean,
        Integer,
        Floating,
        Double,
    };

    struct SerializationNode {
        SerializationNode() = default;
        SerializationNode(const SerializationId& id, SerializationDataType type)
            : id(id)
            , type(type)
        { }

        SerializationNode(SerializationNode&& other) noexcept
            : id(other.id)
            , type(other.type)
            , string(std::move(other.string))
            , data(other.data)
            , children(std::move(other.children))
        { }

        SerializationNode(const SerializationNode& other)
            : id(other.id)
            , type(other.type)
            , string(other.string)
            , data(other.data)
            , children(other.children)
        { }

        SerializationNode& operator=(SerializationNode&& other) noexcept {
            id = other.id;
            type = other.type;
            string = std::move(other.string);
            data = other.data;
            children = std::move(other.children);
            return *this;
        }

        SerializationNode& operator=(const SerializationNode& other) {
            id = other.id;
            type = other.type;
            string = other.string;
            data = other.data;
            children = other.children;
            return *this;
        }

        void Clear() noexcept {
            id = SerializationId();
            type = SerializationDataType::Unknown;
            string.clear();
            data = {};
            children.clear();
        }

        SR_NODISCARD SerializationNode DetachAllocator() const noexcept;

        SR_NODISCARD SerializationNode& AddChild(IAllocator* pAllocator);

        SerializationId id;
        SerializationDataType type = SerializationDataType::Unknown;
        String string;
        SerializationTrivialDataType data = {};
        Vector<SerializationNode> children;

    private:
        void DetachAllocatorImpl() noexcept;

    };

    class IBaseSerializer;
    class IBaseDeserializer;

    class IBaseSerialization {
        friend IBaseSerializer;
        friend IBaseDeserializer;
    public:
        IBaseSerialization();
        ~IBaseSerialization();

        SR_NODISCARD virtual String ToStringBase() const noexcept = 0;

        SR_NODISCARD SerializationNode& GetCurrentNode() noexcept { return *m_stack.back(); }
        SR_NODISCARD const SerializationNode& GetCurrentNode() const noexcept { return *m_stack.back(); }
        SR_NODISCARD SerializationNode& GetWalkNode() noexcept { return *m_walker.back(); }
        SR_NODISCARD const SerializationNode& GetWalkNode() const noexcept { return *m_walker.back(); }

        SR_NODISCARD IAllocator* GetStringsPool() const noexcept { return m_stringsPool.Get(); }
        SR_NODISCARD IAllocator* GetNodesPool() const noexcept { return m_nodesPool.Get(); }

        void WriteNode(const SerializationNode& node) noexcept;

    protected:
        SR_NODISCARD virtual bool IsAllowEmptyElementsInArrayImpl() const noexcept { return true; }
        SR_NODISCARD bool SaveToFileImpl(const SR_UTILS_NS::Path& path) const;

    protected:
        SerializationNode m_root;
        SR_HTYPES_NS::FastMemoryArray<SerializationNode*> m_stack;
        SR_HTYPES_NS::FastMemoryArray<SerializationNode*> m_walker;
        SR_HTYPES_NS::RawPointerHolder<IAllocator> m_stringsPool;
        SR_HTYPES_NS::RawPointerHolder<IAllocator> m_nodesPool;

    };

    class IBaseSerializer : public ISerializer {
    public:
        SR_NODISCARD bool SaveToFile(const SR_UTILS_NS::Path& path) const override { return GetImpl().SaveToFileImpl(path); }
        SR_NODISCARD std::string ToString() const noexcept override { return GetImpl().ToStringBase(); }

        void WriteString(std::string_view value, const SerializationId& name) override;
        void WriteString(std::u32string_view value, const SerializationId& name) override;
        void WriteBool(bool value, const SerializationId& name) override;
        void WriteInt(int8_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteInt(int16_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteInt(int32_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteInt(int64_t value, const SerializationId& name) override;
        void WriteUInt(uint8_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteUInt(uint16_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteUInt(uint32_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteUInt(uint64_t value, const SerializationId& name) override { WriteInt(static_cast<int64_t>(value), name); }
        void WriteFloat(float_t value, const SerializationId& name) override;
        void WriteDouble(double_t value, const SerializationId& name) override;

        void BeginItem(const SerializationId& id) override;
        void EndItem() override;

        void BeginObject(const SerializationId& id) override;
        void EndObject() override;

        void BeginArray(uint64_t size, const SerializationId& id) override;
        void EndArray() override;

    private:
        SR_NODISCARD IBaseSerialization& GetImpl() const noexcept {
            if (!m_pBaseSerialization) SR_UNLIKELY_ATTRIBUTE {
                m_pBaseSerialization = dynamic_cast<IBaseSerialization*>(const_cast<IBaseSerializer*>(this));
                SRAssert2(m_pBaseSerialization, "Failed to cast IBaseSerializer to IBaseSerialization");
            }
            return *m_pBaseSerialization;
        }

    private:
        mutable IBaseSerialization* m_pBaseSerialization = nullptr;

    };

    class IBaseDeserializer : public IDeserializer {
        using Super = IDeserializer;
    public:
        IBaseDeserializer();

        SR_NODISCARD bool SaveToFile(const SR_UTILS_NS::Path& path) const override { return GetImpl().SaveToFileImpl(path); }

        SR_NODISCARD bool IsDefault(const SerializationId& name) const noexcept override;

        void ResetWalker() override { GetImpl().m_walker.resize(1); }

        bool BeginItem(const SerializationId& id, uint32_t index) override;
        void EndItem() override;

        bool BeginObject(const SerializationId& id) override;
        void EndObject() override;

        uint64_t BeginArray(const SerializationId& id) override;
        void EndArray() override;

        void ReadString(std::string& value, const SerializationId& name) override { return ReadStringImpl(value, name); }
        void ReadString(String& value, const SerializationId& name) override { return ReadStringImpl(value, name); }
        void ReadString(SR_UTILS_NS::StringAtom& value, const SerializationId& name) override { return ReadStringImpl(value, name); }
        void ReadString(SR_UTILS_NS::Path& value, const SerializationId& name) override { return ReadStringImpl(value, name); }

        void ReadString(UnicodeString& value, const SerializationId& name) override;

        void ReadBool(bool& value, const SerializationId& name) override {
            auto&& node = GetImpl().GetWalkNode();
            for (auto&& child : node.children) {
                if (child.id.GetHash() == name.GetHash()) {
                    if (child.type == SerializationDataType::Boolean) {
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
        void ReadDouble(double_t& value, const SerializationId& name) override { return ReadDoubleImpl(value, name); }

    private:
        template<typename T> void ReadIntegerImpl(T& value, const SerializationId& name) {
            auto&& node = GetImpl().GetWalkNode();
            for (auto&& child : node.children) {
                if (child.id.GetHash() == name.GetHash()) {
                    if (child.type == SerializationDataType::Integer) {
                        value = static_cast<T>(child.data.integer);
                    }
                    break;
                }
            }
        }

        template<typename T> void ReadFloatingImpl(T& value, const SerializationId& name) {
            auto&& node = GetImpl().GetWalkNode();
            for (auto&& child : node.children) {
                if (child.id.GetHash() == name.GetHash()) {
                    if (child.type == SerializationDataType::Floating) {
                        value = static_cast<T>(child.data.floating);
                    }
                    if (child.type == SerializationDataType::Double) {
                        value = static_cast<T>(child.data.floatingDouble);
                    }
                    break;
                }
            }
        }

        template<typename T> void ReadDoubleImpl(T& value, const SerializationId& name) {
            auto&& node = GetImpl().GetWalkNode();
            for (auto&& child : node.children) {
                if (child.id.GetHash() == name.GetHash()) {
                    if (child.type == SerializationDataType::Double) {
                        value = static_cast<T>(child.data.floatingDouble);
                    }
                    if (child.type == SerializationDataType::Floating) {
                        value = static_cast<T>(child.data.floating);
                    }
                    break;
                }
            }
        }

        template<typename T> void ReadStringImpl(T& value, const SerializationId& name) {
            auto&& node = GetImpl().GetWalkNode();
            for (auto&& child : node.children) {
                if (child.id.GetHash() == name.GetHash()) {
                    if (child.type == SerializationDataType::String) {
                        if (child.string.HasAllocator()) {
                            value = child.string.DetachAllocator();
                        }
                        else {
                            value = child.string;
                        }
                    }
                    break;
                }
            }
        }

    private:
        SR_NODISCARD IBaseSerialization& GetImpl() const noexcept {
            if (!m_pBaseSerialization) SR_UNLIKELY_ATTRIBUTE {
                m_pBaseSerialization = dynamic_cast<IBaseSerialization*>(const_cast<IBaseDeserializer*>(this));
                SRAssert2(m_pBaseSerialization, "Failed to cast IBaseDeserializer to IBaseSerialization");
            }
            return *m_pBaseSerialization;
        }

    private:
        mutable IBaseSerialization* m_pBaseSerialization = nullptr;

    };
}

#endif //SR_ENGINE_BASE_SERIALIZATION_H
