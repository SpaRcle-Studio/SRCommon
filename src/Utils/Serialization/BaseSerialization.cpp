//
// Created by Monika on 09.03.2025.
//

#include <Utils/Serialization/BaseSerialization.h>
#include <Utils/Localization/Encoding.h>

namespace SR_UTILS_NS {
    IBaseSerialization::IBaseSerialization() {
        m_walker.reserve(64);
        m_stack.reserve(64);

        m_walker.emplace_back(&m_root);
        m_stack.emplace_back(&m_root);
    }

    bool IBaseSerialization::SaveToFileImpl(const SR_UTILS_NS::Path& path) const {
        SR_TRACY_ZONE;

        if (path.empty()) {
            SRHalt("IBaseSerialization::SaveToFileImpl() : empty path!");
            return false;
        }

        if (!path.CreateIfNotExists()) {
            SR_ERROR("IBaseSerialization::SaveToFileImpl() : failed to create path!\n\tPath: " + path.ToString());
            return false;
        }

        std::ofstream file(path.c_str());
        if (!file.is_open()) {
            SR_ERROR("IBaseSerialization::SaveToFileImpl() : failed to open file!\n\tPath: " + path.ToString());
            return false;
        }

        file << ToStringBase();
        file.close();
        return true;
    }

    void IBaseSerialization::WriteNode(const SerializationNode& node) noexcept {
        GetCurrentNode().children.emplace_back(node);
    }

    IBaseSerialization::~IBaseSerialization() {
        SR_TRACY_ZONE;
        m_stack.Destroy();
        m_walker.Destroy();
        m_root.Clear();
    }

    /// ========================================== IBaseSerializer =====================================================

    void IBaseSerializer::WriteString(std::string_view value, const SerializationId& name) {
        SerializationNode node(name, SerializationDataType::String);
        node.string = value;
        GetImpl().GetCurrentNode().children.emplace_back(node);
    }

    void IBaseSerializer::WriteBool(const bool value, const SerializationId& name) {
        SerializationNode node(name, SerializationDataType::Boolean);
        node.data.boolean = value;
        GetImpl().GetCurrentNode().children.emplace_back(node);
    }

    void IBaseSerializer::WriteInt(const int64_t value, const SerializationId& name) {
        SerializationNode node(name, SerializationDataType::Integer);
        node.data.integer = value;
        GetImpl().GetCurrentNode().children.emplace_back(node);
    }

    void IBaseSerializer::WriteDouble(const double_t value, const SerializationId& name) {
        SerializationNode node(name, SerializationDataType::Floating);
        node.data.floating = value;
        GetImpl().GetCurrentNode().children.emplace_back(node);
    }

    void IBaseSerializer::BeginItem(const SerializationId& id) {
        auto&& currentNode = GetImpl().GetCurrentNode();
        if (currentNode.type != SerializationDataType::Array) {
            SRHalt("IBaseSerializer::BeginItem() : invalid node type!");
        }

        SerializationNode node(id, SerializationDataType::Item);
        currentNode.children.emplace_back(node);
        GetImpl().m_stack.emplace_back(&currentNode.children.back());
    }

    void IBaseSerializer::EndItem() {
        SRAssert2(GetImpl().GetCurrentNode().type == SerializationDataType::Item, "IBaseSerializer::EndItem() : invalid node type!");
        GetImpl().m_stack.pop_back();
    }

    void IBaseSerializer::BeginObject(const SerializationId& name) {
        SR_TRACY_ZONE;
        SerializationNode node(name, SerializationDataType::Object);
        auto&& currentNode = GetImpl().GetCurrentNode();
        currentNode.children.emplace_back(node);
        GetImpl().m_stack.emplace_back(&currentNode.children.back());
    }

    void IBaseSerializer::EndObject() {
        SR_TRACY_ZONE;
        SRAssert2(GetImpl().GetCurrentNode().type == SerializationDataType::Object, "IBaseSerializer::EndObject() : invalid node type!");
        SRAssert2(!GetImpl().m_stack.empty(), "IBaseSerializer::EndObject() : invalid stack size!");
        GetImpl().m_stack.pop_back();
    }

