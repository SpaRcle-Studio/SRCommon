//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_UI_INPUT_ROUTER_H
#define SR_ENGINE_UTILS_UI_INPUT_ROUTER_H

#include <Utils/UI/UIComponent.h>
#include <Utils/UI/UIInputAdapter.h>

namespace SR_WORLD_NS {
    class Scene;
}

namespace SR_UTILS_NS {
    class SR_COMMON_DLL_API UIInputRouter {
    public:
        void Register(Component& component);
        void Unregister(Component& component);
        void Prepare(bool paused);
        void AfterLayout(bool paused);
        void Process(SR_WORLD_NS::Scene& scene, std::span<const PointerEvent> events, const UINavigationState& navigation);
        SR_NODISCARD bool IsConsumed() const noexcept { return m_consumed; }
        void RequestFocus(UIComponent& component);

    private:
        void Collect(SceneObject& object);
        bool Contains(const SR_HTYPES_NS::SharedPtr<UIComponent>& pComponent) const;
        bool IsEnabled(const SR_HTYPES_NS::SharedPtr<UIComponent>& pComponent) const;
        void RefreshModal();
        SR_HTYPES_NS::SharedPtr<UIComponent> HitTest(const PointerEvent& event) const;
        void ProcessPointer(const PointerEvent& event);
        void ProcessNavigation(const UINavigationState& navigation);
        void ResetPointers();

        struct PointerState {
            int64_t id = 0;
            PointerDevice device = PointerDevice::Mouse;
            PointerButton button = PointerButton::None;
            PointerEvent press;
            PointerEvent last;
            SR_HTYPES_NS::SharedPtr<UIComponent> pCapture;
            SR_HTYPES_NS::SharedPtr<UIComponent> pDragCandidate;
            SR_HTYPES_NS::SharedPtr<UIComponent> pHover;
        };

        Vector<SR_HTYPES_NS::SharedPtr<UIComponent>> m_components;
        Vector<SR_HTYPES_NS::SharedPtr<UIComponent>> m_targets;
        Vector<PointerState> m_pointers;
        SR_HTYPES_NS::SharedPtr<UIComponent> m_focus;
        SR_HTYPES_NS::SharedPtr<UIComponent> m_requestedFocus;
        SR_HTYPES_NS::SharedPtr<UIComponent> m_modal;
        bool m_consumed = false;
    };
}

#endif // SR_ENGINE_UTILS_UI_INPUT_ROUTER_H
