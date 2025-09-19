//
// Created by Monika on 19.09.2025.
//

#ifndef SR_ENGINE_COMMON_ANDROID_EVENT_H
#define SR_ENGINE_COMMON_ANDROID_EVENT_H

#include <Utils/Common/Singleton.h>

#include <android/sensor.h>

namespace SR_UTILS_NS {
    struct AndroidEvent {
        enum Type {
            Motion,
            Key,
            Lifecycle,
            Window,
            Sensor
        } type;

        union {
            struct { int32_t action; float x, y; int32_t pointerId; } motion;
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
        std::list<AndroidEvent> m_events;

    };
}

#endif //SR_ENGINE_COMMON_ANDROID_EVENT_H
