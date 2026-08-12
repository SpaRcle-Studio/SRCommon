//
// Created by Monika on 12.08.2026.
//

#include <Utils/Common/ToString.h>
#include <Utils/Common/LexicalCast.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Serialization/JsonSerialization.h>

namespace SR_UTILS_NS {
    String JsonISerialization::ToStringBase() const noexcept {
        SR_TRACY_ZONE;

        String result;
        {
            SR_TRACY_ZONE_N("Analyze and reserve");
            result.reserve(AnalyzeByteSize(m_root, 0));
            SR_TRACY_ZONE_VALUE(result.capacity());
        }

        /// корневой узел оборачивается в объект, чтобы сохранить его имя
        result += '{';
        NewLine(result, 1);
        SerializeKey(result, m_root.id.GetNameView());
        SerializeValue(result, m_root, 1);
        NewLine(result, 0);
        result += '}';

        if (IsPrettyPrint()) {
            result += '\n';
        }

        return result;
    }

    void JsonISerialization::SerializeValue(String& result, const SerializationNode& node, const uint64_t depth) const {
        if (node.type >= SerializationDataType::String && !node.children.empty()) {
            SRHalt("JsonISerialization::ToString() : node has children!");
        }

        switch (node.type) {
            case SerializationDataType::Root:
            case SerializationDataType::Object:
            case SerializationDataType::Item:
                SerializeObject(result, node, depth);
                return;
            case SerializationDataType::Array:
                SerializeArray(result, node, depth);
                return;
            case SerializationDataType::String:
                SerializeString(result, StringView(node.string));
                return;
            case SerializationDataType::Boolean:
                result += node.data.boolean ? "true" : "false";
                return;
            case SerializationDataType::Integer: {
                char buffer[64];
                result += SerializeInt(node.data.integer, buffer, sizeof(buffer));
                return;
            }
            case SerializationDataType::Floating: {
                if (!std::isfinite(node.data.floating)) {
                    result += "null"; /// inf и nan невозможно представить в JSON
                    return;
                }
                char buffer[64];
                SerializeFloatingPoint(result, SerializeFloat(node.data.floating, buffer, sizeof(buffer)));
                return;
            }
            case SerializationDataType::Double: {
                if (!std::isfinite(node.data.floatingDouble)) {
                    result += "null"; /// inf и nan невозможно представить в JSON
                    return;
                }
                char buffer[64];
                SerializeFloatingPoint(result, SerializeDouble(node.data.floatingDouble, buffer, sizeof(buffer)));
                return;
            }
            case SerializationDataType::Unknown:
            default:
                SRHalt("JsonISerialization::ToString() : unknown type!");
                result += "null";
                return;
        }
    }

    void JsonISerialization::SerializeObject(String& result, const SerializationNode& node, const uint64_t depth) const {
        if (node.children.empty()) {
            result += "{}";
            return;
        }

        result += '{';

        bool isFirst = true;

        for (auto&& child : node.children) {
            if (!isFirst) {
                result += ',';
            }
            isFirst = false;

            NewLine(result, depth + 1);
            SerializeKey(result, child.id.GetNameView());
            SerializeValue(result, child, depth + 1);
        }

        NewLine(result, depth);
        result += '}';
    }

    void JsonISerialization::SerializeArray(String& result, const SerializationNode& node, const uint64_t depth) const {
        result += '[';

        bool isFirst = true;

        for (auto&& child : node.children) {
            if (child.type == SerializationDataType::Item && child.children.empty() && !IsAllowEmptyElementsInArrayImpl()) {
                continue;
            }

            if (!isFirst) {
                result += ',';
            }
            isFirst = false;

            NewLine(result, depth + 1);
            SerializeValue(result, child, depth + 1);
        }

        if (!isFirst) {
            NewLine(result, depth);
        }

        result += ']';
    }

    void JsonISerialization::SerializeKey(String& result, const StringView key) const {
        SerializeString(result, key);
        result += IsPrettyPrint() ? ": " : ":";
    }

    void JsonISerialization::NewLine(String& result, const uint64_t depth) const {
        if (!IsPrettyPrint()) {
            return;
        }

        result += '\n';
        result += GenerateTabs(depth);
    }

