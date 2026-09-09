//
// Created by Monika on 01.03.2026.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Platform/Stacktrace.h>
#include <Utils/Common/Breakpoint.h>
#include <Utils/FileSystem/FileSystem.h>

#include <Utils/Input/KeyCodes.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <filesystem>

#if defined(__EMSCRIPTEN_PTHREADS__)
    #include <pthread.h>
#endif

/// Must match EMSCRIPTEN_CANVAS_ID used by the graphics and GUI layers.
/// EM_JS bodies are stringified by the preprocessor, so the selector is repeated literally there.
#define SR_WEB_CANVAS_SELECTOR "#srengine-canvas"

EM_JS(void, SRWebClientToCanvasPos, (double clientX, double clientY, double* pOut), {
    var pElement = document.querySelector("#srengine-canvas");
    var x = clientX;
    var y = clientY;

    if (pElement) {
        var rect = pElement.getBoundingClientRect();
        x = clientX - rect.left;
        y = clientY - rect.top;

        /// The element may be visually scaled (CSS transform/zoom), so bring the coordinates
        /// back into the logical canvas space the engine actually renders in.
        if (rect.width > 0 && rect.height > 0) {
            x *= (pElement.clientWidth || rect.width) / rect.width;
            y *= (pElement.clientHeight || rect.height) / rect.height;
        }
    }

    HEAPF64[pOut >> 3] = x;
    HEAPF64[(pOut >> 3) + 1] = y;
});

EM_JS(int, SRWebIsEditableFocused, (), {
    var pElement = document.activeElement;
    if (!pElement) {
        return 0;
    }
    if (pElement.isContentEditable) {
        return 1;
    }
    var tag = (pElement.tagName || "").toUpperCase();
    return (tag === "INPUT" || tag === "TEXTAREA" || tag === "SELECT") ? 1 : 0;
});

EM_JS(void, SRWebInstallContextMenuGuard, (), {
    var pElement = document.querySelector("#srengine-canvas");
    if (pElement && !pElement.dataset.srContextMenuGuard) {
        pElement.dataset.srContextMenuGuard = "1";
        pElement.addEventListener("contextmenu", function(e) { e.preventDefault(); });
    }
});

EM_JS(void, SRWebSetCursorVisible, (int isVisible), {
    var pElement = document.querySelector("#srengine-canvas");
    if (pElement) {
        pElement.style.cursor = isVisible ? "" : "none";
    }
});

namespace SR_PLATFORM_NS {
    struct EmscriptenMainLoopData {
        bool(*mainLoop)(void*);
        void* pApplication;
    };

    static EmscriptenMainLoopData g_mainLoopData;

    namespace {
        MouseState g_webMouseState;
        KeyboardState g_webKeyboardState;
        bool g_webInputInstalled = false;
        bool g_webCursorVisible = true;

        /// Browser button ids are 0 - left, 1 - middle, 2 - right, while the engine expects
        /// 0 - left, 1 - right, 2 - middle.
        constexpr uint32_t WEB_MOUSE_BUTTON_REMAP[3] = { 0, 2, 1 };

        EM_BOOL WebMouseCallback(int eventType, const EmscriptenMouseEvent* pEvent, void* pUserData) {
            g_webMouseState.position = WebClientToCanvasPos(static_cast<float_t>(pEvent->clientX), static_cast<float_t>(pEvent->clientY));

            if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN || eventType == EMSCRIPTEN_EVENT_MOUSEUP) {
                const bool isDown = eventType == EMSCRIPTEN_EVENT_MOUSEDOWN;
                if (pEvent->button >= 0 && pEvent->button < 3) {
                    g_webMouseState.buttonStates[WEB_MOUSE_BUTTON_REMAP[pEvent->button]] = isDown;
                }
                else if (pEvent->button == 3 || pEvent->button == 4) {
                    g_webMouseState.buttonStates[pEvent->button] = isDown;
                }
            }

            /// Never swallow the event here, the GUI backend decides what to consume.
            return EM_FALSE;
        }

        EM_BOOL WebKeyCallback(int eventType, const EmscriptenKeyboardEvent* pEvent, void* pUserData) {
            if (IsWebEditableElementFocused()) {
                return EM_FALSE;
            }

            /// DOM keyCode values match the engine key codes one to one.
            if (pEvent->keyCode < 256) {
                g_webKeyboardState.Set(static_cast<SR_UTILS_NS::KeyCode>(pEvent->keyCode), eventType == EMSCRIPTEN_EVENT_KEYDOWN);
            }

            return EM_FALSE;
        }

