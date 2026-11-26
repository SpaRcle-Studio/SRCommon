//
// Created by Codex on 15.11.2026.
//

#include <Utils/UI/UIInputRouter.h>
#include <Utils/UI/IUIInputTarget.h>
#include <Utils/ECS/SceneObject.h>
#include <Utils/World/Scene.h>

namespace SR_UTILS_NS {
    void UIInputRouter::Invalidate(bool targetsDirty) {
        ++m_revision;
        m_targetsDirty |= targetsDirty;
        if (targetsDirty) {
            ++m_targetsRevision;
        }
    }

    void UIInputRouter::RequestFocus(UIComponent& component) {
        m_requestedFocus = &component;
    }

    void UIInputRouter::Register(Component& component) {
        SR_TRACY_ZONE;
        if (auto&& pUI = UIComponent::TryGet(&component)) {
            m_components.emplace_back(pUI);
            Invalidate(true);
        }
    }

    void UIInputRouter::Unregister(Component& component) {
        SR_TRACY_ZONE;
        for (auto it = m_components.begin(); it != m_components.end(); ++it) {
            if (it->Get() == &component) {
                m_components.erase(it);
                Invalidate(true);
                return;
            }
        }
    }

    void UIInputRouter::Prepare(bool paused) {
        SR_TRACY_ZONE;
        for (uint32_t i = 0; i < m_components.size(); ++i) {
            const auto pUI = m_components[i];
            if (pUI && pUI->IsActive() && (!paused || pUI->ExecuteInEditMode())) {
                pUI->PrepareUIInput();
                if (pUI && bool(pUI->GetUIInputTarget()) != m_targetMembership.contains(pUI.Get())) {
                    Invalidate(true);
                }
            }
        }
    }

    void UIInputRouter::AfterLayout(bool paused) {
        SR_TRACY_ZONE;
        for (uint32_t i = 0; i < m_components.size(); ++i) {
            const auto pUI = m_components[i];
            if (pUI && pUI->IsActive() && (!paused || pUI->ExecuteInEditMode())) {
                pUI->AfterLayout();
            }
        }
    }

    void UIInputRouter::Collect(SceneObject& object) {
        if (object.IsDestroyed()) {
            return;
        }
        for (auto&& pComponent : object.GetComponents()) {
            auto&& pUI = UIComponent::TryGet(pComponent ? pComponent->BaseComponent() : nullptr);
            if (pUI && pUI->GetUIInputTarget()) {
                m_targets.emplace_back(pUI);
                m_targetMembership.insert(pUI);
                m_requiresPolling |= pUI->GetUIInputTarget()->RequiresInputPolling();
            }
        }
        for (auto&& pChild : object.GetChildrenRef()) {
            Collect(*pChild);
        }
    }

    void UIInputRouter::RefreshQueryCache() const {
        if (m_queryRevision != m_revision) {
            m_queryRevision = m_revision;
            m_enabledObjects.clear();
            m_enabledTargets.clear();
            m_availableTargets.clear();
        }
        if (m_pQueryModal != m_modal.Get()) {
            m_pQueryModal = m_modal.Get();
            m_availableTargets.clear();
        }
    }

    bool UIInputRouter::IsObjectEnabled(const SceneObject& object) const {
        if (!m_requiresPolling) {
            if (auto&& found = m_enabledObjects.find(&object); found != m_enabledObjects.end()) {
                return found->second;
            }
        }
        const bool enabled = object.IsEnabled() && !object.IsDestroyed()
            && (!object.GetParent() || IsObjectEnabled(*object.GetParent()));
        if (!m_requiresPolling) {
            m_enabledObjects.emplace(&object, enabled);
        }
        return enabled;
    }

    bool UIInputRouter::IsEnabled(const SR_HTYPES_NS::SharedPtr<UIComponent>& pComponent) const {
        if (!pComponent || !pComponent->IsComponentValid()) {
            return false;
        }
        RefreshQueryCache();
        if (!m_requiresPolling) {
            if (auto&& found = m_enabledTargets.find(pComponent.Get()); found != m_enabledTargets.end()) {
                return found->second;
            }
        }
        SR_TRACY_ZONE;
        const auto pObject = pComponent->GetSceneObject();
        const bool enabled = pComponent->IsEnabled() && (!pObject || IsObjectEnabled(*pObject));
        if (!m_requiresPolling) {
            m_enabledTargets.emplace(pComponent.Get(), enabled);
        }
        return enabled;
    }

