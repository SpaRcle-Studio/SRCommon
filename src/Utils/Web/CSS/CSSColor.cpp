//
// Created by Monika on 20.08.2024.
//

#include <Utils/Web/CSS/CSSColor.h>

namespace SR_UTILS_NS::Web {
    CSSRGBAColor CSSRGBAColor::Parse(std::string_view data) {
        if (data.empty()) {
            SRHalt("CSSRGBAColor::Parse() : empty data!");
            return {};
        }

        const auto color = SR_UTILS_NS::LexicalCast<SR_MATH_NS::FColor>(data);
        return {
            static_cast<uint8_t>(color.r * 255.f),
            static_cast<uint8_t>(color.g * 255.f),
            static_cast<uint8_t>(color.b * 255.f),
            static_cast<uint8_t>(color.a * 255.f)
        };
    }

    CSSLinearGradient CSSLinearGradient::Parse(std::string_view data) {
        if (data.empty()) {
            SRHalt("CSSLinearGradient::Parse() : empty data!");
            return {};
        }

        CSSLinearGradient result;

        constexpr size_t prefixLength = sizeof("linear-gradient(") - 1;
        auto&& values = SR_UTILS_NS::StringUtils::Split(std::string(data.substr(prefixLength, data.size() - prefixLength - 1)), ",");
        if (values.size() < 2) {
            SRHalt("CSSLinearGradient::Parse() : invalid data!");
            return {};
        }

        static std::map<SRHashType, CSSDirection::Type> directions = {
            { "to top"_atom_hash, CSSDirection::Type::Top },
            { "to right"_atom_hash, CSSDirection::Type::Right },
            { "to bottom"_atom_hash, CSSDirection::Type::Bottom },
            { "to left"_atom_hash, CSSDirection::Type::Left },
            { "to top right"_atom_hash, CSSDirection::Type::TopRight },
            { "to bottom right"_atom_hash, CSSDirection::Type::BottomRight },
            { "to bottom left"_atom_hash, CSSDirection::Type::BottomLeft },
            { "to top left"_atom_hash, CSSDirection::Type::TopLeft },
            { "to right top"_atom_hash, CSSDirection::Type::TopRight },
            { "to right bottom"_atom_hash, CSSDirection::Type::BottomRight },
            { "to left bottom"_atom_hash, CSSDirection::Type::BottomLeft },
            { "to left top"_atom_hash, CSSDirection::Type::TopLeft },
        };

        for (size_t i = 0; i < values.size(); ++i) {
            auto& value = values[i];

            if (const auto it = directions.find(SR_HASH_STR_VIEW(value)); it != directions.end()) {
                result.direction.direction = it->second;
                result.direction.type = CSSDirection::DirectionType::To;
            }
            else if (value.ends_with("deg")) {
                result.direction.degree = SR_UTILS_NS::LexicalCast<float_t>(value.substr(0, value.size() - 3));
                result.direction.type = CSSDirection::DirectionType::Degree;
            }
            else {
                const auto colorSplit = SR_UTILS_NS::StringUtils::Split(value, " ");
                CSSGradientColor gradientColor;

                if (!colorSplit.empty()) {
                    gradientColor.color = CSSRGBAColor::Parse(colorSplit[0]);
                }

                if (colorSplit.size() > 1) {
                    if (const auto sizeValue = CSSSizeValue::Parse(colorSplit[1])) {
                        gradientColor.size = sizeValue.value();
                    }
                }

                result.colors.emplace_back(gradientColor);
            }
        }

        return result;
    }

    std::string CSSLinearGradient::ToString() const {
        std::string result = "linear-gradient(";
        switch (direction.type) {
            case CSSDirection::DirectionType::To:
                result += "to ";
                switch (direction.direction) {
                    case CSSDirection::Type::Top: result += "top"; break;
                    case CSSDirection::Type::Right: result += "right"; break;
                    case CSSDirection::Type::Bottom: result += "bottom"; break;
                    case CSSDirection::Type::Left: result += "left"; break;
                    case CSSDirection::Type::TopRight: result += "top right"; break;
                    case CSSDirection::Type::BottomRight: result += "bottom right"; break;
                    case CSSDirection::Type::BottomLeft: result += "bottom left"; break;
                    case CSSDirection::Type::TopLeft: result += "top left"; break;
                    default: result += "undefined"; break;
                }
                break;
            case CSSDirection::DirectionType::Degree:
                result += SR_FORMAT("{}deg", direction.degree);
                break;
            default:
                result += "undefined";
                break;
        }

        if (!colors.empty()) {
            result += ", ";
        }

        for (size_t i = 0; i < colors.size(); ++i) {
            const auto& color = colors[i];
            result += SR_FORMAT("{} {}", color.color.ToString(), color.size.ToString());
            if (i + 1 < colors.size()) {
                result += ", ";
            }
        }

        return result + ")";
    }

    CSSRadialGradient CSSRadialGradient::Parse(std::string_view data) {
        if (data.empty()) {
            SRHalt("CSSRadialGradient::Parse() : empty data!");
            return {};
        }

        CSSRadialGradient result;
        return result;
    }

    std::string CSSRadialGradient::ToString() const {
        std::string result = "radial-gradient(";
        return result + ")";
    }

    CSSColor CSSColor::Parse(std::string_view data) {
        if (data.empty()) {
            SRHalt("CSSColor::Parse() : empty data!");
            return {};
        }

        CSSColor result;

        if (data.starts_with("linear-gradient")) {
            result.colorType = ColorType::LinearGradient;
            result.linearGradient = CSSLinearGradient::Parse(data);
            return result;
        }

        if (data.starts_with("radial-gradient")) {
            result.colorType = ColorType::RadialGradient;
            result.radialGradient = CSSRadialGradient::Parse(data);
            return result;
        }

        result.colorType = ColorType::RGBA;
        result.color = CSSRGBAColor::Parse(data);

        return result;
    }
}
