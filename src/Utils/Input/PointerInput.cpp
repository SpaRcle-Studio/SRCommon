//
// Created by Codex on 15.11.2026.
//

#include <Utils/Input/PointerInput.h>
#include <Utils/Input/InputSystem.h>

namespace SR_UTILS_NS {
    void PointerInput::Submit(const PointerEvent& event) {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        m_pending.emplace_back(event);
    }

    void PointerInput::CancelAll() {
        PointerEvent event;
        event.phase = PointerPhase::Reset;
        Submit(event);
    }

    void PointerInput::BeginFrame() {
        SR_TRACY_ZONE;
        std::lock_guard<std::mutex> lock(m_eventMutex);
        m_frame.clear();
        m_frame.swap(m_pending);
    }

    void PointerInput::CollectMouse() {
        auto&& input = Input::Instance();
        PointerEvent event;
        event.position = input.GetMousePos();
        Submit(event);
        for (uint32_t i = 0; i < 5; ++i) {
            event.button = static_cast<PointerButton>(i);
            if (input.GetMouseDown(static_cast<MouseCode>(i))) {
                event.phase = PointerPhase::Down;
                event.pressure = 1.f;
                Submit(event);
            }
            if (input.GetMouseUp(static_cast<MouseCode>(i))) {
                event.phase = PointerPhase::Up;
                event.pressure = 0.f;
                Submit(event);
            }
        }
        event.scroll = input.GetMouseScroll();
        if (event.scroll.x != 0.f || event.scroll.y != 0.f) {
            event.phase = PointerPhase::Scroll;
            event.button = PointerButton::None;
            Submit(event);
        }
    }
}