    void JsonISerialization::SerializeString(String& result, const StringView value) {
        static constexpr char hexDigits[] = "0123456789abcdef";

        result += '\"';

        uint64_t chunkStart = 0;

        for (uint64_t i = 0; i < value.size(); ++i) {
            const char symbol = value[i];
            StringView escaped;

            switch (symbol) {
                case '\"': escaped = "\\\""; break;
                case '\\': escaped = "\\\\"; break;
                case '\b': escaped = "\\b"; break;
                case '\f': escaped = "\\f"; break;
                case '\n': escaped = "\\n"; break;
                case '\r': escaped = "\\r"; break;
                case '\t': escaped = "\\t"; break;
                default:
                    if (static_cast<uint8_t>(symbol) >= 0x20) {
                        continue; /// всё остальное, включая utf-8, пишется как есть
                    }
                    break;
            }

            result.append(value.data() + chunkStart, static_cast<SizeType>(i - chunkStart));

            if (escaped.empty()) {
                /// прочие управляющие символы кодируются как \u00XX
                const char buffer[] = {
                    '\\', 'u', '0', '0',
                    hexDigits[(static_cast<uint8_t>(symbol) >> 4) & 0xF],
                    hexDigits[static_cast<uint8_t>(symbol) & 0xF]
                };
                result.append(buffer, static_cast<SizeType>(sizeof(buffer)));
            }
            else {
                result += escaped;
            }

            chunkStart = i + 1;
        }

        result.append(value.data() + chunkStart, static_cast<SizeType>(value.size() - chunkStart));
        result += '\"';
    }

    void JsonISerialization::SerializeFloatingPoint(String& result, const StringView value) {
        result += value;

        for (uint64_t i = 0; i < value.size(); ++i) {
            const char symbol = value[i];
            if (symbol == '.' || symbol == 'e' || symbol == 'E') {
                return;
            }
        }

        /// JSON не различает целые и дробные числа, поэтому дробная часть указывается явно
        result += ".0";
    }

    StringView JsonISerialization::GenerateTabs(const uint64_t depth) const {
        if (!IsPrettyPrint()) {
            return {};
        }

        static SR_THREAD_LOCAL String tabs;
        tabs.resize(SR_MAX(128, SR_MAX(tabs.size(), depth)), '\t');
        return StringView(tabs.data(), depth);
    }

    uint64_t JsonISerialization::AnalyzeByteSize(const SerializationNode& node, const uint64_t depth) const {
        uint64_t size = 128;

        switch (node.type) {
            case SerializationDataType::Root:
            case SerializationDataType::Object:
            case SerializationDataType::Array:
            case SerializationDataType::Item:
                size += node.id.GetSize() + 8 + (m_isPrettyPrint ? depth : 0);
                break;
            case SerializationDataType::String:
                size += node.id.GetSize() + 8 + (m_isPrettyPrint ? depth : 0);
                size += node.string.size() + 2;
                break;
            default:
                size += node.id.GetSize() + 8 + (m_isPrettyPrint ? depth : 0);
                size += 10; /// среднее количество символов для числа
                break;
        }

        for (auto&& child : node.children) {
            size += AnalyzeByteSize(child, depth + 1);
        }

        return size;
    }

    /// ========================================== JsonSerializer ======================================================

    JsonSerializer::JsonSerializer() {
        m_root.id = SerializationId::Create("Root");
        m_root.type = SerializationDataType::Root;
    }

    std::unique_ptr<IDeserializer> JsonSerializer::CreateDeserializer() const {
        SR_TRACY_ZONE;

        auto&& pDeserializer = std::make_unique<JsonDeserializer>();
        pDeserializer->m_root = m_root;
        pDeserializer->SetPrettyPrint(IsPrettyPrint());
        pDeserializer->SetDontLoadTags(GetDontSaveTags());

        return pDeserializer;
    }

    /// ========================================= JsonDeserializer =====================================================

    bool JsonDeserializer::LoadFromFile(const SR_UTILS_NS::Path& path) {
        SR_TRACY_ZONE;

        if (path.empty()) {
            SRHalt("JsonDeserializer::LoadFromFile() : empty path!");
            return false;
        }

        if (!path.IsFile()) {
            SR_ERROR("JsonDeserializer::LoadFromFile() : path is not a file!\n\tPath: {}", path);
            return false;
        }

        String buffer;
        if (!FileSystem::ReadFile(path, buffer) || buffer.empty()) {
            SR_ERROR("JsonDeserializer::LoadFromFile() : empty data!\n\tPath: {}", path);
            return false;
        }

        return LoadFromStringView(StringView(buffer));
    }

