//
// Created by Monika on 19.09.2025.
//

#include <Utils/Types/Thread.h>
#include <Utils/Platform/AndroidEvent.h>

namespace SR_UTILS_NS {
    void AndroidEventQueue::PushEvent(const AndroidEvent& event) {
        SR_LOCK_GUARD;
        if (m_count == Capacity) {
            m_head = 0;
            m_count = 1;
            m_events[0] = AndroidEvent{};
            m_events[0].type = AndroidEvent::InputReset;
        }
        m_events[(m_head + m_count) % Capacity] = event;
        ++m_count;
    }

    bool AndroidEventQueue::PopEvent(AndroidEvent& event) {
        SR_LOCK_GUARD;
        if (m_count == 0) {
            return false;
        }
        event = m_events[m_head];
        m_head = (m_head + 1) % Capacity;
        --m_count;
        return true;
    }
}
