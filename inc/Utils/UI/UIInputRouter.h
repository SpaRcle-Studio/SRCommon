//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_UI_INPUT_ROUTER_H
#define SR_ENGINE_UTILS_UI_INPUT_ROUTER_H

#include <Utils/UI/UIComponent.h>
#include <Utils/UI/UIInputAdapter.h>
#include <Utils/Types/FlatHashMap.h>

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
        void Invalidate(bool targetsDirty = false);
        SR_NODISCARD uint64_t GetTargetsRevision() const noexcept { return m_targetsRevision; }
        SR_NODISCARD uint64_t GetRevision() const noexcept { return m_revision; }

    private:
        void Collect(SceneObject& object);
        bool Contains(const SR_HTYPES_NS::SharedPtr<UIComponent>& pComponent) const;
        bool IsEnabled(const SR_HTYPES_NS::SharedPtr<UIComponent>& pComponent) const;
        bool IsObjectEnabled(const SceneObject& object) const;
        void RefreshQueryCache() const;
        void RefreshModal();
        SR_HTYPES_NS::SharedPtr<UIComponent> HitTest(const PointerEvent& event) const;
        void ProcessPointer(const PointerEvent& event);
        void ProcessNavigation(const UINavigationState& navigation);
        void ResetPointers();
        bool HasPointerChanges(std::span<const PointerEvent> events) const;

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

        struct TargetOrder {
            int32_t priority = 0;
            uint32_t index = 0;
        };

        Vector<SR_HTYPES_NS::SharedPtr<UIComponent>> m_components;
        Vector<SR_HTYPES_NS::SharedPtr<UIComponent>> m_targets;
        Vector<SR_HTYPES_NS::SharedPtr<UIComponent>> m_sortedTargets;
        Vector<TargetOrder> m_targetOrder;
        Vector<uint8_t> m_targetStates;
        SR_HTYPES_NS::FlatHashSet<const UIComponent*> m_targetMembership;
        Vector<PointerState> m_pointers;
        SR_HTYPES_NS::SharedPtr<UIComponent> m_focus;
        SR_HTYPES_NS::SharedPtr<UIComponent> m_requestedFocus;
        SR_HTYPES_NS::SharedPtr<UIComponent> m_modal;
        bool m_consumed = false;
        bool m_targetsDirty = true;
        bool m_requiresPolling = false;
        bool m_windowFocused = false;
        bool m_keyboardNavigation = false;
        uint64_t m_revision = 1;
        uint64_t m_processedRevision = 0;
        uint64_t m_targetsRevision = 1;
        uint64_t m_modalRevision = 0;
        bool m_modalRefreshed = false;
        mutable uint64_t m_queryRevision = 0;
        mutable const UIComponent* m_pQueryModal = nullptr;
        mutable SR_HTYPES_NS::FlatHashMap<const SceneObject*, bool> m_enabledObjects;
        mutable SR_HTYPES_NS::FlatHashMap<const UIComponent*, bool> m_enabledTargets;
        mutable SR_HTYPES_NS::FlatHashMap<const UIComponent*, bool> m_availableTargets;
        mutable bool m_hitValid = false;
        mutable uint64_t m_hitRevision = 0;
        mutable const UIComponent* m_pHitModal = nullptr;
        mutable PointerEvent m_hitEvent;
        mutable SR_HTYPES_NS::SharedPtr<UIComponent> m_pHit;
    };
}

#endif // SR_ENGINE_UTILS_UI_INPUT_ROUTER_H
