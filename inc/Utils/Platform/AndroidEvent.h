//
// Created by Monika on 19.09.2025.
//

#ifndef SR_ENGINE_COMMON_ANDROID_EVENT_H
#define SR_ENGINE_COMMON_ANDROID_EVENT_H

#include <Utils/Common/Singleton.h>

#include <android/sensor.h>
#include <android/native_window.h>

namespace SR_UTILS_NS {
    struct AndroidEvent {
        enum Type {
            Motion,
            Key,
            Lifecycle,
            Window,
            Sensor,
            InputReset
        } type;

        union {
            struct {
                int32_t action;
                float_t x, y;
                int32_t pointerId;
                int32_t source;
                int32_t buttons;
                float_t scrollX, scrollY;
                float_t pressure;
            } motion;
            struct { int32_t keyCode; int32_t action; } key;
            struct { int32_t command; } lifecycle;
            struct { ANativeWindow* window; } windowEvent;
            struct { ASensorEvent sensor; } sensorEvent;
        };
    };

    class AndroidEventQueue : public Singleton<AndroidEventQueue> {
        SR_REGISTER_SINGLETON(AndroidEventQueue);
    public:
        void PushEvent(const AndroidEvent &event);
        bool PopEvent(AndroidEvent &event);

    private:
        static constexpr uint32_t Capacity = 2048;
        std::array<AndroidEvent, Capacity> m_events;
        uint32_t m_head = 0;
        uint32_t m_count = 0;

    };
}

#endif //SR_ENGINE_COMMON_ANDROID_EVENT_H
