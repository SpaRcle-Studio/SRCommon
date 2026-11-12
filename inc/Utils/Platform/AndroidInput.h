//
// Created by OpenAI Codex on 12.11.2026.
//

#ifndef SR_UTILS_ANDROID_INPUT_H
#define SR_UTILS_ANDROID_INPUT_H

#ifdef SR_ANDROID
#include <Utils/Platform/Platform.h>
#include <Utils/Platform/AndroidEvent.h>
#include <android/input.h>

namespace SR_UTILS_NS::Platform {
    struct AndroidTouchPoint {
        int32_t id = -1;
        SR_MATH_NS::FVector2 position;
        float_t pressure = 0.f;
    };

    SR_COMMON_DLL_API int32_t QueueAndroidInputEvent(const AInputEvent* event);
    SR_COMMON_DLL_API void ApplyAndroidInputEvent(const AndroidEvent& event);
    SR_COMMON_DLL_API void ResetAndroidInput();
    SR_COMMON_DLL_API void BeginAndroidInputFrame();
    SR_COMMON_DLL_API MouseState GetAndroidMouseState();
    SR_COMMON_DLL_API KeyboardState GetAndroidKeyboardState();
    SR_COMMON_DLL_API Vector<AndroidTouchPoint> GetAndroidTouchPoints();
}
#endif

#endif
