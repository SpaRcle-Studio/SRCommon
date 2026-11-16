//
// Created by Codex on 15.11.2026.
//

#include <Utils/UI/UIInputRouter.h>
#include <Utils/UI/IUIInputTarget.h>
#include <Utils/ECS/SceneObject.h>
#include <Utils/World/Scene.h>

namespace SR_UTILS_NS {
    void UIInputRouter::RequestFocus(UIComponent& component) {
        m_requestedFocus = &component;
    }

    void UIInputRouter::Register(Component& component) {
        if (auto&& pUI = UIComponent::TryGet(&component)) {
            m_components.emplace_back(pUI);
        }
    }

    void UIInputRouter::Unregister(Component& component) {
        for (auto it = m_components.begin(); it != m_components.end(); ++it) {
            if (it->Get() == &component) {
                m_components.erase(it);
                return;
            }
        }
    }

    void UIInputRouter::Prepare(bool paused) {
        for (uint32_t i = 0; i < m_components.size(); ++i) {
            const auto pUI = m_components[i];
            if (pUI && pUI->IsActive() && (!paused || pUI->ExecuteInEditMode())) {
                pUI->PrepareUIInput();
            }
        }
    }

    void UIInputRouter::AfterLayout(bool paused) {
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
            }
        }
        for (auto&& pChild : object.GetChildrenRef()) {
            Collect(*pChild);
        }
    }

    bool UIInputRouter::IsEnabled(const SR_HTYPES_NS::SharedPtr<UIComponent>& pComponent) const {
        if (!pComponent || !pComponent->IsEnabled() || !pComponent->IsComponentValid()) {
            return false;
        }
        for (auto pObject = pComponent->GetSceneObject(); pObject; pObject = pObject->GetParent()) {
            if (!pObject->IsEnabled() || pObject->IsDestroyed()) {
                return false;
            }
        }
        return true;
    }

    void UIInputRouter::RefreshModal() {
        if (m_modal && !IsEnabled(m_modal)) {
            return; // Keep the closing modal's input barrier until the next frame.
        }
        for (auto&& pTarget : m_targets) {
            if (IsEnabled(pTarget) && pTarget->GetUIInputTarget()->IsModal()) {
                m_modal = pTarget;
            }
        }
        m_consumed |= m_modal.Valid();
    }

    bool UIInputRouter::Contains(const SR_HTYPES_NS::SharedPtr<UIComponent>& pComponent) const {
        if (!IsEnabled(pComponent) || !pComponent->IsActive()) {
            return false;
        }
        if (m_targets.find(pComponent) == m_targets.end()) {
            return false;
        }
        if (!m_modal) {
            return true;
        }
        const auto pModalObject = m_modal->GetSceneObject();
        for (auto pObject = pComponent->GetSceneObject(); pObject; pObject = pObject->GetParent()) {
            if (pObject == pModalObject) {
                return true;
            }
        }
        return false;
    }

    SR_HTYPES_NS::SharedPtr<UIComponent> UIInputRouter::HitTest(const PointerEvent& event) const {
        for (auto it = m_targets.rbegin(); it != m_targets.rend(); ++it) {
            if (Contains(*it) && (*it)->GetUIInputTarget()->HitTest(event)) {
                return *it;
            }
        }
        return nullptr;
    }

    void UIInputRouter::ResetPointers() {
        for (auto&& pointer : m_pointers) {
            if (pointer.pCapture && pointer.pCapture->IsComponentValid()) {
                pointer.pCapture->GetUIInputTarget()->PointerCancel(pointer.last);
            }
        }
        m_pointers.clear();
    }

    void UIInputRouter::ProcessPointer(const PointerEvent& event) {
        if (event.phase == PointerPhase::Reset) {
            ResetPointers();
            m_consumed = true;
            return;
        }
        auto pHit = HitTest(event);
        if (pHit && event.phase != PointerPhase::Move) {
            m_consumed = true;
        }

        uint32_t index = 0;
        while (index < m_pointers.size() && (m_pointers[index].id != event.pointerId || m_pointers[index].device != event.device)) {
            ++index;
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

    void UIInputRouter::Process(SR_WORLD_NS::Scene& scene, std::span<const PointerEvent> events, const UINavigationState& navigation) {
        SR_TRACY_ZONE;
        m_targets.clear();
        m_modal = nullptr;
        m_consumed = false;
        for (auto&& pRoot : scene.GetRootSceneObjects()) {
            Collect(*pRoot);
        }

        // Stable insertion sort retains sibling order without a temporary allocation.
        for (uint32_t i = 1; i < m_targets.size(); ++i) {
            const auto pTarget = m_targets[i];
            const auto priority = pTarget->GetUIInputTarget()->GetInputPriority();
            uint32_t j = i;
            while (j > 0 && m_targets[j - 1]->GetUIInputTarget()->GetInputPriority() > priority) {
                m_targets[j] = m_targets[j - 1];
                --j;
            }
            m_targets[j] = pTarget;
        }
        RefreshModal();
        if (m_requestedFocus) {
            if (Contains(m_requestedFocus) && m_requestedFocus->GetUIInputTarget()->IsFocusable()) {
                m_focus = m_requestedFocus;
            }
            m_requestedFocus = nullptr;
        }
        for (auto&& pointer : m_pointers) {
            if (pointer.pCapture && (!Contains(pointer.pCapture) || !pointer.pCapture->GetUIInputTarget()->IsInteractable())) {
                pointer.pCapture->GetUIInputTarget()->PointerCancel(pointer.last);
                pointer.pCapture = nullptr;
                pointer.pDragCandidate = nullptr;
            }
            m_consumed |= pointer.pCapture.Valid();
        }
        if (!navigation.focused) {
            ResetPointers();
            m_focus = nullptr;
        }
        else {
            for (auto&& event : events) {
                RefreshModal();
                ProcessPointer(event);
            }
            RefreshModal();
            ProcessNavigation(navigation);
            RefreshModal();
        }

        for (auto&& pointer : m_pointers) {
            pointer.pHover = pointer.device != PointerDevice::Touch ? HitTest(pointer.last) : nullptr;
        }

        for (auto&& pTarget : m_targets) {
            if (!pTarget || !pTarget->IsComponentValid()) {
                continue;
            }
            bool hovered = false;
            bool pressed = false;
            for (auto&& pointer : m_pointers) {
                hovered |= pointer.pHover == pTarget;
                pressed |= pointer.pCapture == pTarget && Contains(pTarget);
            }
            pTarget->GetUIInputTarget()->SetInputState(navigation.focused && hovered, pressed, Contains(pTarget) && m_focus == pTarget);
        }
        m_targets.clear();
    }
}
