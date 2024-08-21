//
// Created by Monika on 20.08.2024.
//

#include <Utils/Web/CSS/CSSParser.h>

#include <cssparser/cssparser/CSSParser.h>

namespace SR_UTILS_NS::Web {
    CSS::Ptr CSSParser::Parse(const Path& path, CSSParserContext context) {
        const std::string data = SR_UTILS_NS::FileSystem::ReadBinaryAsString(path);
        return Parse(data);
    }

    CSS::Ptr CSSParser::Parse(const std::string& data, CSSParserContext context) {
        if (data.empty()) {
            SRHalt("CSSParser::Parse() : empty data");
            return {};
        }

        m_pCSS = CSS::MakeShared();

        ::CSSParser parser;

        parser.set_level("CSS3.0");
        parser.parse_css(data);

        const std::vector<std::string> errors = parser.get_parse_errors();
        const std::vector<std::string> warnings = parser.get_parse_warnings();
        const std::vector<std::string> infos = parser.get_parse_info();

        for (const auto& error : errors) {
            SR_ERROR("CSSParser::Parse() : {}", error.c_str());
        }

        for (const auto& warning : warnings) {
            SR_WARN("CSSParser::Parse() : {}", warning.c_str());
        }

        for (const auto& info : infos) {
            SR_INFO("CSSParser::Parse() : {}", info.c_str());
        }

        ::CSSParser::token token = parser.get_next_token();

        std::optional<CSSStyle> style;
        std::string styleName;
        std::string propertyName;

        while (token.type != ::CSSParser::CSS_END) {
            if (token.type == ::CSSParser::COMMENT || token.type == ::CSSParser::AT_END) {
                token = parser.get_next_token();
                continue;
            }

            /** propery value */

            if (!propertyName.empty()) {
                if (!style.has_value()) {
                    SRHalt("CSSParser::Parse() : unexpected PROPERTY!");
                    propertyName.clear();
                    token = parser.get_next_token();
                    continue;
                }

                if (token.type == ::CSSParser::VALUE) {
                    style->ParseProperty(propertyName, token.data);
                    propertyName.clear();
                }
                else {
                    SRHalt("CSSParser::Parse() : unexpected token type!");
                }

                token = parser.get_next_token();
                continue;
            }

            if (token.type == ::CSSParser::SEL_END) {
                if (style.has_value()) {
                    m_pCSS->AddStyle(SR_EXCHANGE(styleName, {}), style.value());
                    style.reset();
                }
                else {
                    SR_WARN("CSSParser::Parse() : unexpected SEL_END!");
                }
                token = parser.get_next_token();
                continue;
            }

            /** media requests */

            if (token.type == ::CSSParser::AT_START) {
                if (!EvaluateMedia(token.data, context)) {
                    while (token.type != ::CSSParser::AT_END) {
                        if (token.type == ::CSSParser::CSS_END) {
                            break;
                        }
                        token = parser.get_next_token();
                    }
                    continue;
                }
                token = parser.get_next_token();
                continue;
            }

            /** styles */

            if (token.type == ::CSSParser::SEL_START) {
                if (token.data.starts_with(".")) {
                    style = CSSStyle();
                    styleName = token.data.substr(1);
                }
                else {
                    SR_WARN("CSSParser::Parse() : unexpected SEL_START!");
                }
                token = parser.get_next_token();
                continue;
            }

            /** properties */

            if (token.type == ::CSSParser::PROPERTY) {
                propertyName = token.data;
                token = parser.get_next_token();
                continue;
            }

            token = parser.get_next_token();
        }

        return SR_EXCHANGE(m_pCSS, {});
    }

    bool CSSParser::EvaluateMedia(const std::string& media, const CSSParserContext& context) const {
        volatile bool result = true;
        return result;
    }
}

