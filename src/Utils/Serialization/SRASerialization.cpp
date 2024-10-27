//
// Created by Monika on 17.10.2024.
//

#include <Utils/Common/ToString.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Serialization/SRASerialization.h>

namespace SR_UTILS_NS {
    SRANode& SRAISerialization::GetCurrentNode() noexcept {
        if (m_stack.empty()) {
            return m_root;
        }
        std::vector<SRANode>* current = &m_root.children;
        for (uint64_t i = 0; i < m_stack.size(); ++i) {
            if (current->size() <= m_stack[i]) {
                SR_ERROR("SRAISerialization::GetCurrentNode() : invalid stack!");
                return m_root;
            }
            if (i == m_stack.size() - 1) {
                return (*current)[m_stack[i]];
            }
            current = &current->at(m_stack[i]).children;
        }
        SR_ERROR("SRAISerialization::GetCurrentNode() : invalid stack!");
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
                for (size_t j = 0; j < line.size(); ++j) {
                    if (line[j] != '\t') {
                        line = line.substr(j);
                        break;
                    }
                }
            }

            if (line.empty()) {
                continue;
            }

            const std::string_view depthStr = line.substr(0, line.find_first_of('-'));
            const int32_t depth = LexicalCast<int32_t>(depthStr);
            if (depth == 0) {
                if (!m_stack.empty()) {
                    ReportError(SerializationId::Create("Root"), "Double root on line: "s + std::to_string(i + 1));
                    continue;
                }
            }

            const std::string_view& type = line.substr(line.find_first_of('-') + 1, 1);
            if (type.size() != 1) {
                ReportError(SerializationId::Create("Root"), "Type not found on line: "s + std::to_string(i + 1));
                continue;
            }

            switch (type[0]) {
                case 'r': {
                    auto& node = GetCurrentNode();
                    node.type = SRASerializationDataType::Root;
                    node.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    continue;
                }
                case 'v': {
                    UpdateDepth(depth, i + 1);
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.children.emplace_back();
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    m_stack.emplace_back(node.children.size() - 1);
                    continue;
                }
                case 'o': {
                    UpdateDepth(depth, i + 1);
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.children.emplace_back();
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    newNode.type = SRASerializationDataType::Object;
                    m_stack.emplace_back(node.children.size() - 1);
                    break;
                }
                case 'a': {
                    UpdateDepth(depth, i + 1);
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.children.emplace_back();
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    newNode.type = SRASerializationDataType::Array;
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
                    ReportError(SerializationId::Create("Root"), "Invalid type: "s + type.data());
                    break;
            }
        }

        SRAssert2(m_root.type == SRASerializationDataType::Root, "SRADeserializer::LoadFromFile() : invalid root type!");

        return true;
    }

    void SRADeserializer::UpdateDepth(int32_t depth, int32_t line) {
        if (depth != m_stack.size()) {
            const int32_t delta = depth - static_cast<int32_t>(m_stack.size());

            if (delta < 0 && m_stack.size() < SR_ABS(delta)) {
                SRHalt("SRADeserializer::UpdateDepth() : invalid stack size on line: {}!", line);
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