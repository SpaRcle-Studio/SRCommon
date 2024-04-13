//
// Created by Monika on 17.11.2021.
//

#ifndef SR_UTILS_TIMER_H
#define SR_UTILS_TIMER_H

#include <Utils/Debug.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_HTYPES_NS {
    class SR_DLL_EXPORT Timer {
    public:
        explicit Timer(float_t updateFrequency)
            : m_updateFrequency(updateFrequency)
        { }

        explicit Timer(uint32_t countInSecond)
            : m_updateFrequency(static_cast<float_t>(60) / static_cast<float_t>(countInSecond * 60))
        { }

        Timer()
            : Timer(0.f)
        { }

    public:
        bool Update() {
            SR_TRACY_ZONE;

            if (m_deltaTime > m_updateFrequency) {
                m_frames = 0; m_deltaTime = 0;
            }

            auto&& now = std::chrono::high_resolution_clock::now();

            using ms = std::chrono::duration<double, std::milli>;

            m_deltaTime += std::chrono::duration_cast<ms>(now - m_beginFrame).count() / (double_t) CLOCKS_PER_SEC;

            m_frames++;
            m_beginFrame = now;

            return m_deltaTime > m_updateFrequency;
        }

        SR_NODISCARD float_t GetDeltaTime() const { return static_cast<float_t>(m_deltaTime); }
        SR_NODISCARD uint32_t GetFrames() const { return m_frames; }

    private:
        float_t m_updateFrequency;
        double_t m_deltaTime = 0;
        uint32_t m_frames = 0;
        TimePointType m_beginFrame = {};

    };
}

#endif //SR_UTILS_TIMER_H