    void UIInputRouter::RefreshModal() {
        if (!m_requiresPolling && m_modalRefreshed && m_modalRevision == m_revision) {
            m_consumed |= m_modal.Valid();
            return;
        }
        SR_TRACY_ZONE;
        m_modalRefreshed = true;
        m_modalRevision = m_revision;
        if (m_modal && !IsEnabled(m_modal)) {
            return; // Keep the closing modal's input barrier until the next frame.
        }
        for (auto&& pTarget : m_targets) {
            const bool modal = m_requiresPolling
                ? IsEnabled(pTarget) && pTarget->GetUIInputTarget()->IsModal()
                : pTarget && pTarget->IsComponentValid() && pTarget->GetUIInputTarget()->IsModal() && IsEnabled(pTarget);
            if (modal) {
                m_modal = pTarget;
            }
        }
        m_consumed |= m_modal.Valid();
    }

    bool UIInputRouter::Contains(const SR_HTYPES_NS::SharedPtr<UIComponent>& pComponent) const {
        if (!pComponent || !pComponent->IsComponentValid()) {
            return false;
        }
        RefreshQueryCache();
        if (!m_requiresPolling) {
            if (auto&& found = m_availableTargets.find(pComponent.Get()); found != m_availableTargets.end()) {
                return found->second;
            }
        }
        SR_TRACY_ZONE;
        bool available = m_targetMembership.contains(pComponent.Get()) && IsEnabled(pComponent) && pComponent->IsActive();
        if (available && m_modal) {
            available = false;
            const auto pModalObject = m_modal->GetSceneObject();
            for (auto pObject = pComponent->GetSceneObject(); pObject; pObject = pObject->GetParent()) {
                if (pObject == pModalObject) {
                    available = true;
                    break;
                }
            }
        }
        if (!m_requiresPolling) {
            m_availableTargets.emplace(pComponent.Get(), available);
        }
        return available;
    }

    SR_HTYPES_NS::SharedPtr<UIComponent> UIInputRouter::HitTest(const PointerEvent& event) const {
        if (!m_requiresPolling && m_hitValid && m_hitRevision == m_revision && m_pHitModal == m_modal.Get()
            && m_hitEvent.pointerId == event.pointerId && m_hitEvent.device == event.device
            && m_hitEvent.phase == event.phase && m_hitEvent.button == event.button && m_hitEvent.space == event.space
            && m_hitEvent.position == event.position && m_hitEvent.scroll == event.scroll
            && SR_EQUALS(m_hitEvent.pressure, event.pressure) && SR_EQUALS(m_hitEvent.dragThreshold, event.dragThreshold)
            && (!m_pHit.Get() || (m_pHit && m_pHit->IsComponentValid()))) {
            return m_pHit;
        }
        SR_TRACY_ZONE;
        const uint64_t revision = m_revision;
        SR_HTYPES_NS::SharedPtr<UIComponent> pHit;
        for (auto it = m_targets.rbegin(); it != m_targets.rend(); ++it) {
            if (Contains(*it) && (*it)->GetUIInputTarget()->HitTest(event)) {
                pHit = *it;
                break;
            }
        }
        m_pHit = pHit;
        m_hitEvent = event;
        m_hitRevision = revision;
        m_pHitModal = m_modal.Get();
        m_hitValid = !m_requiresPolling;
        return pHit;
    }

    void UIInputRouter::ResetPointers() {
        SR_TRACY_ZONE;
        for (auto&& pointer : m_pointers) {
            if (pointer.pCapture && pointer.pCapture->IsComponentValid()) {
                pointer.pCapture->GetUIInputTarget()->PointerCancel(pointer.last);
            }
        }
        m_pointers.clear();
    }

