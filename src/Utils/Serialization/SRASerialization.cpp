//
// Created by Monika on 17.10.2024.
//

#include <Utils/Common/ToString.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Serialization/SRASerialization.h>

namespace SR_UTILS_NS {
    SRANode& SRAISerialization::GetNode(const std::vector<uint64_t>& stack) noexcept {
        if (stack.empty()) {
            return m_root;
        }
        std::vector<SRANode>* current = &m_root.children;
        for (uint64_t i = 0; i < stack.size(); ++i) {
            if (current->size() <= stack[i]) {
                SR_ERROR("SRAISerialization::GetNode() : invalid stack!");
                return m_root;
            }
            if (i == stack.size() - 1) {
                return (*current)[stack[i]];
            }
            current = &current->at(stack[i]).children;
        }
        SR_ERROR("SRAISerialization::GetNode() : invalid stack!");
        return m_root;
    }

    bool SRAISerialization::SaveToFileImpl(const SR_UTILS_NS::Path& path) const {
        if (path.empty()) {
            return false;
        }

        std::ofstream file(path.c_str());
        if (!file.is_open()) {
            return false;
        }

        file << ToString();
        file.close();
        return true;
    }

    std::string SRAISerialization::ToString() const {
        SR_TRACY_ZONE;

        std::string result;

        result += "sra format\n";

        std::function<void(const SRANode&, size_t)> serializeNode;

        serializeNode = [this, &result, &serializeNode](const SRANode& node, const uint64_t depth) {
            if (IsNeedUseTabs()) {
                result += std::string(depth, '\t');
            }

            switch (node.type) {
                case SRASerializationDataType::Root:
                    result += SR_UTILS_NS::ToString(depth) + "-r:"s + node.id.GetName() + "\n";
                    break;
                case SRASerializationDataType::Object:
                    result += SR_UTILS_NS::ToString(depth) + "-o:"s + node.id.GetName() + "\n";
                    break;
                case SRASerializationDataType::Array:
                    result += SR_UTILS_NS::ToString(depth) + "-a:"s + node.id.GetName() + "\n";
                    break;
                case SRASerializationDataType::Item:
                    if (node.children.empty() && !IsAllowEmptyElementsInArrayImpl()) {
                        return;
                    }
                    result += SR_UTILS_NS::ToString(depth) + "-k:"s + node.id.GetName() + "\n";
                    if (node.children.empty()) {
                        return;
                    }
                    break;
                case SRASerializationDataType::Unknown:
                    SRHalt("SRAISerialization::ToString() : unknown type!");
                    return;
                default:
                    result += SR_UTILS_NS::ToString(depth) + "-v:"s + node.id.GetName() + "\n";
                    break;
            }

            switch (node.type) {
                case SRASerializationDataType::Root:
                    for (auto&& child : node.children) {
                        serializeNode(child, depth + 1);
                    }
                    break;
                case SRASerializationDataType::String:
                    if (IsNeedUseTabs()) {
                        result += std::string(depth + 1, '\t');
                    }
                    result += SR_UTILS_NS::ToString(depth + 1) + "-s:";
                    result += node.string + "\n";
                    SRAssert2(node.children.empty(), "SRASerializationDataType::Integer : children is not empty!");
                    break;
                case SRASerializationDataType::Boolean:
                    if (IsNeedUseTabs()) {
                        result += std::string(depth + 1, '\t');
                    }
                    result += SR_UTILS_NS::ToString(depth + 1) + "-b:";
                    result += node.data.boolean ? "true\n" : "false\n";
                    SRAssert2(node.children.empty(), "SRASerializationDataType::Integer : children is not empty!");
                    break;
                case SRASerializationDataType::Integer:
                    if (IsNeedUseTabs()) {
                        result += std::string(depth + 1, '\t');
                    }
                    result += SR_UTILS_NS::ToString(depth + 1) + "-i:";
                    result += std::to_string(node.data.integer) + "\n";
                    SRAssert2(node.children.empty(), "SRASerializationDataType::Integer : children is not empty!");
                    break;
                case SRASerializationDataType::Floating:
                    if (IsNeedUseTabs()) {
                        result += std::string(depth + 1, '\t');
                    }
                    result += SR_UTILS_NS::ToString(depth + 1) + "-f:";
                    result += std::to_string(node.data.floating) + "\n";
                    SRAssert2(node.children.empty(), "SRASerializationDataType::Integer : children is not empty!");
                    break;
                case SRASerializationDataType::Object:
                    for (auto&& child : node.children) {
                        serializeNode(child, depth + 1);
                    }
                    break;
                case SRASerializationDataType::Item:
                    serializeNode(node.children[0], depth + 1);
                    break;
                case SRASerializationDataType::Array:
                    for (auto&& child : node.children) {
                        serializeNode(child, depth + 1);
                    }
                    break;
                default:
                    break;
            }
        };

        serializeNode(m_root, 0);

        return result;
    }

