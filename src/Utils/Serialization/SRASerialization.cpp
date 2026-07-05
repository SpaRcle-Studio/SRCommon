//
// Created by Monika on 17.10.2024.
//

#include <Utils/Common/ToString.h>
#include <Utils/Common/Features.h>
#include <Utils/Common/LexicalCast.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Serialization/SRASerialization.h>

namespace SR_UTILS_NS {
    String SRAISerialization::ToStringBase() const noexcept {
        SR_TRACY_ZONE;

        String result;
        {
            SR_TRACY_ZONE_N("Analyze and reserve");
            result.reserve(AnalyzeByteSize(m_root, 0));
            SR_TRACY_ZONE_VALUE(result.capacity());
        }

        result += "sra format\n";

        if (IsNeedUseTabs()) {
            result += "use tabs: true\n";
        }
        else {
            result += "use tabs: false\n";
        }

        SerializeNode(result, m_root, 0);

        return result;
    }

    void SRAISerialization::SerializeNode(String& result, const SerializationNode& node, const uint64_t depth) const {
        result += GenerateTabs(depth);

        static constexpr StringView prefixes[] = {
            /* Unknown  */ "",
            /* Root     */ "-r:",
            /* Object   */ "-o:",
            /* Item     */ "-k:",
            /* Array     */ "-a:",
            /* String   */ "-s:",
            /* Boolean  */ "-b:",
            /* Integer  */ "-i:",
            /* Floating */ "-f:",
            /* Double   */ "-d:",
        };

        static constexpr StringView basePrefixes[] = {
            /* Unknown  */ "",
            /* Root     */ "-r:",
            /* Object   */ "-o:",
            /* Item     */ "-k:",
            /* Array     */ "-a:",
            /* String   */ "-v:",
            /* Boolean  */ "-v:",
            /* Integer  */ "-v:",
            /* Floating */ "-v:",
            /* Double   */ "-v:",
        };

        switch (node.type) {
            case SerializationDataType::Item:
                if (node.children.empty() && !IsAllowEmptyElementsInArrayImpl()) {
                    return;
                }
                result += StringFromSmallInt(depth);
                result += basePrefixes[static_cast<uint8_t>(node.type)];
                result += node.id.GetNameView();
                result += "\n";
                if (node.children.empty()) {
                    return;
                }
                break;
            default:
                result += StringFromSmallInt(depth);
                result += basePrefixes[static_cast<uint8_t>(node.type)];
                result += node.id.GetNameView();
                result += "\n";
                break;
            case SerializationDataType::Unknown:
                SRHalt("SRAISerialization::ToString() : unknown type!");
                return;
        }

        if (node.type >= SerializationDataType::Root && node.type <= SerializationDataType::Array) {
            for (auto&& child : node.children) {
                SerializeNode(result, child, depth + 1);
            }
            return;
        }


        if (node.type >= SerializationDataType::String && node.type <= SerializationDataType::Double) {
            if (!node.children.empty()) {
                SRHalt("SRAISerialization::ToString() : node has children!");
            }
        }
        else {
            return;
        }

        result += GenerateTabs(depth + 1);

        if (node.type == SerializationDataType::String) {
            if (const uint32_t newLineCount = std::ranges::count(node.string, '\n'); newLineCount > 0) {
                char buffer[64];
                result += StringFromSmallInt(depth + 1);
                result += "-m:";
                result += SerializeInt(newLineCount + 1, buffer, sizeof(buffer));
                result += "\n";
            }
            else {
                result += StringFromSmallInt(depth + 1);
                result += prefixes[static_cast<uint8_t>(node.type)];
            }

            result += node.string;
            result += "\n";
            return;
        }

        result += StringFromSmallInt(depth + 1);
        result += prefixes[static_cast<uint8_t>(node.type)];

        if (node.type == SerializationDataType::Boolean) {
            result += node.data.boolean ? "true\n" : "false\n";
            return;
        }

        char buffer[64];
        switch (node.type) {
            case SerializationDataType::Integer:
                result += SerializeInt(node.data.integer, buffer, sizeof(buffer));
                break;
            case SerializationDataType::Double:
                result += SerializeDouble(node.data.floating, buffer, sizeof(buffer));
                break;
            case SerializationDataType::Floating:
                result += SerializeFloat(node.data.floating, buffer, sizeof(buffer));
                break;
            default:
                SRHalt("SRAISerialization::ToString() : unknown type!");
                break;
        }

        result += "\n";
    }

    StringView SRAISerialization::GenerateTabs(uint64_t depth) const {
        if (!IsNeedUseTabs()) {
            return {};
        }

        static SR_THREAD_LOCAL String tabs;
        tabs.resize(SR_MAX(128, SR_MAX(tabs.size(), depth)), '\t');
        return StringView(tabs.data(), depth);
    }

    uint64_t SRAISerialization::AnalyzeByteSize(const SerializationNode& node, uint64_t depth) const {
        uint64_t size = 128;

        switch (node.type) {
            case SerializationDataType::Root:
            case SerializationDataType::Object:
            case SerializationDataType::Array:
            case SerializationDataType::Item:
                size += node.id.GetSize() + 4 + (m_isNeedUseTabs ? depth : 0);
                break;
            case SerializationDataType::String:
                size += node.id.GetSize() + 4 + (m_isNeedUseTabs ? depth : 0);
                size += 4 + (m_isNeedUseTabs ? (depth + 1) : 0);
                size += node.string.size();
                break;
            default:
                size += node.id.GetSize() + 4 + (m_isNeedUseTabs ? depth : 0);
                size += 4 + (m_isNeedUseTabs ? (depth + 1) : 0);
                size += 10; /// среднее количество символов для числа
                break;
        }

        for (auto&& child : node.children) {
            size += AnalyzeByteSize(child, depth + 1);
        }

        return size;
    }

