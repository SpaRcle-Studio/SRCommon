//
// Created by Monika on 02.12.2025.
//

#include <Utils/Common/Subscription.h>
#include <Utils/Common/SubscriptionHolder.h>

namespace SR_UTILS_NS {
    Subscription::Subscription() = default;

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

    Subscription& Subscription::operator=(Subscription &&other) noexcept {
        Reset();
        m_internalInfo = SR_EXCHANGE(other.m_internalInfo, nullptr);
        return *this;
    }

    bool Subscription::IsValid() const noexcept {
        return m_internalInfo != nullptr;
    }
}