    void IBaseSerializer::BeginArray(const uint64_t size, const SerializationId& name) {
        SR_TRACY_ZONE;
        SerializationNode node(name, SerializationDataType::Array);
        node.children.reserve(size);
        auto&& currentNode = GetImpl().GetCurrentNode();
        currentNode.children.emplace_back(node);
        GetImpl().m_stack.emplace_back(&currentNode.children.back());
    }

    void IBaseSerializer::EndArray() {
        SR_TRACY_ZONE;
        SRAssert2(GetImpl().GetCurrentNode().type == SerializationDataType::Array, "IBaseSerializer::EndArray() : invalid node type!");
        SRAssert2(!GetImpl().m_stack.empty(), "IBaseSerializer::EndArray() : invalid stack size!");
        GetImpl().m_stack.pop_back();
    }

    void IBaseSerializer::WriteString(std::u32string_view value, const SerializationId& name) {
        SerializationNode node(name, SerializationDataType::String);
        node.string = SR_UTILS_NS::Localization::UtfToUtf<char, char32_t>(value);
        GetImpl().GetCurrentNode().children.emplace_back(node);
    }

    /// ========================================== IBaseDeserializer ===================================================

    IBaseDeserializer::IBaseDeserializer()
        : Super()
    { }

    bool IBaseDeserializer::IsDefault(const SerializationId& name) const noexcept {
        for (auto&& child : GetImpl().GetWalkNode().children) {
            if (child.id.GetHash() == name.GetHash()) {
                return false;
            }
        }
        return true;
    }

    bool IBaseDeserializer::BeginItem(const SerializationId& id, uint32_t index) {
        auto&& node = GetImpl().GetWalkNode();
        if (node.children.size() <= index) {
            return false;
        }
        if (node.children[index].id.GetHash() != id.GetHash()) {
            return false;
        }
        GetImpl().m_walker.emplace_back(&node.children[index]);
        return true;
    }

    void IBaseDeserializer::EndItem() {
        if (GetImpl().m_walker.empty()) {
            SRHalt("IBaseDeserializer::EndItem() : invalid walker!");
            return;
        }
        if (GetImpl().GetWalkNode().type != SerializationDataType::Item) {
            SRHalt("IBaseDeserializer::EndItem() : node type is not Item!");
            return;
        }
        GetImpl().m_walker.pop_back();
        if (GetImpl().GetWalkNode().type != SerializationDataType::Array) {
            SRHalt("IBaseDeserializer::EndItem() : node type is not Array!");
        }
    }

    bool IBaseDeserializer::BeginObject(const SerializationId& id) {
        auto& node = GetImpl().GetWalkNode();

        for (uint64_t i = 0; i < node.children.size(); ++i) {
            if (node.children[i].id.GetHash() == id.GetHash()) {
                GetImpl().m_walker.emplace_back(&node.children[i]);
                return true;
            }
        }

        return false;
    }

    void IBaseDeserializer::EndObject() {
        if (GetImpl().m_walker.empty()) {
            ReportError("IBaseDeserializer::EndObject() : invalid walker!");
            return;
        }
        if (GetImpl().GetWalkNode().type != SerializationDataType::Object) {
            ReportError("IBaseDeserializer::EndObject() : invalid node type!");
            return;
        }
        GetImpl().m_walker.pop_back();
    }

    uint64_t IBaseDeserializer::BeginArray(const SerializationId& id) {
        auto& node = GetImpl().GetWalkNode();
        for (uint64_t i = 0; i < node.children.size(); ++i) {
            if (node.children[i].id.GetHash() == id.GetHash()) {
                if (node.children[i].children.empty()) {
                    return 0;
                }
                GetImpl().m_walker.emplace_back(&node.children[i]);
                return node.children[i].children.size();
            }
        }
        return 0;
    }

    void IBaseDeserializer::EndArray() {
        if (GetImpl().m_walker.empty()) {
            ReportError("IBaseDeserializer::EndArray() : invalid walker!");
            return;
        }
        if (GetImpl().GetWalkNode().type != SerializationDataType::Array) {
            ReportError("IBaseDeserializer::EndArray() : invalid node type!");
        }
        GetImpl().m_walker.pop_back();
    }
}