    void UIInputRouter::ProcessPointer(const PointerEvent& event) {
        SR_TRACY_ZONE;
        if (event.phase == PointerPhase::Reset) {
            ResetPointers();
            m_keyboardNavigation = false;
            m_consumed = true;
            return;
        }
        auto pHit = HitTest(event);
        if (pHit && event.phase != PointerPhase::Move) {
            m_consumed = true;
        }

        uint32_t index = 0;
        {
            SR_TRACY_ZONE_N("Calculate index");
            while (index < m_pointers.size() && (m_pointers[index].id != event.pointerId || m_pointers[index].device != event.device)) {
                ++index;
            }
        }

        if (index == m_pointers.size()) {
            if (event.phase != PointerPhase::Down && !(event.phase == PointerPhase::Move && event.device != PointerDevice::Touch)) {
                return;
            }
            auto&& pointer = m_pointers.emplace_back();
            pointer.id = event.pointerId;
            pointer.device = event.device;
        }

        auto&& pointer = m_pointers[index];
        if (event.phase == PointerPhase::Down || event.phase == PointerPhase::Scroll
            || (event.phase == PointerPhase::Move && (pointer.last.position != event.position || pointer.last.space != event.space))) {
            m_keyboardNavigation = false;
        }
        pointer.last = event;
        if (pointer.pCapture && (!Contains(pointer.pCapture) || !pointer.pCapture->GetUIInputTarget()->IsInteractable())) {
            pointer.pCapture->GetUIInputTarget()->PointerCancel(event);
            pointer.pCapture = nullptr;
            pointer.pDragCandidate = nullptr;
        }

        if (event.phase == PointerPhase::Down && !pointer.pCapture && pHit) {
            auto&& pTarget = pHit->GetUIInputTarget();
            bool alreadyCaptured = false;
            for (auto&& other : m_pointers) {
                alreadyCaptured |= other.pCapture == pHit;
            }
            if (!alreadyCaptured && pTarget->IsInteractable() && pTarget->AcceptsButton(event.button)) {
                pointer.pCapture = pHit;
                pointer.button = event.button;
                pointer.press = event;
                if (pTarget->IsFocusable()) {
                    m_focus = pHit;
                }
                if (!pTarget->CanDrag()) {
                    for (auto pParent = pHit->GetSceneObject()->GetParent(); pParent && !pointer.pDragCandidate; pParent = pParent->GetParent()) {
                        for (auto&& pComponent : pParent->GetComponents()) {
                            SR_HTYPES_NS::SharedPtr<UIComponent> pUI(UIComponent::TryGet(pComponent ? pComponent->BaseComponent() : nullptr));
                            if (Contains(pUI) && pUI->GetUIInputTarget() && pUI->GetUIInputTarget()->IsInteractable() && pUI->GetUIInputTarget()->CanDrag()) {
                                pointer.pDragCandidate = pUI;
                                break;
                            }
                        }
                    }
                }
                pTarget->PointerDown(event);
            }
        }

        if (pointer.pCapture && Contains(pointer.pCapture)) {
            m_consumed = true;
            const auto delta = event.position - pointer.press.position;
            const float_t threshold = std::max(0.f, pointer.press.dragThreshold);
            if ((event.phase == PointerPhase::Move || (event.phase == PointerPhase::Up && event.button == pointer.button))
                && Contains(pointer.pDragCandidate) && delta.x * delta.x + delta.y * delta.y >= threshold * threshold) {
                bool alreadyCaptured = false;
                for (auto&& other : m_pointers) {
                    alreadyCaptured |= other.pCapture == pointer.pDragCandidate;
                }
                pointer.pCapture->GetUIInputTarget()->PointerCancel(event);
                pointer.pCapture = !alreadyCaptured && pointer.pDragCandidate->GetUIInputTarget()->IsInteractable()
                    ? pointer.pDragCandidate : nullptr;
                pointer.pDragCandidate = nullptr;
                if (!pointer.pCapture) {
                    if (event.device == PointerDevice::Touch && event.phase == PointerPhase::Up) {
                        m_pointers.erase(m_pointers.begin() + index);
                    }
                    return;
                }
                pointer.pCapture->GetUIInputTarget()->PointerDown(pointer.press);
            }
            const auto pCaptured = pointer.pCapture;
            if (event.phase == PointerPhase::Up && event.button == pointer.button) {
                pointer.pCapture = nullptr;
                pointer.pDragCandidate = nullptr;
                pCaptured->GetUIInputTarget()->PointerDrag(event);
                if (pCaptured && pCaptured->IsComponentValid()) {
                    pCaptured->GetUIInputTarget()->PointerUp(event, pCaptured == pHit);
                }
            }
            else if (event.phase == PointerPhase::Cancel) {
                pointer.pCapture = nullptr;
                pointer.pDragCandidate = nullptr;
                pCaptured->GetUIInputTarget()->PointerCancel(event);
            }
            else if (event.phase == PointerPhase::Move) {
                pCaptured->GetUIInputTarget()->PointerDrag(event);
            }
        }

        if (event.phase == PointerPhase::Scroll && Contains(pHit)) {
            auto pParent = pHit->GetSceneObject()->GetParent();
            bool handled = pHit->GetUIInputTarget()->Scroll(event.scroll);
            while (pParent && !handled) {
                const auto pNext = pParent->GetParent();
                for (auto&& pComponent : pParent->GetComponents()) {
                    SR_HTYPES_NS::SharedPtr<UIComponent> pUI(UIComponent::TryGet(pComponent ? pComponent->BaseComponent() : nullptr));
                    if (Contains(pUI) && pUI->GetUIInputTarget() && pUI->GetUIInputTarget()->IsInteractable()) {
                        handled = pUI->GetUIInputTarget()->Scroll(event.scroll);
                        if (handled) {
                            break;
                        }
                    }
                }
                pParent = pNext;
            }
        }
        if (event.phase == PointerPhase::Cancel || (event.device == PointerDevice::Touch && event.phase == PointerPhase::Up)) {
            m_pointers.erase(m_pointers.begin() + index);
        }
    }

