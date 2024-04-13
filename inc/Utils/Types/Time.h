//
// Created by Nikita on 30.12.2020.
//

#ifndef SR_ENGINE_TIME_H
#define SR_ENGINE_TIME_H

#include <Utils/Debug.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_HTYPES_NS {
    class SR_DLL_EXPORT Time : public Singleton<Time> {
        SR_REGISTER_SINGLETON(Time)
    public:
        using ClockT = std::chrono::high_resolution_clock;

    public:
        void Update() {
            SR_TRACY_ZONE;
            m_timeInfo = TimeInfo {
                ClockT::now(),
                static_cast<uint64_t>(clock())
            };
        }

        SR_NODISCARD TimePointType Now() const noexcept { return m_timeInfo.load().m_point; }
        SR_NODISCARD uint64_t Count() const noexcept { return m_timeInfo.load().m_point.time_since_epoch().count(); }
        SR_NODISCARD float_t FClock() const noexcept { return static_cast<float_t>(Count()) / CLOCKS_PER_SEC / CLOCKS_PER_SEC; }
        SR_NODISCARD clock_t Clock() const noexcept { return static_cast<clock_t>(m_timeInfo.load().m_clock); }

    private:
        struct TimeInfo {
            TimePointType m_point;
            uint64_t m_clock = 0;
        };

        std::atomic<TimeInfo> m_timeInfo;
    };
}

#endif // SR_ENGINE_TIME_H
