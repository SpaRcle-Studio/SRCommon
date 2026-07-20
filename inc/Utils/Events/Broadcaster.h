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
        SR_INLINE_STATIC const SR_UTILS_NS::StringAtom EVENT_ON_PREPARE_FRAME = "OnPrepareFrame";
        SR_INLINE_STATIC const SR_UTILS_NS::StringAtom EVENT_ON_RESOURCE_RELOADED_ID = "OnResourceReloaded";
        SR_INLINE_STATIC const SR_UTILS_NS::StringAtom EVENT_ON_FONT_RELOADED_ID = "OnFontReloaded";
        SR_INLINE_STATIC const SR_UTILS_NS::StringAtom EVENT_DO_INSPECT_ENTITY_ID = "DoInspectEntity";
        SR_INLINE_STATIC const SR_UTILS_NS::StringAtom EVENT_ON_COMMAND_UNDO_ID = "OnCommandUndo";
        SR_INLINE_STATIC const SR_UTILS_NS::StringAtom EVENT_ON_COMMAND_REDO_ID = "OnCommandRedo";
    }
}

#endif //SR_ENGINE_UTILS_EVENTS_BROADCASTER_H