    void UIInputRouter::ProcessNavigation(const UINavigationState& navigation) {
        SR_TRACY_ZONE;
        if (!Contains(m_focus) || (m_focus && !m_focus->GetUIInputTarget()->IsInteractable())) {
            m_focus = nullptr;
        }
        if (navigation.cancel && m_modal) {
            m_consumed = true;
            const auto pModal = m_modal;
            pModal->GetUIInputTarget()->Cancel();
            return;
        }
        const bool tab = navigation.nextFocus || navigation.previousFocus;
        if ((tab || navigation.direction != 0) && !m_targets.empty()) {
            m_keyboardNavigation = true;
            m_consumed = true;
            const int32_t direction = navigation.previousFocus ? -1 : navigation.nextFocus ? 1 : navigation.direction < 0 ? -1 : 1;
            if (tab || !m_focus || !m_focus->GetUIInputTarget()->Navigate(direction)) {
                int32_t index = direction > 0 ? -1 : static_cast<int32_t>(m_targets.size());
                for (uint32_t i = 0; i < m_targets.size(); ++i) {
                    if (m_targets[i] == m_focus) {
                        index = static_cast<int32_t>(i);
                        break;
                    }
                }
                for (uint32_t i = 0; i < m_targets.size(); ++i) {
                    index = (index + direction + static_cast<int32_t>(m_targets.size())) % static_cast<int32_t>(m_targets.size());
                    const auto pCandidate = m_targets[index];
                    if (Contains(pCandidate) && pCandidate->GetUIInputTarget()->IsInteractable() && pCandidate->GetUIInputTarget()->IsFocusable()) {
                        m_focus = pCandidate;
                        break;
                    }
                }
            }
        }
        if (navigation.submit && Contains(m_focus)) {
            m_consumed = true;
            const auto pFocused = m_focus;
            pFocused->GetUIInputTarget()->Submit();
        }
    }

    bool UIInputRouter::HasPointerChanges(std::span<const PointerEvent> events) const {
        for (auto&& pointer : m_pointers) {
            // Preserve per-frame capture validation and stationary drag callbacks.
            if (pointer.pCapture) {
                return true;
            }
        }
        for (auto&& event : events) {
            if (event.phase != PointerPhase::Move || event.device != PointerDevice::Mouse) {
                return true;
            }
            bool unchanged = false;
            for (auto&& pointer : m_pointers) {
                auto&& last = pointer.last;
                if (pointer.id == event.pointerId && pointer.device == event.device) {
                    unchanged = last.position == event.position && last.space == event.space
                        && SR_EQUALS(last.pressure, event.pressure) && last.button == event.button
                        && last.scroll == event.scroll && SR_EQUALS(last.dragThreshold, event.dragThreshold);
                    break;
                }
            }
            if (!unchanged) {
                return true;
            }
        }
        return false;
    }

