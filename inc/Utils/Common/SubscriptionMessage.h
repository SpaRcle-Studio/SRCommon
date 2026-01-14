//
// Created by Monika on 01.12.2025.
//

#ifndef SR_ENGINE_UTILS_SUBSCRIPTION_MESSAGE_H
#define SR_ENGINE_UTILS_SUBSCRIPTION_MESSAGE_H

#include <Utils/FileSystem/Path.h>
#include <Utils/Types/StringAtom.h>
#include <Utils/Common/NonCopyable.h>

namespace SR_UTILS_NS {
    class SR_COMMON_DLL_API SubscriptionMessage final : SR_UTILS_NS::NonCopyable {
    public:
        SubscriptionMessage();
        ~SubscriptionMessage() override;

        void SetInt(StringAtom id, uint64_t value);
        void SetBool(StringAtom id, bool value);
        void SetString(StringAtom id, const std::string& value);
        void SetPath(StringAtom id, const SR_UTILS_NS::Path& value);

        void Reset();

        SR_NODISCARD uint64_t GetInt(StringAtom id, const std::optional<uint64_t>& def = std::nullopt) const;
        SR_NODISCARD bool GetBool(StringAtom id, const std::optional<bool>& def = std::nullopt) const;
        SR_NODISCARD std::string GetString(StringAtom id, const std::optional<std::string>& def = std::nullopt) const;
        SR_NODISCARD SR_UTILS_NS::Path GetPath(StringAtom id, const std::optional<SR_UTILS_NS::Path>& def = std::nullopt) const;
        SR_NODISCARD const SR_UTILS_NS::Path& GetPathRef(StringAtom id) const;

    private:
        static void PrintError(const char* format, StringAtom id);

    private:
        template<typename T, typename Container> SR_NODISCARD T GetValue(const StringAtom id, const Container& container, const std::optional<T> def) const {
            if (const auto it = container.find(id); it != container.end()) {
                return it->second;
            }
            if (def.has_value()) {
                return def.value();
            }
            PrintError("SubscriptionMessage::GetValue() : id \"{}\" not found!", id);
            return T();
        }

        template<typename T, typename Container> SR_NODISCARD const T& GetValueRef(const StringAtom id, const Container& container) const {
            if (const auto it = container.find(id); it != container.end()) {
                return it->second;
            }
            PrintError("SubscriptionMessage::GetValueRef() : id \"{}\" not found!", id);
            static T defaultValue;
            return defaultValue;
        }

    private:
        std::map<StringAtom, uint64_t> m_ints;
        std::map<StringAtom, bool> m_bools;
        std::map<StringAtom, std::string> m_strings;
        std::map<StringAtom, SR_UTILS_NS::Path> m_paths;

    };
}

#endif //SR_ENGINE_UTILS_SUBSCRIPTION_MESSAGE_H
