//
// Created by Nikita on 30.12.2020.
//

#ifndef SR_ENGINE_TIME_H
#define SR_ENGINE_TIME_H

#include <Utils/Debug.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_HTYPES_NS {
    class Time : public Singleton<Time> {
        SR_REGISTER_SINGLETON(Time)
    public:
        using ClockT = std::chrono::high_resolution_clock;

    public:
        ~Time() override = default;

        void Update();

        void SetDeltaTime(float_t dt) { m_deltaTime = dt; }
        void SetFixedDeltaTime(float_t dt) { m_fixedDeltaTime = dt; }

        SR_NODISCARD float_t FixedDeltaTime() const noexcept { return m_fixedDeltaTime; }
        SR_NODISCARD float_t DeltaTime() const noexcept { return m_deltaTime; }
        SR_NODISCARD TimePointType Now() const noexcept { return m_timeInfo.load().m_point; }
        SR_NODISCARD uint64_t Count() const noexcept { return m_timeInfo.load().m_point.time_since_epoch().count(); }
        SR_NODISCARD float_t FClock() const noexcept { return static_cast<float_t>(Count()) / SR_CLOCKS_PER_SEC / SR_CLOCKS_PER_SEC; }

#ifdef SR_LINUX
        SR_NODISCARD uint64_t Clock() const noexcept { return static_cast<uint64_t>(m_timeInfo.load().m_clock) / SR_CLOCKS_PER_SEC; }
#else
        SR_NODISCARD uint64_t Clock() const noexcept { return static_cast<uint64_t>(m_timeInfo.load().m_clock); }
#endif

    private:
        float_t m_deltaTime = 0.f;
        float_t m_fixedDeltaTime = 0.f;

        struct TimeInfo {
            TimeInfo() = default;

            TimeInfo(TimePointType point, uint64_t clock)
                : m_point(point)
                , m_clock(clock)
            { }

            TimeInfo(const TimeInfo& other) = default;
            TimeInfo& operator=(const TimeInfo& other) = default;

            TimePointType m_point = TimePointType();
            uint64_t m_clock = 0;
        };

        std::atomic<TimeInfo> m_timeInfo = TimeInfo();
    };
}

#endif // SR_ENGINE_TIME_H
