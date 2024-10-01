//
// Created by Monika on 22.09.2021.
//

#ifndef SR_ENGINE_UTILS_STRING_FORMAT_H
#define SR_ENGINE_UTILS_STRING_FORMAT_H

#include <Utils/Math/Mathematics.h>
#include <Utils/Types/StringAtom.h>
#include <Utils/Common/Breakpoint.h>
#include <Utils/Math/Vector4.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Common/StringUtils.h>

#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/color.h>

namespace SR_UTILS_NS {
    template<typename T> T LexicalCast(std::string_view str) {
        try {
            if constexpr (std::is_same<T, bool>()) {
                const char c = str.front();
                return c == 't' || c == 'T' || c == 'y' || c == 'Y' || c == '1';
            }
            else if constexpr (std::is_same<T, int8_t>()) {
                return static_cast<int8_t>(str.front());
            }
            else if constexpr (std::is_same<T, uint8_t>()) {
                return static_cast<uint8_t>(str.front());
            }
            else if constexpr (std::is_same<T, int16_t>()) {
                return std::stoi(str.data());
            }
            else if constexpr (std::is_same<T, uint16_t>()) {
                return static_cast<uint16_t>(std::stoi(str.data()));
            }
            else if constexpr (std::is_same<T, int32_t>()) {
                return std::stoi(str.data());
            }
            else if constexpr (std::is_same<T, int64_t>()) {
                return std::stoll(str.data());
            }
            else if constexpr (std::is_same<T, uint32_t>()) {
                return static_cast<uint32_t>(std::stoi(str.data()));
            }
            else if constexpr (std::is_same<T, uint64_t>()) {
                return static_cast<uint64_t>(std::stoll(str.data()));
            }
            else if constexpr (std::is_same<T, float_t>() || std::is_same<T, float>()) {
                return std::stof(str.data());
            }
            else if constexpr (std::is_same<T, double_t>() || std::is_same<T, double>() || std::is_same<T, Math::Unit>()) {
                return std::stod(str.data());
            }
            else if constexpr (std::is_same<T, SR_MATH_NS::FColor>()) {
                if (str.empty()) {
                    SR_PLATFORM_NS::WriteConsoleError("LexicalCast: empty string!\n");
                    SR_MAKE_BREAKPOINT;
                    return T();
                }

                if (str.size() >= 5) {
                    const bool r = tolower(str[0]) == 'r';
                    const bool g = tolower(str[1 + (r - 1)]) == 'g';
                    const bool b = tolower(str[2 + (r - 1) + (g - 1)]) == 'b';
                    const bool a = tolower(str[3 + (r - 1) + (g - 1) + (b - 1)]) == 'a';
                    uint8_t offset = r + g + b + a; // max 4

                    if (str[offset] == '(' && str.back() == ')') {
                        std::vector<std::string> values;

                        values = SR_UTILS_NS::StringUtils::Split(std::string(str.substr(offset + 1, str.size() - (offset + 1) - 1)), ",");

                        uint8_t index = 0;
                        SR_MATH_NS::FColor color = SR_MATH_NS::FColor::Alpha();
                        if (index < values.size() && r) {
                            color.r = (LexicalCast<float_t>(values[index++])) / 255.0f;
                        }
                        if (index < values.size() && g) {
                            color.g = (LexicalCast<float_t>(values[index++])) / 255.0f;
                        }
                        if (index < values.size() && b) {
                            color.b = (LexicalCast<float_t>(values[index++])) / 255.0f;
                        }
                        if (index < values.size() && a) {
                            color.a = (LexicalCast<float_t>(values[index++])) / 255.0f;
                        }
                        return color;
                    }
                }

                if (str.size() > 9) {
                    SR_PLATFORM_NS::WriteConsoleError("LexicalCast: unsupported color format!\n");
                    SR_MAKE_BREAKPOINT;
                    return T();
                }

                const bool hasSharp = str[0] == '#';
                SR_MATH_NS::FColor color; /// FF0000FF
                if (str.size() >= 2 + hasSharp) {
                    color.r = static_cast<float_t>(LexicalCast<uint8_t>(str.substr(hasSharp + 0, 2))) / 255.0f;
                }
                if (str.size() >= 4 + hasSharp) {
                    color.g = static_cast<float_t>(LexicalCast<uint8_t>(str.substr(hasSharp + 2, 2))) / 255.0f;
                }
                if (str.size() >= 6 + hasSharp) {
                    color.b = static_cast<float_t>(LexicalCast<uint8_t>(str.substr(hasSharp + 4, 2))) / 255.0f;
                }
                if (str.size() >= 8 + hasSharp) {
                    color.a = static_cast<float_t>(LexicalCast<uint8_t>(str.substr(hasSharp + 6, 2))) / 255.0f;
                }

                return color;
            }
            else {
                SR_PLATFORM_NS::WriteConsoleError("LexicalCast: unsupported type!\n");
                SR_MAKE_BREAKPOINT;
                return T();
            }
        }
        catch (...) {
            SR_PLATFORM_NS::WriteConsoleError("LexicalCast: failed to cast!\n");
            SR_MAKE_BREAKPOINT;
            return T();
        }
    }

    template <class... Args> std::string Format(const char* fmt, Args&&... args) {
        try {
            return fmt::format(fmt::runtime(fmt), args...);
        }
        catch (std::exception& exception) {
            std::cerr << "Format: an exception has been occurred! Exception: " << exception.what() << std::endl;
            SR_MAKE_BREAKPOINT;
            return std::string(); /// NOLINT
        }
    }

    template <class... Args> std::string SPrintF(const char* fmt, Args&&... args) {
        try {
            return fmt::sprintf(fmt, args...);
        }
        catch (std::exception& exception) {
            std::cerr << "SPrintFFormat: an exception has been occurred! Exception: " << exception.what() << std::endl;
            SR_MAKE_BREAKPOINT;
            return std::string(); /// NOLINT
        }
    }

    template <class... Args> std::string Format(const std::string& fmt, Args&&... args) {
        return Format<Args...>(fmt.c_str(), args...);
    }
}

#define SR_FORMAT(...) SR_UTILS_NS::Format(__VA_ARGS__)
#define SR_SPRINTF(...) SR_UTILS_NS::SPrintF(__VA_ARGS__)
#define SR_FORMAT_C(...) SR_UTILS_NS::Format(__VA_ARGS__).c_str()

#endif //SR_ENGINE_UTILS_STRING_FORMAT_H
