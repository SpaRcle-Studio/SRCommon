//
// Created by OpenAI Codex on 12.11.2026.
//

#ifndef SR_UTILS_ANDROID_LIFECYCLE_H
#define SR_UTILS_ANDROID_LIFECYCLE_H

#ifdef SR_ANDROID
#include <Utils/Platform/Platform.h>
#include <Utils/Platform/AndroidNativeAppGlue.h>
#include <Utils/Platform/AndroidEvent.h>

namespace SR_UTILS_NS::Platform {
    struct AndroidWindowSnapshot {
        ANativeWindow* window = nullptr;
        uint64_t revision = 0;
        bool resumed = false;
        bool focused = false;
        bool destroying = false;
    };

    SR_COMMON_DLL_API void PublishAndroidLifecycle(int32_t command, ANativeWindow* window);
    // The returned window owns one native reference. Release it or transfer it to AndroidWindow.
    SR_COMMON_DLL_API AndroidWindowSnapshot AcquireAndroidWindowSnapshot();
    SR_COMMON_DLL_API void ReleaseAndroidWindow(ANativeWindow* window);
    SR_COMMON_DLL_API void AcknowledgeAndroidWindow(uint64_t revision);
    SR_COMMON_DLL_API void SetAndroidRendererAttached(bool attached);
    SR_COMMON_DLL_API bool TryAttachAndroidRenderer(ANativeWindow* expectedWindow);
    SR_COMMON_DLL_API bool IsAndroidInputAccepted();
    SR_COMMON_DLL_API bool PushAndroidInputEvent(const AndroidEvent& event);
    SR_COMMON_DLL_API void NotifyAndroidEngineStopped();
    SR_COMMON_DLL_API void ResetAndroidLifecycle();
}
#endif

#endif
