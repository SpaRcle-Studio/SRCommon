//
// Created by Monika on 01.12.2025.
//

#ifndef SR_ENGINE_UTILS_SUBSCRIPTION_MESSAGE_H
#define SR_ENGINE_UTILS_SUBSCRIPTION_MESSAGE_H

#include <Utils/FileSystem/Path.h>
#include <Utils/Types/StringAtom.h>
#include <Utils/Types/ArrayVector.h>
#include <Utils/Common/NonCopyable.h>

namespace SR_UTILS_NS {
    class SR_COMMON_DLL_API SubscriptionMessage final : SR_UTILS_NS::NonCopyable {
    public:
        SubscriptionMessage();
        ~SubscriptionMessage() override;

        void SetInt(StringAtom id, uint64_t value);
        void SetBool(StringAtom id, bool value);
        void SetString(StringAtom id, const std::string& value);
        void SetStringAtom(StringAtom id, StringAtom value);
        void SetPath(StringAtom id, const SR_UTILS_NS::Path& value);
        void SetAny(StringAtom id, const std::any& value);
        void SetAny(StringAtom id, std::any&& value);

        void Reset();

        SR_NODISCARD uint64_t GetInt(StringAtom id, const std::optional<uint64_t>& def = std::nullopt) const;
        SR_NODISCARD bool GetBool(StringAtom id, const std::optional<bool>& def = std::nullopt) const;
        SR_NODISCARD std::string GetString(StringAtom id, const std::optional<std::string>& def = std::nullopt) const;
        SR_NODISCARD StringAtom GetStringAtom(StringAtom id, const std::optional<StringAtom>& def = std::nullopt) const;
        SR_NODISCARD SR_UTILS_NS::Path GetPath(StringAtom id, const std::optional<SR_UTILS_NS::Path>& def = std::nullopt) const;
        SR_NODISCARD const SR_UTILS_NS::Path& GetPathRef(StringAtom id) const;
        SR_NODISCARD const std::any& GetAny(StringAtom id) const;

    private:
        static void PrintError(const char* format, StringAtom id);

    private:
        template<typename T> SR_NODISCARD T& GetValueRef(const StringAtom id, const std::optional<T>& def) {
            auto&& pIt = std::find_if(m_data.begin(), m_data.end(), [&id](const auto& item) { return item.id == id; });
            if (pIt != m_data.end()) {
                return pIt->template GetValueRef<T>();
            }
            if (def.has_value()) {
                return const_cast<T&>(*def);
            }
            PrintError("SubscriptionMessage::GetValueRef() : id \"{}\" not found!", id);
            static T defaultValue;
            return defaultValue;
        }

        template<typename T> SR_NODISCARD const T& GetValueRef(const StringAtom id, const std::optional<T>& def) const {
            return const_cast<SubscriptionMessage*>(this)->GetValueRef<T>(id, def);
        }

        template<typename T> void SetValue(const StringAtom id, const T& value) {
            auto&& pIt = std::find_if(m_data.begin(), m_data.end(), [&id](const auto& item) { return item.id == id; });
            if (pIt != m_data.end()) {
                pIt->template GetValueRef<T>() = value;
            }
            else {
                auto&& data = *m_data.emplace_back(Data());
                data.id = id;
                data.template GetValueRef<T>() = value;
            }
        }

        template<typename T> void SetValue(const StringAtom id, T&& value) {
            auto&& pIt = std::find_if(m_data.begin(), m_data.end(), [&id](const auto& item) { return item.id == id; });
            if (pIt != m_data.end()) {
                pIt->template GetValueRef<T>() = std::forward<T>(value);
            }
            else {
                auto&& data = *m_data.emplace_back(Data());
                data.id = id;
                data.template GetValueRef<T>() = std::forward<T>(value);
            }
        }

    private:
        struct Data {
            StringAtom id;

            bool boolValue = false;
            uint64_t intValue = 0;
            StringAtom atomValue;
            std::optional<SR_UTILS_NS::Path> pathValue;
            std::optional<std::any> anyValue;
            std::optional<std::string> strValue;

            template<typename T> SR_NODISCARD T& GetValueRef() {
                if constexpr (std::is_same_v<T, bool>) {
                    return boolValue;
                }
                else if constexpr (std::is_same_v<T, uint64_t>) {
                    return intValue;
                }
                else if constexpr (std::is_same_v<T, StringAtom>) {
                    return atomValue;
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    if (!strValue.has_value()) {
                        strValue.emplace();
                    }
                    return *strValue;
                }
                else if constexpr (std::is_same_v<T, SR_UTILS_NS::Path>) {
                    if (!pathValue.has_value()) {
                        pathValue.emplace();
                    }
                    return *pathValue;
                }
                else if constexpr (std::is_same_v<T, std::any>) {
                    if (!anyValue.has_value()) {
                        anyValue.emplace();
                    }
                    return *anyValue;
                }
                else {
                    SRHalt("Unsupported type!");
                    return T();
                }
            }
        };

        SR_HTYPES_NS::ArrayVector<Data, 16> m_data;

    };
}

#endif //SR_ENGINE_UTILS_SUBSCRIPTION_MESSAGE_H
