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
        SR_INLINE_STATIC const SR_UTILS_NS::StringAtom EVENT_ON_SCRIPT_MODULE_RELOADED_ID = "OnScriptModuleReloaded";
        SR_INLINE_STATIC const SR_UTILS_NS::StringAtom EVENT_ON_ENGINE_UPDATE_ID = "OnEngineUpdate";
        SR_INLINE_STATIC const SR_UTILS_NS::StringAtom EVENT_ON_RENDER_SETTINGS_CHANGED_ID = "OnRenderSettingsChanged";
    }
}

#endif //SR_ENGINE_UTILS_EVENTS_BROADCASTER_H