    bool JsonDeserializer::LoadFromString(const std::string& str) {
        return LoadFromStringView(StringView(str));
    }

    bool JsonDeserializer::LoadFromStringView(const StringView str) {
        SR_TRACY_ZONE;

        m_data = str;
        m_position = 0;
        m_isFailed = false;

        /// пропускаем utf-8 bom, если он есть
        if (m_data.size() >= 3 && static_cast<uint8_t>(m_data[0]) == 0xEF && static_cast<uint8_t>(m_data[1]) == 0xBB && static_cast<uint8_t>(m_data[2]) == 0xBF) {
            m_position = 3;
        }

        const bool result = ParseDocument();

        m_data = StringView(); /// данные принадлежат вызывающей стороне, ссылку хранить нельзя

        if (!result) {
            return false;
        }

        SRAssert2(m_root.type == SerializationDataType::Root, "JsonDeserializer::LoadFromStringView() : invalid root type!");

        return true;
    }

    bool JsonDeserializer::LoadFromNode(SerializationNode&& node) {
        m_root = std::move(node);
        return true;
    }

    bool JsonDeserializer::ParseDocument() {
        SkipWhitespaces();

        if (!Consume('{')) {
            ReportParseError("root value must be an object");
            return false;
        }

        SkipWhitespaces();

        if (Peek() == '}') {
            ReportParseError("root object must contain the only one key");
            return false;
        }

        static SR_THREAD_LOCAL String rootName;

        if (!ParseStringValue(rootName)) {
            return false;
        }

        SkipWhitespaces();

        if (!Consume(':')) {
            ReportParseError("expected ':' after the root key");
            return false;
        }

        if (!ParseValue(m_root, 0)) {
            return false;
        }

        if (m_root.type != SerializationDataType::Object) {
            ReportParseError("root value must be an object");
            return false;
        }

        m_root.type = SerializationDataType::Root;
        m_root.id = SerializationId::CreateFromString(StringView(rootName));

        SkipWhitespaces();

        if (!Consume('}')) {
            ReportParseError("root object must contain the only one key");
            return false;
        }

        SkipWhitespaces();

        if (HasData()) {
            ReportParseError("unexpected data after the root object");
            return false;
        }

        return true;
    }

    bool JsonDeserializer::ParseValue(SerializationNode& node, const uint32_t depth) {
        if (depth >= MaxDepth) {
            ReportParseError("too deep nesting");
            return false;
        }

        SkipWhitespaces();

        if (!HasData()) {
            ReportParseError("unexpected end of data");
            return false;
        }

        switch (Peek()) {
            case '{':
                return ParseObject(node, depth + 1);
            case '[':
                return ParseArray(node, depth + 1);
            case '\"':
                node.type = SerializationDataType::String;
                node.string = String(GetStringsPool());
                return ParseStringValue(node.string);
            case 't':
                if (!ParseLiteral("true")) {
                    return false;
                }
                node.type = SerializationDataType::Boolean;
                node.data.boolean = true;
                return true;
            case 'f':
                if (!ParseLiteral("false")) {
                    return false;
                }
                node.type = SerializationDataType::Boolean;
                node.data.boolean = false;
                return true;
            case 'n':
                if (!ParseLiteral("null")) {
                    return false;
                }
                /// null не имеет аналога в дереве узлов, поэтому трактуется как пустая строка
                node.type = SerializationDataType::String;
                node.string = String(GetStringsPool());
                return true;
            default:
                return ParseNumber(node);
        }
    }