        EM_BOOL WebBlurCallback(int eventType, const EmscriptenFocusEvent* pEvent, void* pUserData) {
            /// The browser stops delivering key events once the page loses focus, so without this
            /// the keys held at that moment would stay pressed forever.
            g_webKeyboardState = KeyboardState();
            g_webMouseState = MouseState();
            return EM_FALSE;
        }

        void EnsureWebInputInstalled() {
            if (g_webInputInstalled) {
                return;
            }
            g_webInputInstalled = true;

            /// There is no emscripten_set_contextmenu_callback, so the right mouse button has to
            /// be freed from the browser menu directly.
            SRWebInstallContextMenuGuard();

            /// Buttons may only be pressed over the canvas, but movement and release have to be
            /// tracked page-wide, otherwise a drag that leaves the canvas gets stuck.
        #if defined(__EMSCRIPTEN_PTHREADS__)
            constexpr pthread_t targetThread = EM_CALLBACK_THREAD_CONTEXT_MAIN_RUNTIME_THREAD;

            emscripten_set_mousedown_callback_on_thread(SR_WEB_CANVAS_SELECTOR, nullptr, EM_FALSE, WebMouseCallback, targetThread);
            emscripten_set_mouseup_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebMouseCallback, targetThread);
            emscripten_set_mousemove_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebMouseCallback, targetThread);

            emscripten_set_keydown_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebKeyCallback, targetThread);
            emscripten_set_keyup_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebKeyCallback, targetThread);

            emscripten_set_blur_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, WebBlurCallback, targetThread);
        #else
            emscripten_set_mousedown_callback(SR_WEB_CANVAS_SELECTOR, nullptr, EM_FALSE, WebMouseCallback);
            emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebMouseCallback);
            emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebMouseCallback);

            emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebKeyCallback);
            emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebKeyCallback);

            emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, WebBlurCallback);
        #endif
        }
    }

    SR_MATH_NS::FVector2 WebClientToCanvasPos(float_t clientX, float_t clientY) {
        double out[2] = { static_cast<double>(clientX), static_cast<double>(clientY) };
        SRWebClientToCanvasPos(out[0], out[1], out);
        return SR_MATH_NS::FVector2(static_cast<float_t>(out[0]), static_cast<float_t>(out[1]));
    }

    bool IsWebEditableElementFocused() {
        return SRWebIsEditableFocused() != 0;
    }

    void MainLoopProxy() {
        if (!g_mainLoopData.mainLoop(g_mainLoopData.pApplication)) {
            emscripten_cancel_main_loop();
        }
    }

    void SetApplicationMainLoop(bool(*mainLoop)(void*), void* pApplication) {
        g_mainLoopData.mainLoop = mainLoop;
        g_mainLoopData.pApplication = pApplication;
        emscripten_set_main_loop(MainLoopProxy, 0, true);
    }

    void InitSegmentationHandler() {

    }

    WindowProtocolType GetWindowProtocolType() {
        return WindowProtocolType::Web;
    }

    KeyboardState GetSystemKeyboardState() {
        if (auto&& overridden = GetOverriddenKeyboardState()) {
            return overridden.value();
        }
        return g_webKeyboardState;
    }

    bool IsConsoleFocused() {
        return true; /// always focused
    }

    void SetInstance(void* pInstance) {

    }

    void SetCursorVisible(bool isVisible) {
        if (g_webCursorVisible == isVisible) {
            return;
        }
        g_webCursorVisible = isVisible;
        SRWebSetCursorVisible(isVisible ? 1 : 0);
    }

    MouseState GetMouseState() {
        if (auto&& overridden = GetOverriddenMouseState()) {
            return overridden.value();
        }
        return g_webMouseState;
    }

    void Unzip(const SR_UTILS_NS::Path& source, const SR_UTILS_NS::Path& destination, bool replace) {
        SRHaltOnce("Not yet implemented!");
    }

    void SetMousePos(const SR_MATH_NS::IVector2& pos) {
        /// Browsers do not allow warping the cursor, the pointer lock API would have to be used
        /// instead. Called every frame while the cursor is locked, so it must stay silent.
    }

    void OpenFile(const SR_UTILS_NS::Path& path, const std::string& args) {
        SRHaltOnce("Not implemented!");
    }

    bool IsRunningUnderDebugger() {
        return false;
    }

    void* GetInstance() {
        return nullptr;
    }

    void WriteConsoleLog(const std::string& msg) {
        std::cout << msg << std::endl;
    }

    void WriteConsoleError(const std::string& msg) {
        std::cerr << msg << std::endl;
    }

    void WriteConsoleWarn(const std::string& msg) {
        std::cerr << msg << std::endl;
    }

    SR_UTILS_NS::String CLIPBOARD_BUFFER;

    void TextToClipboard(const std::string &text) {
        CLIPBOARD_BUFFER = text;
    }

    void CopyFilesToClipboard(std::list<SR_UTILS_NS::Path> paths) {
        SRHaltOnce("Not implemented!");
    }

    void SetCurrentProcessDirectory(const SR_UTILS_NS::Path& directory) {
        SRHaltOnce("Not implemented!");
    }

    void PasteFilesFromClipboard(const SR_UTILS_NS::Path &topath) {
        SRHaltOnce("Not implemented!");
    }

    std::string GetClipboardText() {
        return CLIPBOARD_BUFFER;
    }

    void InitializePlatform() {
        SR_PLATFORM_NS::WriteConsoleLog("Platform::InitializePlatform() : initializing Emscripten platform...\n");
        EnsureWebInputInstalled();
    }

    void DeInitializePlatform() {

    }

    void ClearClipboard() {
        CLIPBOARD_BUFFER.clear();
    }

    SR_MATH_NS::FVector2 GetMousePos() {
        return GetMouseState().position;
    }

    void Sleep(uint64_t milliseconds) {
    }

    uint64_t GetProcessUsedMemory() {
        return 0;
    }

    void SetThreadPriority(void *nativeHandle, ThreadPriority priority) {
        SRHaltOnce("Not implemented!");
    }

    void Terminate(bool isError) {
        std::exit(isError ? EXIT_FAILURE : EXIT_SUCCESS);
    }

    void OpenWithAssociatedApp(const Path &filepath) {
        SRHaltOnce("Not implemented!");
    }

    Path GetApplicationPath() {
        return Path();
    }

    Path GetApplicationDirectory() {
        return GetApplicationPath();
    }

    std::optional<Path> GetApplicationCachePath() {
        return std::nullopt;
    }


    std::optional<Path> GetApplicationLogPath() {
        return std::nullopt;
    }

    String GetApplicationName() {
        return "SREngine";
    }

    bool FileIsHidden(const Path &path) {
        SRHaltOnce("Not implemented!");
        return false;
    }

    void SelfOpen() {
        SRHaltOnce("Not suitable for web platform!");
    }

    bool IsAbsolutePath(const Path& path) {
        return std::filesystem::path(path.ToStringRef()).is_absolute();
    }

    SR_MATH_NS::UVector2 GetScreenResolution() {
        SRHaltOnce("Not implemented!");
        return SR_MATH_NS::UVector2();
    }

    double_t GetScreenDPI() {
        SRHaltOnce("Not implemented!");
        return 0.0;
    }

    void SetEnvironmentVar(const std::string_view& name, const std::string_view& value) {

    }

    std::string ExecuteCommand(const std::string& command, const std::vector<std::string>& env) {
        SRHaltOnce("Not implemented!");
        return std::string();
    }

    bool DownloadFile(const std::string& url, const Path& outputPath) {
        SRHaltOnce("Not implemented!");
        return false;
    }

    void ConfineCursor() {
        SRHaltOnce("Not implemented!");
    }

    void ReleaseCursorConfinement() {
        SRHaltOnce("Not implemented!");
    }

    void ShowWindow(void* pHandle, ShowWindowActionType action) {

    }

    PlatformType GetType() {
        return PlatformType::Emscripten;
    }

    void* LoadLibraryModule(const Path& path) {
        SRHaltOnce("Not implemented!");
        return nullptr;
    }

    bool UnloadLibraryModule(void* pLibrary) {
        SRHaltOnce("Not implemented!");
        return false;
    }

    void* GetLibraryFunctionAddress(void* pLibrary, const char* pFunctionName) {
        SRHaltOnce("Not implemented!");
        return nullptr;
    }

    bool IsLibraryModuleLoaded(const Path& path) {
        SRHaltOnce("Not implemented!");
        return false;
    }

    std::vector<SR_MATH_NS::UVector2> GetScreenResolutions() {
        auto&& resolutions = std::vector<SR_MATH_NS::UVector2>();
        resolutions.emplace_back(1280, 720);
        return resolutions;
    }
}

