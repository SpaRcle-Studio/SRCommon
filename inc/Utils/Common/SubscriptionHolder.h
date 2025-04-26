//
// Created by Monika on 27.05.2024.
//

#ifndef SR_ENGINE_UTILS_SUBSCRIPTION_HOLDER_H
#define SR_ENGINE_UTILS_SUBSCRIPTION_HOLDER_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/Types/StringAtom.h>
#include <Utils/Types/ObjectPool.h>
#include <Utils/Types/Function.h>

namespace SR_UTILS_NS {
    class SubscriptionHolder;

    class SubscriptionMessage final : SR_UTILS_NS::NonCopyable {
    public:
        void SetInt(const StringAtom id, const uint64_t value) { m_ints[id] = value; }
        void SetBool(const StringAtom id, const bool value) { m_bools[id] = value; }
        void SetString(const StringAtom id, const std::string& value) { m_strings[id] = value; }
        void SetPath(const StringAtom id, const SR_UTILS_NS::Path& value) { m_paths[id] = value; }

        SR_NODISCARD uint64_t GetInt(const StringAtom id, const std::optional<uint64_t>& def = std::nullopt) const {
            return GetValue<uint64_t>(id, m_ints, def);
        }

        SR_NODISCARD bool GetBool(const StringAtom id, const std::optional<bool>& def = std::nullopt) const {
            return GetValue<bool>(id, m_bools, def);
        }

        SR_NODISCARD std::string GetString(const StringAtom id, const std::optional<std::string>& def = std::nullopt) const {
            return GetValue<std::string>(id, m_strings, def);
        }

        SR_NODISCARD SR_UTILS_NS::Path GetPath(const StringAtom id, const std::optional<SR_UTILS_NS::Path>& def = std::nullopt) const {
            return GetValue<SR_UTILS_NS::Path>(id, m_paths, def);
        }

        SR_NODISCARD const SR_UTILS_NS::Path& GetPathRef(const StringAtom id) const {
            return GetValueRef<SR_UTILS_NS::Path>(id, m_paths);
        }

    private:
        template<typename T, typename Container> SR_NODISCARD T GetValue(const StringAtom id, const Container& container, const std::optional<T> def) const {
            if (const auto it = container.find(id); it != container.end()) {
                return it->second;
            }
            if (def.has_value()) {
                return def.value();
            }
            SRHalt("SubscriptionMessage::GetValue() : id \"{}\" not found!", id);
            return T();
        }

        template<typename T, typename Container> SR_NODISCARD const T& GetValueRef(const StringAtom id, const Container& container) const {
            if (const auto it = container.find(id); it != container.end()) {
                return it->second;
            }
            SRHalt("SubscriptionMessage::GetValueRef() : id \"{}\" not found!", id);
            static T defaultValue;
            return defaultValue;
        }

    private:
        std::map<StringAtom, uint64_t> m_ints;
        std::map<StringAtom, bool> m_bools;
        std::map<StringAtom, std::string> m_strings;
        std::map<StringAtom, SR_UTILS_NS::Path> m_paths;

    };

    class SubscriptionInternalInfo : SR_UTILS_NS::NonCopyable {
    public:
        explicit SubscriptionInternalInfo(SR_HTYPES_NS::Function<void(const SubscriptionMessage&)>&& callback, SubscriptionHolder* pHolder)
            : SR_UTILS_NS::NonCopyable()
            , callback(std::move(callback))
            , pHolder(pHolder)
        { }

        uint32_t index = SR_ID_INVALID;
        SR_HTYPES_NS::Function<void(const SubscriptionMessage&)> callback;
        SubscriptionHolder* pHolder = nullptr;
        StringAtom id;
    };

    class Subscription final : SR_UTILS_NS::NonCopyable {
    public:
        Subscription() = default;
        ~Subscription() override;

        explicit Subscription(SubscriptionInternalInfo* pInternalInfo)
            : m_internalInfo(pInternalInfo)
        { }

        Subscription(Subscription&& other) noexcept
            : m_internalInfo(SR_EXCHANGE(other.m_internalInfo, nullptr))
        { }

        Subscription& operator=(Subscription&& other) noexcept {
            Reset();
            m_internalInfo = SR_EXCHANGE(other.m_internalInfo, nullptr);
            return *this;
        }

        void Reset();

    private:
        SubscriptionInternalInfo* m_internalInfo = nullptr;

    };

    class SubscriptionHolder {
    public:
        virtual ~SubscriptionHolder();

        SR_NODISCARD Subscription Subscribe(StringAtom id, SR_HTYPES_NS::Function<void(const SubscriptionMessage&)>&& callback);

        SR_NODISCARD bool HasSubscriptions() const noexcept {
            return m_count > 0;
        }

        void Unsubscribe(const SubscriptionInternalInfo* pSubscription) {
            if (auto it = m_subscriptions.find(pSubscription->id); it != m_subscriptions.end()) {
                auto& pool = it->second;
                pool.RemoveByIndex(pSubscription->index);
                delete pSubscription;
                SRAssert(m_count > 0);
                --m_count;
            }
            else {
                SRHalt("SubscriptionHolder::Unsubscribe() : subscription not found!");
            }
        }

        void Broadcast(const StringAtom id) {
            static SubscriptionMessage message;
            if (const auto it = m_subscriptions.find(id); it != m_subscriptions.end()) {
                it->second.ForEach([](uint32_t, auto&& pSubscription) {
                    pSubscription->callback(message);
                });
            }
        }

        void Broadcast(const StringAtom id, const SubscriptionMessage& message) {
            if (const auto it = m_subscriptions.find(id); it != m_subscriptions.end()) {
                it->second.ForEach([&message](uint32_t, auto&& pSubscription) {
                    pSubscription->callback(message);
                });
            }
        }

    private:
        std::map<StringAtom, SR_HTYPES_NS::ObjectPool<SubscriptionInternalInfo*, uint32_t>> m_subscriptions;
        uint32_t m_count = 0;

    };
}

#endif //SR_ENGINE_UTILS_SUBSCRIPTION_HOLDER_H
