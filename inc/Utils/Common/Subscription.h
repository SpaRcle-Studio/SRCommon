//
// Created by Monika on 02.12.2025.
//

#ifndef SR_ENGINE_UTILS_SUBSCRIPTION_H
#define SR_ENGINE_UTILS_SUBSCRIPTION_H

#include <Utils/Common/NonCopyable.h>

namespace SR_UTILS_NS {
    class SubscriptionInternalInfo;

    class SR_COMMON_DLL_API Subscription final : SR_UTILS_NS::NonCopyable {
    public:
        Subscription();
        ~Subscription() override;
        explicit Subscription(SubscriptionInternalInfo* pInternalInfo);
        Subscription(Subscription&& other) noexcept;
        Subscription& operator=(Subscription&& other) noexcept;

        SR_NODISCARD bool IsValid() const noexcept;

        void Reset();

    private:
        SubscriptionInternalInfo* m_internalInfo = nullptr;

    };
}

#endif //SR_ENGINE_UTILS_SUBSCRIPTION_H