    StringView SRAISerialization::StringFromSmallInt(uint64_t value) {
        if (value >= 1000) {
            SRHaltTerminate("Value is too big! Depth: {}. Limits is reached... Crash.", value);
        }
        struct SmallString {
            char buffer[4] = { 0 };
            uint8_t size = 0;
        };
        static SR_THREAD_LOCAL Vector<SmallString> strings; /// maximum 5 kilobytes of memory for 1000 strings

        if ((value + 1) > strings.size()) {
            SR_TRACY_ZONE;
            strings.reserve(128);
            const int64_t oldSize = strings.size();
            strings.resize(value + 1);
            for (int64_t i = oldSize; i <= value; ++i) {
                strings[i].size = SerializeInt(i, strings[i].buffer, sizeof(strings[i].buffer)).size();
            }
        }
        return StringView(strings[value].buffer, strings[value].size);
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
            SR_ERROR("SRADeserializer::LoadFromFile() : path is not a file!\n\tPath: {}", path);
            return false;
        }

        String buffer;
        std::vector<std::string_view> lines = FileSystem::ReadAllTextAsStringViewVector(path, buffer);
        if (lines.empty()) {
            SR_ERROR("SRADeserializer::LoadFromFile() : empty data!\n\tPath: {}", path);
            return false;
        }

        return LoadFromStringsBuffer(lines);
    }

    bool SRADeserializer::LoadFromString(const std::string& str) {
        const std::vector<std::string_view> lines = SR_UTILS_NS::StringUtils::SplitViewWithEmpty(str, "\n");
        return LoadFromStringsBuffer(lines);
    }

    bool SRADeserializer::LoadFromStringsBuffer(const std::vector<std::string_view>& lines) {
        SR_TRACY_ZONE;

        if (lines.empty()) {
            SR_ERROR("SRADeserializer::LoadFromString() : no lines found!");
            return false;
        }

        if (lines[0].find("sra format") == std::string::npos) {
            SR_ERROR("SRADeserializer::LoadFromString() : invalid format!");
            return false;
        }

        static const SR_UTILS_NS::StringAtom RESTORE_MODE_TAG = "SRARestoreMode";
        bool restoreMode = SR_UTILS_NS::Features::Instance().Enabled(RESTORE_MODE_TAG, false);

        for (int32_t i = 1; i < lines.size(); ++i) {
            std::string_view line = lines[i];

            if (i == 1 && line.find("use tabs:") != std::string::npos) {
                const std::string_view useTabs = line.substr(line.find_first_of(':') + 1);
                restoreMode &= (useTabs == " true" || useTabs == "true");
                continue;
            }

            int32_t depth = 0;

            if (restoreMode) {
                /// подсчитаем количество табов в начале строки
                for (const char c : line) {
                    if (c == '\t') {
                        ++depth;
                    }
                    else {
                        break;
                    }
                }
            }

            if (!line.empty() && line[0] == '\t') {
                if (const size_t pos = line.find_first_not_of('\t'); pos != std::string::npos) {
                    line = line.substr(pos);
                }
            }

            if (line.empty()) {
                continue;
            }

            if (!restoreMode) {
                const std::string_view depthStr = line.substr(0, line.find_first_of('-'));
                depth = FastSToI(depthStr);
            }

            if (depth == 0) {
                if (m_stack.size() > 1) {
                    ReportError("Double root on line: "s + std::to_string(i + 1));
                    continue;
                }
            }

            const std::string_view& type = line.substr(line.find_first_of('-') + 1, 1);
            if (type.size() != 1) {
                ReportError("Type not found on line: "s + std::to_string(i + 1));
                continue;
            }

            while (m_stack.size() > static_cast<size_t>(depth) && m_stack.size() > 1) {
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
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.AddChild(GetNodesPool());
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    m_stack.emplace_back(&newNode);
                    continue;
                }
                case 'o': {
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.AddChild(GetNodesPool());
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    newNode.type = SerializationDataType::Object;
                    m_stack.emplace_back(&newNode);
                    break;
                }
                case 'a': {
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.AddChild(GetNodesPool());
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    newNode.type = SerializationDataType::Array;
                    m_stack.emplace_back(&newNode);
                    break;
                }
                case 'k': {
                    auto& node = GetCurrentNode();
                    auto&& newNode = node.AddChild(GetNodesPool());
                    newNode.id = SerializationId::CreateFromString(line.substr(line.find_first_of(':') + 1));
                    newNode.type = SerializationDataType::Item;
                    m_stack.emplace_back(&newNode);
                    break;
                }
                case 's': {
                    auto& node = GetCurrentNode();
                    node.type = SerializationDataType::String;
                    StringView data = line.substr(line.find_first_of(':') + 1);
                    node.string = String(data, GetStringsPool());
                    m_stack.pop_back();
                    continue;
                }
                case 'm': {
                    auto& node = GetCurrentNode();
                    node.type = SerializationDataType::String;
                    const uint32_t lineCount = FastSToU(line.substr(line.find_first_of(':') + 1));

                    static SR_THREAD_LOCAL String multiline;
                    multiline.clear();

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
                    node.string = String(multiline, GetStringsPool());
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
                    node.data.floating = ParseFloat(line.substr(line.find_first_of(':') + 1));
                    m_stack.pop_back();
                    continue;
                }
                case 'd': {
                    auto& node = GetCurrentNode();
                    node.type = SerializationDataType::Double;
                    node.data.floatingDouble = ParseDouble(line.substr(line.find_first_of(':') + 1));
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