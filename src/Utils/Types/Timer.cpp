//
// Created by innerviewer on 2024-03-02.
//

#include <Utils/Types/Timer.h>

#include <Utils/stdInclude.h>

namespace SR_HTYPES_NS {
    void Timer::AsyncWait(std::function<void()> callback, std::chrono::seconds duration) {
        m_callback = std::move(callback);
        m_duration = duration;

        m_thread = std::thread(&Timer::WaitAndCall, this);
    }

    void Timer::Cancel() {
        m_thread.detach(); // TODO: change to join
    }

    void Timer::WaitAndCall() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_conditionVariable.wait_for(lock, m_duration);
        m_callback();
    }
}