    /// ========================================= SRAISerialization ====================================================

    SRASerializer::SRASerializer() {
        m_root.id = SerializationId::Create("Root");
        m_root.type = SRASerializationDataType::Root;
    }

    void SRASerializer::WriteString(std::string_view value, const SerializationId& name) {
        SRANode node(name, SRASerializationDataType::String);
        node.string = value;
        GetCurrentNode().children.emplace_back(node);
    }

    void SRASerializer::WriteBool(const bool value, const SerializationId& name) {
        SRANode node(name, SRASerializationDataType::Boolean);
        node.data.boolean = value;
        GetCurrentNode().children.emplace_back(node);
    }

    void SRASerializer::WriteInt(const int64_t value, const SerializationId& name) {
        SRANode node(name, SRASerializationDataType::Integer);
        node.data.integer = value;
        GetCurrentNode().children.emplace_back(node);
    }

    void SRASerializer::WriteDouble(const double_t value, const SerializationId& name) {
        SRANode node(name, SRASerializationDataType::Floating);
        node.data.floating = value;
        GetCurrentNode().children.emplace_back(node);
    }

    void SRASerializer::BeginItem(const SerializationId& id) {
        auto&& currentNode = GetCurrentNode();
        if (currentNode.type != SRASerializationDataType::Array) {
            SRHalt("SRASerializer::BeginItem() : invalid node type!");
        }

        SRANode node(id, SRASerializationDataType::Item);
        currentNode.children.emplace_back(node);
        m_stack.emplace_back(GetCurrentNode().children.size() - 1);
    }

    void SRASerializer::EndItem() {
        SRAssert2(GetCurrentNode().type == SRASerializationDataType::Item, "SRASerializer::EndItem() : invalid node type!");
        m_stack.pop_back();
    }

    void SRASerializer::BeginObject(const SerializationId& name) {
        SRANode node(name, SRASerializationDataType::Object);
        GetCurrentNode().children.emplace_back(node);
        m_stack.emplace_back(GetCurrentNode().children.size() - 1);
    }

    void SRASerializer::EndObject() {
        SRAssert2(GetCurrentNode().type == SRASerializationDataType::Object, "SRASerializer::EndObject() : invalid node type!");
        SRAssert2(!m_stack.empty(), "SRASerializer::EndObject() : invalid stack size!");
        m_stack.pop_back();
    }

    void SRASerializer::BeginArray(const uint64_t size, const SerializationId& name) {
        SRANode node(name, SRASerializationDataType::Array);
        node.children.reserve(size);
        GetCurrentNode().children.emplace_back(node);
        m_stack.emplace_back(GetCurrentNode().children.size() - 1);
    }

    void SRASerializer::EndArray() {
        SRAssert2(GetCurrentNode().type == SRASerializationDataType::Array, "SRASerializer::EndArray() : invalid node type!");
        SRAssert2(!m_stack.empty(), "SRASerializer::EndArray() : invalid stack size!");
        m_stack.pop_back();
    }

    /// ========================================== SRADeserializer =====================================================

