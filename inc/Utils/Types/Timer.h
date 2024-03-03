//
// Created by Monika on 17.11.2021.
//

#ifndef SR_UTILS_TIMER_H
#define SR_UTILS_TIMER_H

#include <Utils/Debug.h>
#include <asio.hpp>

namespace SR_HTYPES_NS {
    class Timer {
    public:
        ~Timer() {
            if (m_thread.joinable()) {
                m_thread.join();
            }
        }
    public:
        void AsyncWait(std::function<void()> callback, std::chrono::seconds duration);
        void Cancel();

    private:
        void WaitAndCall();
    private:
        std::chrono::seconds m_duration;
        std::function<void()> m_callback;
        std::mutex m_mutex;
        std::thread m_thread;
        std::condition_variable m_conditionVariable;
    };
}

#endif //SR_UTILS_TIMER_H
