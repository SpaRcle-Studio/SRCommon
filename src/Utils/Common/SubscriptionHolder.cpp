//
// Created by Monika on 27.05.2024.
//

#include <Utils/Common/SubscriptionHolder.h>
#include <Utils/Common/SubscriptionMessage.h>
#include <Utils/Debug.h>

namespace SR_UTILS_NS {
    void SubscriptionMessage::SetInt(const StringAtom id, const uint64_t value) { m_ints[id] = value; }
    void SubscriptionMessage::SetBool(const StringAtom id, const bool value) { m_bools[id] = value; }
    void SubscriptionMessage::SetString(const StringAtom id, const std::string& value) { m_strings[id] = value; }
    void SubscriptionMessage::SetPath(const StringAtom id, const SR_UTILS_NS::Path& value) { m_paths[id] = value; }

    uint64_t SubscriptionMessage::GetInt(const StringAtom id, const std::optional<uint64_t>& def) const {
        return GetValue<uint64_t>(id, m_ints, def);
    }

    bool SubscriptionMessage::GetBool(const StringAtom id, const std::optional<bool>& def) const {
        return GetValue<bool>(id, m_bools, def);
    }

    std::string SubscriptionMessage::GetString(const StringAtom id, const std::optional<std::string>& def) const {
        return GetValue<std::string>(id, m_strings, def);
    }

    SR_UTILS_NS::Path SubscriptionMessage::GetPath(const StringAtom id, const std::optional<SR_UTILS_NS::Path>& def) const {
        return GetValue<SR_UTILS_NS::Path>(id, m_paths, def);
    }

    const SR_UTILS_NS::Path& SubscriptionMessage::GetPathRef(const StringAtom id) const {
        return GetValueRef<SR_UTILS_NS::Path>(id, m_paths);
    }

    void SubscriptionMessage::PrintError(const char* format, const StringAtom id) {
        SRHalt(SR_UTILS_NS::Format(format, id.c_str()));
    }

    SubscriptionMessage::SubscriptionMessage() = default;

    SubscriptionMessage::~SubscriptionMessage() = default;

    Subscription::~Subscription() {
        Reset();
    }

    void Subscription::Reset() {
        if (m_internalInfo) {
            if (m_internalInfo->pHolder) {
                m_internalInfo->pHolder->Unsubscribe(m_internalInfo);
            }
            m_internalInfo = nullptr;
        }
    }

    Subscription::Subscription(SubscriptionInternalInfo* pInternalInfo)
        : m_internalInfo(pInternalInfo)
    { }

    Subscription::Subscription(Subscription&& other) noexcept
        : m_internalInfo(SR_EXCHANGE(other.m_internalInfo, nullptr))
    { }

    Subscription &Subscription::operator=(Subscription &&other) noexcept {
        Reset();
        m_internalInfo = SR_EXCHANGE(other.m_internalInfo, nullptr);
        return *this;
    }

    Subscription::Subscription() = default;

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