    bool SRADeserializer::LoadFromFile(const SR_UTILS_NS::Path& path) {
        if (path.empty()) {
            return false;
        }

        if (!path.IsFile()) {
            return false;
        }

        const std::vector<std::string> lines = FileSystem::ReadAllLines(path);
        if (lines.empty()) {
            return false;
        }

        if (lines[0].find("sra format") == std::string::npos) {
            return false;
        }

        for (int32_t i = 1; i < lines.size(); ++i) {
            std::string_view line = lines[i];

            if (!line.empty() && line[0] == '\t') {
                if (const size_t pos = line.find_first_not_of('\t'); pos != std::string::npos) {
                    line = line.substr(pos);
                }
            }

            if (line.empty()) {
                continue;
            }

            const std::string_view depthStr = line.substr(0, line.find_first_of('-'));
            const int32_t depth = LexicalCast<int32_t>(depthStr);
            if (depth == 0) {
                if (!m_stack.empty()) {
                    ReportError("Double root on line: "s + std::to_string(i + 1));
                    continue;
                }
            }

            const std::string_view& type = line.substr(line.find_first_of('-') + 1, 1);
            if (type.size() != 1) {
                ReportError("Type not found on line: "s + std::to_string(i + 1));
                continue;
            }

            while (m_stack.size() + 1 > static_cast<size_t>(depth) && !m_stack.empty()) {
                m_stack.pop_back();
            }

            switch (type[0]) {
                case 'r': {
                    auto& node = GetCurrentNode();
                    node.type = SRASerializationDataType::Root;
                    node.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    continue;
                }
                case 'v': {
                    //UpdateDepth(depth, i + 1);
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.children.emplace_back();
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    m_stack.emplace_back(node.children.size() - 1);
                    continue;
                }
                case 'o': {
                    //UpdateDepth(depth, i + 1);
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.children.emplace_back();
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    newNode.type = SRASerializationDataType::Object;
                    m_stack.emplace_back(node.children.size() - 1);
                    break;
                }
                case 'a': {
                    //UpdateDepth(depth, i + 1);
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.children.emplace_back();
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    newNode.type = SRASerializationDataType::Array;
                    m_stack.emplace_back(node.children.size() - 1);
                    break;
                }
                case 'k': {
                    //UpdateDepth(depth, i + 1);
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.children.emplace_back();
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    newNode.type = SRASerializationDataType::Item;
                    m_stack.emplace_back(node.children.size() - 1);
                    break;
                }
                case 's': {
                    auto& node = GetCurrentNode();
                    node.type = SRASerializationDataType::String;
                    node.string = std::string(line.substr(line.find_first_of(':') + 1));
                    m_stack.pop_back();
                    continue;
                }
                case 'b': {
                    auto& node = GetCurrentNode();
                    node.type = SRASerializationDataType::Boolean;
                    node.data.boolean = line.substr(line.find_first_of(':') + 1) == "true";
                    m_stack.pop_back();
                    continue;
                }
                case 'i': {
                    auto& node = GetCurrentNode();
                    node.type = SRASerializationDataType::Integer;
                    node.data.integer = LexicalCast<int64_t>(line.substr(line.find_first_of(':') + 1));
                    m_stack.pop_back();
                    continue;
                }
                case 'f': {
                    auto& node = GetCurrentNode();
                    node.type = SRASerializationDataType::Floating;
                    node.data.floating = LexicalCast<double_t>(line.substr(line.find_first_of(':') + 1));
                    m_stack.pop_back();
                    continue;
                }
                default:
                    ReportError("Invalid type: "s + type.data());
                    break;
            }
        }

        SRAssert2(m_root.type == SRASerializationDataType::Root, "SRADeserializer::LoadFromFile() : invalid root type!");

        return true;
    }

