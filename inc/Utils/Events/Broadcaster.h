//
// Created by Monika on 04.05.2025.
//

#ifndef SR_ENGINE_UTILS_EVENTS_BROADCASTER_H
#define SR_ENGINE_UTILS_EVENTS_BROADCASTER_H

#include <Utils/Common/Singleton.h>
#include <Utils/Common/SubscriptionHolder.h>

namespace SR_UTILS_NS {
    class Broadcaster : public Singleton<Broadcaster>, public SR_UTILS_NS::SubscriptionHolder {
        SR_REGISTER_SINGLETON(Broadcaster);
    public:

    };

    namespace Events {
        static inline const SR_UTILS_NS::StringAtom EVENT_ON_SCRIPT_MODULE_RELOADED_ID = "OnScriptModuleReloaded";
        static inline const SR_UTILS_NS::StringAtom EVENT_ON_ENGINE_UPDATE_ID = "OnEngineUpdate";
    }
}

#endif //SR_ENGINE_UTILS_EVENTS_BROADCASTER_H
