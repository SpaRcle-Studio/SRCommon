//
// Created by Monika on 17.10.2024.
//

#include <Utils/Common/ToString.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Serialization/SRASerialization.h>

namespace SR_UTILS_NS {
    std::string SRAISerialization::ToStringBase() const noexcept {
        SR_TRACY_ZONE;

        std::string result;

        result += "sra format\n";

        std::function<void(const SerializationNode&, size_t)> serializeNode;

        serializeNode = [this, &result, &serializeNode](const SerializationNode& node, const uint64_t depth) {
            if (IsNeedUseTabs()) {
                result += std::string(depth, '\t');
            }

            switch (node.type) {
                case SerializationDataType::Root:
                    result += SR_UTILS_NS::ToString(depth) + "-r:"s + node.id.GetName() + "\n";
                    break;
                case SerializationDataType::Object:
                    result += SR_UTILS_NS::ToString(depth) + "-o:"s + node.id.GetName() + "\n";
                    break;
                case SerializationDataType::Array:
                    result += SR_UTILS_NS::ToString(depth) + "-a:"s + node.id.GetName() + "\n";
                    break;
                case SerializationDataType::Item:
                    if (node.children.empty() && !IsAllowEmptyElementsInArrayImpl()) {
                        return;
                    }
                    result += SR_UTILS_NS::ToString(depth) + "-k:"s + node.id.GetName() + "\n";
                    if (node.children.empty()) {
                        return;
                    }
                    break;
                case SerializationDataType::Unknown:
                    SRHalt("SRAISerialization::ToString() : unknown type!");
                    return;
                default:
                    result += SR_UTILS_NS::ToString(depth) + "-v:"s + node.id.GetName() + "\n";
                    break;
            }

            switch (node.type) {
                case SerializationDataType::Root:
                    for (auto&& child : node.children) {
                        serializeNode(child, depth + 1);
                    }
                    break;
                case SerializationDataType::String: {
                    if (IsNeedUseTabs()) {
                        result += std::string(depth + 1, '\t');
                    }

                    const uint32_t newLineCount = std::ranges::count(node.string, '\n');

                    if (newLineCount > 0) {
                        result += SR_UTILS_NS::ToString(depth + 1) + "-m:";
                        result += std::to_string(newLineCount + 1) + "\n";
                    }
                    else {
                        result += SR_UTILS_NS::ToString(depth + 1) + "-s:";
                    }

                    result += node.string + "\n";

                    SRAssert2(node.children.empty(), "SerializationDataType::Integer : children is not empty!");
                    break;
                }
                case SerializationDataType::Boolean:
                    if (IsNeedUseTabs()) {
                        result += std::string(depth + 1, '\t');
                    }
                    result += SR_UTILS_NS::ToString(depth + 1) + "-b:";
                    result += node.data.boolean ? "true\n" : "false\n";
                    SRAssert2(node.children.empty(), "SerializationDataType::Integer : children is not empty!");
                    break;
                case SerializationDataType::Integer:
                    if (IsNeedUseTabs()) {
                        result += std::string(depth + 1, '\t');
                    }
                    result += SR_UTILS_NS::ToString(depth + 1) + "-i:";
                    result += std::to_string(node.data.integer) + "\n";
                    SRAssert2(node.children.empty(), "SerializationDataType::Integer : children is not empty!");
                    break;
                case SerializationDataType::Floating:
                    if (IsNeedUseTabs()) {
                        result += std::string(depth + 1, '\t');
                    }
                    result += SR_UTILS_NS::ToString(depth + 1) + "-f:";
                    result += std::to_string(node.data.floating) + "\n";
                    SRAssert2(node.children.empty(), "SerializationDataType::Integer : children is not empty!");
                    break;
                case SerializationDataType::Object:
                    for (auto&& child : node.children) {
                        serializeNode(child, depth + 1);
                    }
                    break;
                case SerializationDataType::Item:
                    for (auto&& child : node.children) {
                        serializeNode(child, depth + 1);
                    }
                    break;
                case SerializationDataType::Array:
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
        m_root.type = SerializationDataType::Root;
    }

    std::unique_ptr<IDeserializer> SRASerializer::CreateDeserializer() const {
        SR_TRACY_ZONE;

        auto&& pDeserializer = std::make_unique<SRADeserializer>();
        pDeserializer->m_root = m_root;
        pDeserializer->SetUseTabs(IsNeedUseTabs());
        pDeserializer->SetDontLoadTags(GetDontSaveTags());

        return pDeserializer;
    }

    /// ========================================== SRADeserializer =====================================================

    bool SRADeserializer::LoadFromFile(const SR_UTILS_NS::Path& path) {
        SR_TRACY_ZONE;

        if (path.empty()) {
            SRHalt("SRADeserializer::LoadFromFile() : empty path!");
            return false;
        }

        if (!path.IsFile()) {
            SR_ERROR("SRADeserializer::LoadFromFile() : path is not a file!\n\tPath: " + path.ToString());
            return false;
        }

        const std::string data = FileSystem::ReadAllText(path.ToStringRef());
        if (data.empty()) {
            SR_ERROR("SRADeserializer::LoadFromFile() : empty data!\n\tPath: " + path.ToString());
            return false;
        }

        return LoadFromString(data);
    }

    bool SRADeserializer::LoadFromString(const std::string& str) {
        SR_TRACY_ZONE;

        const std::vector<std::string_view> lines = SR_UTILS_NS::StringUtils::SplitViewWithEmpty(str, "\n");
        if (lines.empty()) {
            SR_ERROR("SRADeserializer::LoadFromString() : no lines found!");
            return false;
        }

        if (lines[0].find("sra format") == std::string::npos) {
            SR_ERROR("SRADeserializer::LoadFromString() : invalid format!");
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
            const int32_t depth = FastSToI(depthStr);
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
                    node.type = SerializationDataType::Root;
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
                    newNode.type = SerializationDataType::Object;
                    m_stack.emplace_back(node.children.size() - 1);
                    break;
                }
                case 'a': {
                    //UpdateDepth(depth, i + 1);
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.children.emplace_back();
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    newNode.type = SerializationDataType::Array;
                    m_stack.emplace_back(node.children.size() - 1);
                    break;
                }
                case 'k': {
                    //UpdateDepth(depth, i + 1);
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.children.emplace_back();
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    newNode.type = SerializationDataType::Item;
                    m_stack.emplace_back(node.children.size() - 1);
                    break;
                }
                case 's': {
                    auto& node = GetCurrentNode();
                    node.type = SerializationDataType::String;
                    node.string = std::string(line.substr(line.find_first_of(':') + 1));
                    m_stack.pop_back();
                    continue;
                }
                case 'm': {
                    auto& node = GetCurrentNode();
                    node.type = SerializationDataType::String;
                    const uint32_t lineCount = FastSToU(line.substr(line.find_first_of(':') + 1));

                    std::string multiline;

                    if (lineCount == 0) {
                        ReportError("Invalid line count: "s + std::to_string(lineCount));
                    }
                    else {
                        for (int32_t j = 0; j < lineCount; ++j) {
                            ++i;
                            if (i >= lines.size()) {
                                ReportError("Invalid line count: "s + std::to_string(lineCount));
                                break;
                            }
                            multiline += lines[i];
                            if (j < lineCount - 1) {
                                multiline += "\n";
                            }
                        }
                    }
                    node.string = multiline;
                    m_stack.pop_back();
                    continue;
                }
                case 'b': {
                    auto& node = GetCurrentNode();
                    node.type = SerializationDataType::Boolean;
                    node.data.boolean = line.substr(line.find_first_of(':') + 1) == "true";
                    m_stack.pop_back();
                    continue;
                }
                case 'i': {
                    auto& node = GetCurrentNode();
                    node.type = SerializationDataType::Integer;
                    node.data.integer = FastSToL(line.substr(line.find_first_of(':') + 1));
                    m_stack.pop_back();
                    continue;
                }
                case 'f': {
                    auto& node = GetCurrentNode();
                    node.type = SerializationDataType::Floating;
                    node.data.floating = FastSToD(line.substr(line.find_first_of(':') + 1));
                    m_stack.pop_back();
                    continue;
                }
                default:
                    ReportError("Invalid type: "s + type.data());
                    break;
            }
        }

        SRAssert2(m_root.type == SerializationDataType::Root, "SRADeserializer::LoadFromFile() : invalid root type!");

        return true;
    }

    bool SRADeserializer::LoadFromNode(SerializationNode&& node) {
        m_root = std::move(node);
        return true;
    }

    void SRADeserializer::ReportError(const std::string& message) {
        SR_TRACY_ZONE;
        SRHalt("SRADeserializer::ReportError() : {}!", message);
    }
} // namespace SR_UTILS_NS