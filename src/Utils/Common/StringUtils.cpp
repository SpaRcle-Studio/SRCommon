//
// Created by Nikita on 17.11.2020.
//

#include <Utils/Common/StringUtils.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    std::string StringUtils::Reverse(const std::string& str) {
        std::string result;
        result.resize(str.size());

        size_t i = 0;

        for (size_t t = str.size(); t > 0; --t, ++i) {
            result[i] += str[t - 1];
        }

        return result;
    }

    std::string StringUtils::ToKebabCase(std::string_view str) {
        std::string result;
        bool was_lower_or_digit = false; // to detect camelCase boundaries
        bool in_separator = false;

        for (size_t i = 0; i < str.size(); ++i) {
            char ch = str[i];

            if (std::isalnum(static_cast<unsigned char>(ch))) {
                if (std::isupper(static_cast<unsigned char>(ch))) {
                    if (was_lower_or_digit && !in_separator) {
                        result += '-';
                    }
                    result += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                    in_separator = false;
                } else {
                    result += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                    in_separator = false;
                }
                was_lower_or_digit = std::islower(ch) || std::isdigit(ch);
            }
            else if (ch == ' ' || ch == '_' || ch == '-') {
                if (!in_separator && !result.empty()) {
                    result += '-';
                    in_separator = true;
                }
                was_lower_or_digit = false;
            }
        }

        if (!result.empty() && result.back() == '-') {
            result.pop_back();
        }

        return result;
    }

    std::string StringUtils::ReadFrom(const std::string &str, const char &c, uint32_t start) {
        if (start >= str.size())
            return std::string();

        std::string newStr = str.substr(start, str.size() - 1);

        int32_t to = IndexOf(newStr, c);
        if (to <= 0)
            return std::string();

        return newStr.substr(0, to);
    }

    std::vector<std::string> StringUtils::Split(std::string source, const std::string &delimiter)  {
        size_t pos = 0;
        std::vector<std::string> tokens = {};
        while ((pos = source.find(delimiter)) != std::string::npos) {
            if (auto&& token = source.substr(0, pos); !token.empty())
                tokens.emplace_back(std::move(token));
            source.erase(0, pos + delimiter.length());
        }

        if (!source.empty())
            tokens.emplace_back(source);

        return tokens;
    }

    std::vector<std::string_view> StringUtils::SplitView(std::string_view source, std::string_view delimiter) {
        size_t pos = 0;

        std::vector<std::string_view> tokens;
        while ((pos = source.find(delimiter)) != std::string::npos) {
            if (auto&& token = source.substr(0, pos); !token.empty()) {
                tokens.emplace_back(token);
            }
            source.remove_prefix(pos + delimiter.length());
        }

        if (!source.empty()) {
            tokens.emplace_back(source);
        }

        return tokens;
    }

    std::vector<std::string_view> StringUtils::SplitViewWithEmpty(std::string_view source, std::string_view delimiter) {
        size_t pos = 0;
        std::vector<std::string_view> tokens;

        while ((pos = source.find(delimiter)) != std::string::npos) {
            auto&& token = source.substr(0, pos);
            tokens.emplace_back(token);
            source.remove_prefix(pos + delimiter.length());
        }

        if (!source.empty()) {
            tokens.emplace_back(source);
        }

        return tokens;
    }

    std::string StringUtils::Tab(std::string code, uint32_t count) {
        if (!code.empty()) {
            code = std::string(count, '\t') + code;

            uint64_t lastPos = 0;

            do {
                const auto pos = code.find("\n", lastPos);
                lastPos = pos + 1;

                if (pos == std::string::npos || pos + 1 == code.size()) {
                    break;
                }

                code.insert(pos + 1, std::string(count, '\t'));
            }
            while (true);
        }

        return code;
    }

    std::string StringUtils::Base64Encode(const std::string &data) {
        int in_len = static_cast<int>(data.size());
        char* bytes_to_encode = (char*)(data.data());

        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while (in_len--) {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; (i <4) ; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i)
        {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++)
                ret += base64_chars[char_array_4[j]];

            while((i++ < 3))
                ret += '=';

        }

        return ret;
    }

    std::string StringUtils::Base64Decode(const std::string & base64) {
        SR_TRACY_ZONE;

        int in_len = static_cast<int>(base64.size());
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;

        while (in_len-- && (base64[in_] != '=') && is_base64(base64[in_])) {
            char_array_4[i++] = base64[in_]; in_++;
            if (i ==4) {
                for (i = 0; i <4; i++)
                    char_array_4[static_cast<size_t>(i)] = static_cast<unsigned char>(std::string_view(base64_chars).find(char_array_4[static_cast<size_t>(i)]));

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret += char_array_3[i];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j <4; j++)
                char_array_4[j] = 0;

            for (j = 0; j <4; j++)
                char_array_4[j] = static_cast<unsigned char>(std::string_view(base64_chars).find(char_array_4[j]));

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
        }

        return ret;
    }

    std::string StringUtils::Remove(std::string source, uint32_t count) {
        return source.substr(count, source.size() - count);
    }

    std::string StringUtils::Remove(std::string source, uint32_t start, uint32_t count) {
        const uint64_t size = source.size();

        if (static_cast<int64_t>(size) - count < 0) {
            SRHalt("StringUtils::Remove() : out of range!");
            return std::string();
        }

        return source.substr(count + start, size - count);
    }

    std::string StringUtils::ReadNumber(const std::string& str, uint32_t start) {
        if (start >= str.size()) {
            return std::string();
        }

        std::string numberStr;
        numberStr.reserve(str.size() / 2);

        for (uint32_t i = start; i < str.size(); ++i) {
            switch (str[i]) {
                case '-':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case '.':
                    numberStr += str[i];
                    break;
                default:
                    return numberStr;
            }
        }

        return numberStr;
    }

    std::string StringUtils::ReplaceAllRecursive(const std::string &original, const std::vector<std::string> &fromList, const std::string &to) noexcept {
        std::string result = original;

    repeat:
        for (const auto& from : fromList) {
            size_t pos = result.find(from);

            if (pos != std::string::npos) {
                result.replace(pos, from.size(), to);
                goto repeat;
            }
        }

        return result;
    }

    std::string StringUtils::GetBetween(const std::string &source, int64_t begin, uint64_t end) {
        return source.substr(begin + 1, (end - begin) - 1);
    }

    std::string_view StringUtils::GetBetween(std::string_view source, char begin, char end, int8_t *pResult) {
        if (pResult) {
            *pResult = 0;
        }

        auto first = source.find(begin);
        if (first == std::string::npos) {
            first = 0;
            if (pResult) {
                *pResult -= 1;
            }
        }

        auto last = source.find(end);
        if (last == std::string::npos) {
            last = source.size() - 1;
            if (pResult) {
                *pResult -= 1;
            }
        }

        return source.substr(first + 1, (last - first) - 1);
    }

    std::string StringUtils::GetBetween(const std::string &source, const std::string &begin, const std::string &end) {
        auto first = source.find(begin);
        if (first == std::string::npos)
            first = 0;

        auto last = source.find(end);
        if (last == std::string::npos)
            last = source.size() - 1;

        return source.substr(first + 1, (last - first) - 1);
    }

    std::string_view StringUtils::SubstringView(const std::string_view &source, StringAtom substr, uint32_t offset) {
        if (auto&& pos = source.find(substr.ToStringView()); pos == std::string::npos) {
            return source;
        }
        else {
            return source.substr(pos + offset, source.size() - 1);
        }
    }

    std::string_view StringUtils::SubstringView(const std::string_view &source, char symbol, uint32_t offset) {
        if (auto&& pos = source.find(symbol); pos == std::string::npos) {
            return source;
        }
        else {
            return source.substr(pos + offset, source.size() - 1);
        }
    }

    std::pair<std::string, std::string> StringUtils::SplitTwo(std::string source, const std::string &delimiter) {
        std::pair<std::string, std::string> result = {};
        auto pos = source.find(delimiter);
        if (pos == std::string::npos) {
            result.first = source;
            result.second = "";
            return result;
        }

        result.first = source.substr(0, pos);
        source.erase(0, pos + delimiter.length());
        result.second = source;

        return result;
    }
}
