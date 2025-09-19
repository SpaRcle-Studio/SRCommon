//
// Created by Monika on 19.09.2025.
//

#include <Utils/Types/Thread.h>
#include <Utils/Platform/AndroidEvent.h>

namespace SR_UTILS_NS {
    void AndroidEventQueue::PushEvent(const AndroidEvent& event) {
        SR_LOCK_GUARD;
        constexpr uint32_t MAX_EVENTS = 2048;

        m_events.push_back(event);
        if (m_events.size() > MAX_EVENTS) {
            m_events.pop_front();
        }
    }

    bool AndroidEventQueue::PopEvent(AndroidEvent& event) {
        SR_LOCK_GUARD;
        if (m_events.empty()) {
            return false;
        }
        event = m_events.front();
        m_events.pop_front();
        return true;
    }
}