    bool JsonDeserializer::ParseObject(SerializationNode& node, const uint32_t depth) {
        node.type = SerializationDataType::Object;

        if (!Consume('{')) {
            ReportParseError("expected '{'");
            return false;
        }

        SkipWhitespaces();

        if (Consume('}')) {
            return true;
        }

        static SR_THREAD_LOCAL String key;

        while (true) {
            SkipWhitespaces();

            if (!ParseStringValue(key)) {
                return false;
            }

            SkipWhitespaces();

            if (!Consume(':')) {
                ReportParseError("expected ':' after the object key");
                return false;
            }

            auto&& child = node.AddChild(GetNodesPool());
            child.id = SerializationId::CreateFromString(StringView(key));

            /// ключ будет перезаписан вложенными объектами, но он уже не нужен
            if (!ParseValue(child, depth)) {
                return false;
            }

            SkipWhitespaces();

            if (Consume(',')) {
                continue;
            }

            if (Consume('}')) {
                return true;
            }

            ReportParseError("expected ',' or '}'");
            return false;
        }
    }

    bool JsonDeserializer::ParseArray(SerializationNode& node, const uint32_t depth) {
        node.type = SerializationDataType::Array;

        if (!Consume('[')) {
            ReportParseError("expected '['");
            return false;
        }

        SkipWhitespaces();

        if (Consume(']')) {
            return true;
        }

        while (true) {
            auto&& child = node.AddChild(GetNodesPool());

            if (!ParseValue(child, depth)) {
                return false;
            }

            /// объекты внутри массива являются его элементами
            if (child.type == SerializationDataType::Object) {
                child.type = SerializationDataType::Item;
            }

            SkipWhitespaces();

            if (Consume(',')) {
                continue;
            }

            if (Consume(']')) {
                return true;
            }

            ReportParseError("expected ',' or ']'");
            return false;
        }
    }

    bool JsonDeserializer::ParseStringValue(String& result) {
        if (!Consume('\"')) {
            ReportParseError("expected '\"'");
            return false;
        }

        result.clear();

        uint64_t chunkStart = m_position;

        while (HasData()) {
            const char symbol = m_data[m_position];

            if (symbol == '\"') {
                result.append(m_data.data() + chunkStart, static_cast<SizeType>(m_position - chunkStart));
                ++m_position;
                return true;
            }

            if (symbol == '\\') {
                result.append(m_data.data() + chunkStart, static_cast<SizeType>(m_position - chunkStart));
                ++m_position;

                if (!HasData()) {
                    break;
                }

                const char escaped = m_data[m_position];
                ++m_position;

                switch (escaped) {
                    case '\"': result += '\"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u':
                        if (!ParseUnicodeEscape(result)) {
                            return false;
                        }
                        break;
                    default:
                        ReportParseError("unknown escape sequence");
                        return false;
                }

                chunkStart = m_position;
                continue;
            }

            if (static_cast<uint8_t>(symbol) < 0x20) {
                ReportParseError("unescaped control character in the string");
                return false;
            }

            ++m_position;
        }

        ReportParseError("unterminated string");
        return false;
    }

    bool JsonDeserializer::ParseUnicodeEscape(String& result) {
        static constexpr uint32_t ReplacementCodePoint = 0xFFFD;

        const auto readCodeUnit = [this](uint32_t& value) -> bool {
            if (m_position + 4 > m_data.size()) {
                return false;
            }

            value = 0;

            for (uint32_t i = 0; i < 4; ++i) {
                const char symbol = m_data[m_position];
                ++m_position;

                if (symbol >= '0' && symbol <= '9') {
                    value = (value << 4) | static_cast<uint32_t>(symbol - '0');
                }
                else if (symbol >= 'a' && symbol <= 'f') {
                    value = (value << 4) | static_cast<uint32_t>(symbol - 'a' + 10);
                }
                else if (symbol >= 'A' && symbol <= 'F') {
                    value = (value << 4) | static_cast<uint32_t>(symbol - 'A' + 10);
                }
                else {
                    return false;
                }
            }

            return true;
        };

        uint32_t codePoint = 0;

        if (!readCodeUnit(codePoint)) {
            ReportParseError("invalid unicode escape sequence");
            return false;
        }

        if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
            /// старший символ суррогатной пары, ожидаем младший
            if (m_position + 2 <= m_data.size() && m_data[m_position] == '\\' && m_data[m_position + 1] == 'u') {
                const uint64_t position = m_position;
                m_position += 2;

                uint32_t lowCodePoint = 0;
                if (!readCodeUnit(lowCodePoint)) {
                    ReportParseError("invalid unicode escape sequence");
                    return false;
                }

                if (lowCodePoint >= 0xDC00 && lowCodePoint <= 0xDFFF) {
                    codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (lowCodePoint - 0xDC00);
                }
                else {
                    m_position = position;
                    codePoint = ReplacementCodePoint;
                }
            }
            else {
                codePoint = ReplacementCodePoint;
            }
        }
        else if (codePoint >= 0xDC00 && codePoint <= 0xDFFF) {
            codePoint = ReplacementCodePoint; /// младший символ суррогатной пары без старшего
        }