    void UIInputRouter::Process(SR_WORLD_NS::Scene& scene, std::span<const PointerEvent> events, const UINavigationState& navigation) {
        SR_TRACY_ZONE;
        if (m_processedRevision == m_revision && !m_requiresPolling && !m_requestedFocus
            && m_windowFocused == navigation.focused && !navigation.nextFocus && !navigation.previousFocus
            && !navigation.submit && !navigation.cancel && navigation.direction == 0
            && (!navigation.focused || !HasPointerChanges(events))) {
            // Consumption caused by an event belongs only to its frame.
            m_consumed = m_modal.Valid();
            return;
        }
        const uint64_t revision = m_revision;
        m_windowFocused = navigation.focused;
        m_modal = nullptr;
        m_modalRefreshed = false;
        m_consumed = false;

        const bool rebuildTargets = m_targetsDirty || m_requiresPolling;
        if (rebuildTargets) {
            SR_TRACY_ZONE_N("Collect targets");
            m_targetsDirty = false;
            m_requiresPolling = false;
            m_targets.clear();
            m_targetMembership.clear();
            for (auto&& pRoot : scene.GetRootSceneObjects()) {
                Collect(*pRoot);
            }
        }

        if (rebuildTargets) {
            SR_TRACY_ZONE_N("Sort");
            m_targetStates.clear();
            m_targetStates.resize(m_targets.size(), static_cast<uint8_t>(0xff));
            m_targetOrder.clear();
            m_targetOrder.reserve(m_targets.size());
            bool ordered = true;
            for (uint32_t i = 0; i < m_targets.size(); ++i) {
                const int32_t priority = m_targets[i]->GetUIInputTarget()->GetInputPriority();
                if (i > 0 && m_targetOrder.back().priority > priority) {
                    ordered = false;
                }
                m_targetOrder.emplace_back(TargetOrder{ priority, i });
            }
            if (!ordered) {
                // The traversal index preserves the previous stable ordering for equal priorities.
                std::sort(m_targetOrder.begin(), m_targetOrder.end(), [](const TargetOrder& left, const TargetOrder& right) {
                    return left.priority < right.priority || (left.priority == right.priority && left.index < right.index);
                });
                m_sortedTargets.reserve(m_targets.size());
                for (auto&& order : m_targetOrder) {
                    m_sortedTargets.emplace_back(std::move(m_targets[order.index]));
                }
                m_targets.swap(m_sortedTargets);
                m_sortedTargets.clear();
            }
        }

        RefreshModal();

        if (m_requestedFocus) {
            if (Contains(m_requestedFocus) && m_requestedFocus->GetUIInputTarget()->IsFocusable()) {
                m_focus = m_requestedFocus;
            }
            m_requestedFocus = nullptr;
        }

        {
            SR_TRACY_ZONE_N("Process pointer cancel");
            for (auto&& pointer : m_pointers) {
                if (pointer.pCapture && (!Contains(pointer.pCapture) || !pointer.pCapture->GetUIInputTarget()->IsInteractable())) {
                    pointer.pCapture->GetUIInputTarget()->PointerCancel(pointer.last);
                    pointer.pCapture = nullptr;
                    pointer.pDragCandidate = nullptr;
                }
                m_consumed |= pointer.pCapture.Valid();
            }
        }

        if (!navigation.focused) {
            ResetPointers();
            m_focus = nullptr;
            m_keyboardNavigation = false;
        }
        else {
            {
                SR_TRACY_ZONE_N("Process events");
                for (auto&& event : events) {
                    RefreshModal();
                    ProcessPointer(event);
                }
            }
            RefreshModal();
            ProcessNavigation(navigation);
            RefreshModal();
        }

        {
            SR_TRACY_ZONE_N("Process hover");
            for (auto&& pointer : m_pointers) {
                pointer.pHover = pointer.device != PointerDevice::Touch ? HitTest(pointer.last) : nullptr;
            }
        }

        {
            SR_TRACY_ZONE_N("Process input state");
            const bool forceStates = m_requiresPolling || m_processedRevision != revision;
            for (uint32_t i = 0; i < m_targets.size(); ++i) {
                auto&& pTarget = m_targets[i];
                if (!pTarget || !pTarget->IsComponentValid()) {
                    continue;
                }
                bool hovered = false;
                bool pressed = false;
                for (auto&& pointer : m_pointers) {
                    hovered |= pointer.pHover == pTarget;
                    pressed |= pointer.pCapture == pTarget && Contains(pTarget);
                }
                const bool focused = m_keyboardNavigation && m_focus == pTarget && Contains(pTarget);
                hovered &= navigation.focused && !m_keyboardNavigation;
                const uint8_t state = static_cast<uint8_t>((hovered ? 1 : 0) | (pressed ? 2 : 0) | (focused ? 4 : 0));
                if (forceStates || m_revision != revision || m_targetStates[i] != state) {
                    m_targetStates[i] = state;
                    pTarget->GetUIInputTarget()->SetInputState(hovered, pressed, focused);
                }
            }
        }

        // Changes made by callbacks must still wake the next frame.
        m_processedRevision = revision;
    }
}
