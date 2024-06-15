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
            m_internalInfo->pHolder->Unsubscribe(m_internalInfo);
            m_internalInfo = nullptr;
        }
    }

    SubscriptionHolder::~SubscriptionHolder() {
        SRAssert(m_count == 0);
    }

    Subscription SubscriptionHolder::Subscribe(const StringAtom id, SR_HTYPES_NS::Function<void()> &&callback) {
        SRAssert(callback);
        auto& pool = m_subscriptions[id];
        auto&& pSubsciprtionInfo = new SubscriptionInternalInfo(std::move(callback), this);
        const auto index = pool.Add(pSubsciprtionInfo);
        pSubsciprtionInfo->index = index;
        pSubsciprtionInfo->id = id;
        ++m_count;
        return Subscription(pSubsciprtionInfo);
    }
}