    /*bool SRADeserializer::NextItem(const SerializationId& id) noexcept {
        if (m_walker.empty() || m_arrayStack.empty()) {
            SRHalt("SRADeserializer::NextItem() : invalid walker or array stack!");
            return false;
        }

        ArrayInfo& info = m_arrayStack.back();

        if (info.state == ArrayInfo::State::End) {
            return false;
        }

        if (info.state == ArrayInfo::State::None) {
            for (uint64_t i = 0; i < info.pNode->children.size(); ++i) {
                if (info.pNode->children[i].id.GetHash() == id.GetHash()) {
                    m_walker.emplace_back(i);
                    info.state = ArrayInfo::State::Reading;
                    return true;
                }
            }
            info.state = ArrayInfo::State::End;
            return false;
        }

        if (info.state == ArrayInfo::State::Reading) {
            for (uint64_t i = m_walker.back() + 1; i < info.pNode->children.size(); ++i) {
                if (info.pNode->children[i].id.GetHash() == id.GetHash()) {
                    m_walker.back() = i;
                    return true;
                }
            }
            info.state = ArrayInfo::State::End;
            return false;
        }

        SRHalt("SRADeserializer::NextItem() : invalid state!");
        return false;
    }*/

    bool SRADeserializer::BeginItem(const SerializationId& id, uint32_t index) {
        auto&& node = GetWalkNode();
        if (node.children.size() <= index) {
            return false;
        }
        if (node.children[index].id.GetHash() != id.GetHash()) {
            return false;
        }
        m_walker.emplace_back(index);
        return true;
    }

    void SRADeserializer::EndItem() {
        if (m_walker.empty()) {
            SRHalt("SRADeserializer::EndItem() : invalid walker!");
            return;
        }
        if (GetWalkNode().type != SRASerializationDataType::Item) {
            SRHalt("SRADeserializer::EndItem() : node type is not Item!");
            return;
        }
        m_walker.pop_back();
        if (GetWalkNode().type != SRASerializationDataType::Array) {
            SRHalt("SRADeserializer::EndItem() : node type is not Array!");
        }
    }

    bool SRADeserializer::BeginObject(const SerializationId& id) {
        auto& node = GetWalkNode();

        for (uint64_t i = 0; i < node.children.size(); ++i) {
            if (node.children[i].id.GetHash() == id.GetHash()) {
                m_walker.emplace_back(i);
                return true;
            }
        }

        return false;
    }

    void SRADeserializer::EndObject() {
        if (m_walker.empty()) {
            ReportError("SRADeserializer::EndObject() : invalid walker!");
            return;
        }
        if (GetWalkNode().type != SRASerializationDataType::Object) {
            ReportError("SRADeserializer::EndObject() : invalid node type!");
            return;
        }
        m_walker.pop_back();
    }

    uint64_t SRADeserializer::BeginArray(const SerializationId& id) {
        auto& node = GetWalkNode();
        for (uint64_t i = 0; i < node.children.size(); ++i) {
            if (node.children[i].id.GetHash() == id.GetHash()) {
                m_walker.emplace_back(i);
                return node.children[i].children.size();
            }
        }
        return 0;
    }

    void SRADeserializer::EndArray() {
        if (m_walker.empty()) {
            ReportError("SRADeserializer::EndArray() : invalid walker!");
            return;
        }
        if (GetWalkNode().type != SRASerializationDataType::Array) {
            ReportError("SRADeserializer::EndArray() : invalid node type!");
        }
        m_walker.pop_back();
    }

    void SRADeserializer::ReportError(const std::string& message) {
        SRHalt("SRADeserializer::ReportError() : {}!", message);
    }

    void SRADeserializer::UpdateDepth(int32_t depth, int32_t line) {
        if (depth != m_stack.size()) {
            const int32_t delta = depth - static_cast<int32_t>(m_stack.size());

            if (delta < 0 && m_stack.size() < SR_ABS(delta)) {
                ReportError(SR_FORMAT("SRADeserializer::UpdateDepth() : invalid stack size on line: {}!", line));
                return;
            }

            if (delta == -1) {
                m_stack.pop_back();
            }
            else if (delta < 0) {
                for (uint32_t j = 0; j <= -delta; ++j) {
                    m_stack.pop_back();
                }
            }
        }
        else {
            m_stack.pop_back();
        }
    }
} // namespace SR_UTILS_NS