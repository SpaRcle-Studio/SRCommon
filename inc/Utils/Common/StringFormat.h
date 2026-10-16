//
// Created by Monika on 22.09.2021.
//

#if !defined(SR_ENGINE_UTILS_STRING_FORMAT_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_ENGINE_UTILS_STRING_FORMAT_H

#include <Utils/Types/String.h>

namespace SR_UTILS_NS {
    namespace Details {
        extern void StringFormatError(StringView msg, bool breakPoint);
    }

    template <class... Args> SR_NODISCARD String Format(const char* format_str, Args&&... args) {
        try {
            auto&& string = fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...);
            return StringView(string);
        }
        catch (std::exception& exception) {
            Details::StringFormatError("Format() : an exception has occurred! Exception: " + String(exception.what()) + "\n", true);
            return String(); /// NOLINT
        }
    }

    template <class... Args> SR_NODISCARD String Format(StringView format_str, Args&&... args) {
        return Format(format_str.c_str(), std::forward<Args>(args)...);
    }

    template <class... Args> SR_NODISCARD String Format(const std::string& format_str, Args&&... args) {
        return Format(format_str.c_str(), std::forward<Args>(args)...);
    }

    template <class... Args> SR_NODISCARD String Format(const String& format_str, Args&&... args) {
        return Format(format_str.c_str(), std::forward<Args>(args)...);
    }

    template <class... Args> SR_NODISCARD String SPrintF(const char* format_str, Args&&... args) {
        try {
            return fmt::sprintf(format_str, std::forward<Args>(args)...);
        }
        catch (std::exception& exception) {
            Details::StringFormatError("SPrintF() : an exception has occurred! Exception: " + String(exception.what()) + "\n", true);
            return String(); /// NOLINT
        }
    }

    template<class... Args> void FormatTo(String& dest, const char* format_str, Args&&... args) {
        try {
            fmt::format_to(std::back_inserter(dest), fmt::runtime(format_str), std::forward<Args>(args)...);
        }
        catch (std::exception& exception) {
            Details::StringFormatError("FormatTo() : an exception has occurred! Exception: " + String(exception.what()) + "\n", true);
        }
    }

    template<class... Args> void FormatConcat(String& dest, String& buffer, const char* format_str, Args&&... args) {
        try {
            buffer.clear();
            fmt::format_to(std::back_inserter(buffer), fmt::runtime(format_str), std::forward<Args>(args)...);
            dest += buffer;
        }
        catch (std::exception& exception) {
            Details::StringFormatError("FormatConcat() : an exception has occurred! Exception: " + String(exception.what()) + "\n", true);
        }
    }
}

#define SR_FORMAT(...) SR_UTILS_NS::Format(__VA_ARGS__)
#define SR_SPRINTF(...) SR_UTILS_NS::SPrintF(__VA_ARGS__)
#define SR_FORMAT_C(...) SR_UTILS_NS::Format(__VA_ARGS__).c_str()

#endif //SR_ENGINE_UTILS_STRING_FORMAT_H
