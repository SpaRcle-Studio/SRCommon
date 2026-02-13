//
// Created by Monika on 27.05.2024.
//

#include <Utils/Common/SubscriptionHolder.h>
#include <Utils/Common/SubscriptionMessage.h>

namespace SR_UTILS_NS {
    void SubscriptionMessage::SetInt(const StringAtom id, const uint64_t value) { SetValue(id, value); }
    void SubscriptionMessage::SetBool(const StringAtom id, const bool value) { SetValue(id, value); }
    void SubscriptionMessage::SetString(const StringAtom id, const std::string& value) { SetValue(id, value); }
    void SubscriptionMessage::SetPath(const StringAtom id, const SR_UTILS_NS::Path& value) { SetValue(id, value); }
    void SubscriptionMessage::SetAny(StringAtom id, const std::any& value) { SetValue(id, value); }
    void SubscriptionMessage::SetAny(StringAtom id, std::any&& value) { SetValue(id, std::move(value)); }

    uint64_t SubscriptionMessage::GetInt(const StringAtom id, const std::optional<uint64_t>& def) const { return GetValueRef(id, def); }
    bool SubscriptionMessage::GetBool(const StringAtom id, const std::optional<bool>& def) const { return GetValueRef(id, def); }
    std::string SubscriptionMessage::GetString(const StringAtom id, const std::optional<std::string>& def) const { return GetValueRef(id, def); }
    SR_UTILS_NS::Path SubscriptionMessage::GetPath(const StringAtom id, const std::optional<SR_UTILS_NS::Path>& def) const { return GetValueRef(id, def); }
    const SR_UTILS_NS::Path& SubscriptionMessage::GetPathRef(const StringAtom id) const { return GetValueRef(id, std::optional<SR_UTILS_NS::Path>()); }
    const std::any& SubscriptionMessage::GetAny(StringAtom id) const { return GetValueRef(id, std::optional<std::any>()); }

    void SubscriptionMessage::PrintError(const char* format, const StringAtom id) {
        SRHalt(SR_UTILS_NS::Format(format, id.c_str()));
    }

    void SubscriptionMessage::Reset() {
        m_data.clear();
    }

    SubscriptionMessage::SubscriptionMessage() = default;

    SubscriptionMessage::~SubscriptionMessage() = default;

    SubscriptionHolder::~SubscriptionHolder() {
        SRAssert2(m_count == 0, "Not all subscriptions were unsubscribed!");
        for (auto& [id, pool] : m_subscriptions) {
            pool.ForEach([](uint64_t, SubscriptionInternalInfo* pSubscription) {
                pSubscription->pHolder = nullptr;
            });
        }
    }

    Subscription SubscriptionHolder::Subscribe(const StringAtom id, SR_HTYPES_NS::Function<void(const SubscriptionMessage& msg)>&& callback) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(id);

        SRAssert(callback);
        auto& pool = m_subscriptions[id];
        auto&& pSubscription = new SubscriptionInternalInfo(std::move(callback), this);
        const auto index = pool.Add(pSubscription);
        pSubscription->index = index;
        pSubscription->id = id;
        ++m_count;
        return Subscription(pSubscription);
    }

    Subscription* SubscriptionHolder::SubscribeDynamic(const StringAtom id, SR_HTYPES_NS::Function<void(const SubscriptionMessage& msg)>&& callback) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(id);

        SRAssert(callback);
        auto& pool = m_subscriptions[id];
        auto&& pSubscription = new SubscriptionInternalInfo(std::move(callback), this);
        const auto index = pool.Add(pSubscription);
        pSubscription->index = index;
        pSubscription->id = id;
        ++m_count;
        return new Subscription(pSubscription);
    }

    void SubscriptionHolder::Broadcast(const StringAtom id) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(id);

        static SubscriptionMessage message;
        if (const auto it = m_subscriptions.find(id); it != m_subscriptions.end()) {
            it->second.ForEach([](uint32_t, auto&& pSubscription) {
                pSubscription->callback(message);
            });
        }
    }

    void SubscriptionHolder::Broadcast(const StringAtom id, const SubscriptionMessage &message) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(id);

        if (const auto it = m_subscriptions.find(id); it != m_subscriptions.end()) {
            it->second.ForEach([&message](uint32_t, auto&& pSubscription) {
                pSubscription->callback(message);
            });
        }
    }

    void SubscriptionHolder::Unsubscribe(const SubscriptionInternalInfo* pSubscription) {
        SR_TRACY_ZONE;

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

    bool SubscriptionHolder::HasSubscriptions() const noexcept {
        return m_count > 0;
    }

    SubscriptionHolder::SubscriptionHolder() = default;

    SubscriptionInternalInfo::SubscriptionInternalInfo(SR_HTYPES_NS::Function<void(const SubscriptionMessage &)>&& callback, SubscriptionHolder* pHolder)
        : SR_UTILS_NS::NonCopyable()
        , callback(std::move(callback))
        , pHolder(pHolder)
    { }
}
