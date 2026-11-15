//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_UI_COMPONENT_H
#define SR_ENGINE_UTILS_UI_COMPONENT_H

#include <Utils/ECS/Component.h>

namespace SR_UTILS_NS {
    class IUIInputTarget;

    /// @abstract @hidden
    class SR_COMMON_DLL_API UIComponent : public Component {
        SR_CLASS()
    public:
        static UIComponent* TryGet(Component* pComponent) noexcept;

        virtual void PrepareUIInput();
        virtual void AfterLayout();
        SR_NODISCARD virtual IUIInputTarget* GetUIInputTarget() noexcept {
            return nullptr;
        }
    };
}

#endif // SR_ENGINE_UTILS_UI_COMPONENT_H
