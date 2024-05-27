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
}
