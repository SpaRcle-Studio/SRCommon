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

    class SubscriptionInternalInfo : SR_UTILS_NS::NonCopyable {
    public:
        explicit SubscriptionInternalInfo(SR_HTYPES_NS::Function<void()>&& callback, SubscriptionHolder* pHolder)
            : SR_UTILS_NS::NonCopyable()
            , callback(std::move(callback))
            , pHolder(pHolder)
        { }

        uint32_t index = SR_ID_INVALID;
        SR_HTYPES_NS::Function<void()> callback = nullptr;
        SubscriptionHolder* pHolder = nullptr;
        StringAtom id;
    };

    class Subscription : SR_UTILS_NS::NonCopyable {
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

        Subscription Subscribe(const StringAtom id, SR_HTYPES_NS::Function<void()>&& callback) {
            SRAssert(callback);
            auto& pool = m_subscriptions[id];
            auto&& pSubsciprtionInfo = new SubscriptionInternalInfo(std::move(callback), this);
            const auto index = pool.Add(pSubsciprtionInfo);
            pSubsciprtionInfo->index = index;
            pSubsciprtionInfo->id = id;
            ++m_count;
            return Subscription(pSubsciprtionInfo);
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
                SRHalt("SubscriptionHolder::Unsubscribe() : Subscription not found!");
            }
        }

        void Broadcast(const StringAtom id) {
            if (const auto it = m_subscriptions.find(id); it != m_subscriptions.end()) {
                it->second.ForEach([](uint32_t, auto&& pSubscription) {
                    pSubscription->callback();
                });
            }
        }

    private:
        std::map<StringAtom, SR_HTYPES_NS::ObjectPool<SubscriptionInternalInfo*, uint32_t>> m_subscriptions;
        uint32_t m_count = 0;

    };
}

#endif //SR_ENGINE_UTILS_SUBSCRIPTION_HOLDER_H