        if (codePoint < 0x80) {
            result += static_cast<char>(codePoint);
        }
        else if (codePoint < 0x800) {
            result += static_cast<char>(0xC0 | (codePoint >> 6));
            result += static_cast<char>(0x80 | (codePoint & 0x3F));
        }
        else if (codePoint < 0x10000) {
            result += static_cast<char>(0xE0 | (codePoint >> 12));
            result += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (codePoint & 0x3F));
        }
        else {
            result += static_cast<char>(0xF0 | (codePoint >> 18));
            result += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
            result += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (codePoint & 0x3F));
        }

        return true;
    }

    bool JsonDeserializer::ParseNumber(SerializationNode& node) {
        const uint64_t start = m_position;

        uint32_t digitCount = 0;
        bool isFloating = false;

        if (Peek() == '-' || Peek() == '+') {
            ++m_position;
        }

        while (HasData()) {
            const char symbol = m_data[m_position];

            if (symbol >= '0' && symbol <= '9') {
                ++digitCount;
                ++m_position;
                continue;
            }

            if (symbol == '.' || symbol == 'e' || symbol == 'E') {
                isFloating = true;
                ++m_position;

                if (symbol != '.' && (Peek() == '-' || Peek() == '+')) {
                    ++m_position;
                }

                continue;
            }

            break;
        }

        if (digitCount == 0) {
            ReportParseError("unexpected token");
            return false;
        }

        const std::string_view number(m_data.data() + start, m_position - start);

        if (isFloating) {
            node.type = SerializationDataType::Double;
            node.data.floatingDouble = ParseDouble(number);
        }
        else {
            node.type = SerializationDataType::Integer;
            node.data.integer = FastSToL(number);
        }

        return true;
    }

    bool JsonDeserializer::ParseLiteral(const StringView literal) {
        if (m_position + literal.size() > m_data.size() || m_data.substr(m_position, literal.size()) != literal) {
            ReportParseError("unexpected token");
            return false;
        }

        m_position += literal.size();

        return true;
    }

    bool JsonDeserializer::Consume(const char symbol) {
        if (Peek() != symbol) {
            return false;
        }

        ++m_position;

        return true;
    }

    void JsonDeserializer::SkipWhitespaces() noexcept {
        while (HasData()) {
            switch (m_data[m_position]) {
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    ++m_position;
                    break;
                default:
                    return;
            }
        }
    }

    bool JsonDeserializer::BeginItem(const SerializationId& id, const uint32_t index) {
        auto&& node = GetWalkNode();

        if (node.children.size() <= index) {
            return false;
        }

        auto&& child = node.children[index];
        if (child.type != SerializationDataType::Item) {
            return false;
        }

        /// имена элементов массива в JSON не хранятся, поэтому восстанавливаем их при чтении
        if (child.id.GetHash() == 0) {
            child.id = id;
        }

        m_walker.emplace_back(&child);

        return true;
    }

    void JsonDeserializer::ReportParseError(const std::string& message) {
        if (m_isFailed) {
            return; /// сообщаем только о первой ошибке, дальше разбор всё равно не имеет смысла
        }

        m_isFailed = true;

        uint64_t line = 1;
        uint64_t column = 1;

        for (uint64_t i = 0; i < m_position && i < m_data.size(); ++i) {
            if (m_data[i] == '\n') {
                ++line;
                column = 1;
            }
            else {
                ++column;
            }
        }

        SR_ERROR("JsonDeserializer::ReportParseError() : {}!\n\tLine: {}, column: {}", message, line, column);
    }

    void JsonDeserializer::ReportError(const std::string& message) {
        SR_TRACY_ZONE;
        SRHalt("JsonDeserializer::ReportError() : {}!", message);
    }
} // namespace SR_UTILS_NS
