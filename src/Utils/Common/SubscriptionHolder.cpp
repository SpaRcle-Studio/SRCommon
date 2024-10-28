//
// Created by Monika on 27.05.2024.
//

#include <Utils/Common/SubscriptionHolder.h>

namespace SR_UTILS_NS {
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

    SubscriptionHolder::~SubscriptionHolder() {
        SRAssert2(m_count == 0, "Not all subscriptions were unsubscribed!");
        for (auto& [id, pool] : m_subscriptions) {
            pool.ForEach([](uint64_t, SubscriptionInternalInfo* pSubscription) {
                pSubscription->pHolder = nullptr;
            });
        }
    }

    Subscription SubscriptionHolder::Subscribe(const StringAtom id, SR_HTYPES_NS::Function<void(const SubscriptionMessage& msg)>&& callback) {
        SRAssert(callback);
        auto& pool = m_subscriptions[id];
        auto&& pSubscription = new SubscriptionInternalInfo(std::move(callback), this);
        const auto index = pool.Add(pSubscription);
        pSubscription->index = index;
        pSubscription->id = id;
        ++m_count;
        return Subscription(pSubscription);
    }
}
