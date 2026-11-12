//
// Created by OpenAI Codex on 12.11.2026.
//

#ifdef SR_ANDROID
#include <Utils/Platform/AndroidLifecycle.h>
#include <Utils/Platform/AndroidEvent.h>

namespace SR_UTILS_NS::Platform {
    namespace {
        std::mutex g_lifecycleMutex;
        std::condition_variable g_windowReleased;
        AndroidWindowSnapshot g_state;
        uint64_t g_acknowledged = 0;
        bool g_rendererAttached = false;
        bool g_engineStopped = false;
    }

    void ReleaseAndroidWindow(ANativeWindow* window) {
        if (window) {
            ANativeWindow_release(window);
        }
    }

    void ResetAndroidLifecycle() {
        std::lock_guard lock(g_lifecycleMutex);
        ReleaseAndroidWindow(g_state.window);
        g_state = {};
        g_acknowledged = 0;
        g_rendererAttached = false;
        g_engineStopped = false;
    }

    void PublishAndroidLifecycle(int32_t command, ANativeWindow* window) {
        std::unique_lock lock(g_lifecycleMutex);
        if (command == APP_CMD_INIT_WINDOW || command == APP_CMD_TERM_WINDOW) {
            const auto next = command == APP_CMD_INIT_WINDOW ? window : nullptr;
            if (next) {
                ANativeWindow_acquire(next);
            }
            ReleaseAndroidWindow(g_state.window);
            g_state.window = next;
            ++g_state.revision;
        }
        if (command == APP_CMD_RESUME) {
            g_state.resumed = true;
        }
        else if (command == APP_CMD_PAUSE || command == APP_CMD_STOP) {
            g_state.resumed = false;
        }
        if (command == APP_CMD_GAINED_FOCUS) {
            g_state.focused = true;
        }
        else if (command == APP_CMD_LOST_FOCUS || command == APP_CMD_TERM_WINDOW) {
            g_state.focused = false;
        }
        if (command == APP_CMD_DESTROY) {
            g_state.destroying = true;
            g_state.resumed = false;
            g_state.focused = false;
            if (g_engineStopped) {
                ReleaseAndroidWindow(g_state.window);
                g_state.window = nullptr;
            }
        }
        if (g_rendererAttached && !g_engineStopped &&
            (command == APP_CMD_LOST_FOCUS || command == APP_CMD_PAUSE || command == APP_CMD_STOP ||
                command == APP_CMD_TERM_WINDOW || command == APP_CMD_DESTROY))
        {
            AndroidEvent reset{};
            reset.type = AndroidEvent::InputReset;
            AndroidEventQueue::Instance().PushEvent(reset);
        }
        if (command == APP_CMD_TERM_WINDOW) {
            const auto revision = g_state.revision;
            g_windowReleased.wait(lock, [revision]() {
                return g_acknowledged >= revision || !g_rendererAttached || g_engineStopped;
            });
        }
    }

    AndroidWindowSnapshot AcquireAndroidWindowSnapshot() {
        std::lock_guard lock(g_lifecycleMutex);
        auto state = g_state;
        if (state.window) {
            ANativeWindow_acquire(state.window);
        }
        return state;
    }

    void AcknowledgeAndroidWindow(uint64_t revision) {
        {
            std::lock_guard lock(g_lifecycleMutex);
            g_acknowledged = std::max(g_acknowledged, revision);
        }
        g_windowReleased.notify_all();
    }

    void SetAndroidRendererAttached(bool attached) {
        {
            std::lock_guard lock(g_lifecycleMutex);
            g_rendererAttached = attached;
        }
        g_windowReleased.notify_all();
    }

    bool TryAttachAndroidRenderer(ANativeWindow* expectedWindow) {
        std::lock_guard lock(g_lifecycleMutex);
        if (!expectedWindow || g_state.window != expectedWindow || g_state.destroying || g_engineStopped) {
            return false;
        }
        g_rendererAttached = true;
        return true;
    }

    bool IsAndroidInputAccepted() {
        std::lock_guard lock(g_lifecycleMutex);
        return g_rendererAttached && !g_engineStopped && !g_state.destroying &&
            g_state.window && g_state.resumed && g_state.focused;
    }

    bool PushAndroidInputEvent(const AndroidEvent& event) {
        std::lock_guard lock(g_lifecycleMutex);
        if (!g_rendererAttached || g_engineStopped || g_state.destroying ||
            !g_state.window || !g_state.resumed || !g_state.focused)
        {
            return false;
        }
        AndroidEventQueue::Instance().PushEvent(event);
        return true;
    }

    void NotifyAndroidEngineStopped() {
        {
            std::lock_guard lock(g_lifecycleMutex);
            g_engineStopped = true;
            g_rendererAttached = false;
            ReleaseAndroidWindow(g_state.window);
            g_state.window = nullptr;
            g_state.resumed = false;
            g_state.focused = false;
        }
        g_windowReleased.notify_all();
    }
}
#endif
