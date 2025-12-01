//
// Created by Monika on 27.05.2024.
//

#ifndef SR_ENGINE_UTILS_SUBSCRIPTION_HOLDER_H
#define SR_ENGINE_UTILS_SUBSCRIPTION_HOLDER_H

#include <Utils/Types/StringAtom.h>
#include <Utils/Types/ObjectPool.h>
#include <Utils/Types/Function.h>

namespace SR_UTILS_NS {
    class SubscriptionHolder;
    class SubscriptionMessage;

    class SR_COMMON_DLL_API SubscriptionInternalInfo : SR_UTILS_NS::NonCopyable {
    public:
        explicit SubscriptionInternalInfo(SR_HTYPES_NS::Function<void(const SubscriptionMessage&)>&& callback, SubscriptionHolder* pHolder);

        uint32_t index = SR_ID_INVALID;
        SR_HTYPES_NS::Function<void(const SubscriptionMessage&)> callback;
        SubscriptionHolder* pHolder = nullptr;
        StringAtom id;
    };

    class SR_COMMON_DLL_API Subscription final : SR_UTILS_NS::NonCopyable {
    public:
        Subscription();
        ~Subscription() override;
        explicit Subscription(SubscriptionInternalInfo* pInternalInfo);
        Subscription(Subscription&& other) noexcept;
        Subscription& operator=(Subscription&& other) noexcept;

        SR_NODISCARD bool IsValid() const noexcept { return m_internalInfo != nullptr; }

        void Reset();

    private:
        SubscriptionInternalInfo* m_internalInfo = nullptr;

    };

    class SR_COMMON_DLL_API SubscriptionHolder {
    public:
        SubscriptionHolder();
        SubscriptionHolder(SubscriptionHolder& other) = delete;
        SubscriptionHolder(SubscriptionHolder&& other) = delete;
        SubscriptionHolder& operator=(SubscriptionHolder& other) = delete;
        SubscriptionHolder& operator=(SubscriptionHolder&& other) = delete;
        virtual ~SubscriptionHolder();

        SR_NODISCARD Subscription Subscribe(StringAtom id, SR_HTYPES_NS::Function<void(const SubscriptionMessage&)>&& callback);
        SR_NODISCARD Subscription* SubscribeDynamic(StringAtom id, SR_HTYPES_NS::Function<void(const SubscriptionMessage&)>&& callback);
        SR_NODISCARD bool HasSubscriptions() const noexcept;

        void Unsubscribe(const SubscriptionInternalInfo* pSubscription);

        void Broadcast(StringAtom id);
        void Broadcast(StringAtom id, const SubscriptionMessage& message);

    private:
        std::map<StringAtom, SR_HTYPES_NS::ObjectPool<SubscriptionInternalInfo*, uint32_t>> m_subscriptions;
        uint32_t m_count = 0;

    };
}

#endif //SR_ENGINE_UTILS_SUBSCRIPTION_HOLDER_H
