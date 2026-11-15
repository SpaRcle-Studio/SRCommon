//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_POINTER_INPUT_H
#define SR_ENGINE_UTILS_POINTER_INPUT_H

#include <Utils/Common/Singleton.h>
#include <Utils/Math/Vector2.h>
#include <Utils/Types/Vector.h>

namespace SR_UTILS_NS {
    enum class PointerDevice : uint8_t { Mouse, Touch, Pen };
    enum class PointerPhase : uint8_t { Down, Move, Up, Cancel, Scroll, Reset };
    enum class PointerButton : uint8_t { Primary, Secondary, Middle, Auxiliary1, Auxiliary2, None };
    enum class PointerSpace : uint8_t { Screen, Client };

    struct PointerEvent {
        int64_t pointerId = 0;
        PointerDevice device = PointerDevice::Mouse;
        PointerPhase phase = PointerPhase::Move;
        PointerButton button = PointerButton::None;
        PointerSpace space = PointerSpace::Screen;
        SR_MATH_NS::FVector2 position;
        SR_MATH_NS::FVector2 scroll;
        float_t pressure = 0.f;
        float_t dragThreshold = 4.f;
    };

    class SR_COMMON_DLL_API PointerInput : public Singleton<PointerInput> {
        SR_REGISTER_SINGLETON(PointerInput)
    public:
        void Submit(const PointerEvent& event);
        void CancelAll();
        void BeginFrame();
        void CollectMouse();
        SR_NODISCARD std::span<const PointerEvent> GetEvents() const noexcept {
            return { m_frame.data(), m_frame.size() };
        }

    private:
        std::mutex m_eventMutex;
        Vector<PointerEvent> m_pending;
        Vector<PointerEvent> m_frame;
    };
}

#endif // SR_ENGINE_UTILS_POINTER_INPUT_H
