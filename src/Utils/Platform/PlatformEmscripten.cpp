//
// Created by Monika on 01.03.2026.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Platform/Stacktrace.h>
#include <Utils/Common/Breakpoint.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/FileSystem/VFS.h>
#ifdef SR_COMMON_ZLIB
    #include <zlib/contrib/minizip/unzip.h>
#endif
#include <Utils/Input/InputSystem.h>

#include <Utils/Input/KeyCodes.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/heap.h>
#include <filesystem>

#include "../Network/HTTP/EmscriptenHTTPClient.cpp"

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
    if (pElement.dataset.srEngineInput) { return 0; }
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

/*
 * В браузере нет argv, его роль играет query-строка адреса. Разбор делается на стороне JS,
 * потому что URLSearchParams декодирует процентные последовательности корректнее ручного парсера.
 *
 * Наружу отдаётся готовый список токенов (в том же виде, в котором они пришли бы в argv),
 * склеенный через ASCII Unit Separator (0x1F): в осмысленных значениях этот символ не
 * встречается, поэтому его можно использовать как разделитель, не экранируя данные.
 *
 * Забирается результат в два прохода: первый разбирает адрес и сообщает размер, второй копирует
 * строку в память, выделенную уже на стороне C++. Так не нужно тянуть malloc/free через
 * границу JS.
 */
EM_JS(int, SRWebPrepareCommandLineArgs, (), {
    var tokens = [];

    try {
        var params = new URLSearchParams((globalThis.location && globalThis.location.search) || "");

        params.forEach(function(value, key) {
            if (!key) {
                return;
            }

            /// Позволяем писать как `?headless`, так и `?--headless`.
            var name = key.charAt(0) === "-" ? key : "--" + key;

            /// Путь до проекта в командной строке передаётся позиционным аргументом,
            /// поэтому `?project=Game.srproject` разворачивается без имени опции.
            if (name === "--project") {
                if (value) {
                    tokens.push(value);
                }
                return;
            }

            tokens.push(name);

            /// `?headless` даёт пустое значение - это флаг, значения у него нет.
            if (value) {
                tokens.push(value);
            }
        });
    } catch (e) {
        console.error("SRWebPrepareCommandLineArgs() : failed to parse the query string: " + e);
        tokens = [];
    }

    globalThis.__srWebArgs = tokens.join(String.fromCharCode(0x1F));

    return lengthBytesUTF8(globalThis.__srWebArgs);
});

EM_JS(void, SRWebTakeCommandLineArgs, (char* pOut, int size), {
    var args = globalThis.__srWebArgs || "";
    globalThis.__srWebArgs = null;

    if (size > 0) {
        stringToUTF8(args, pOut, size + 1);
    }
});

EM_JS_DEPS(srengine_platform, "$lengthBytesUTF8,$stringToUTF8");

EM_ASYNC_JS(int, SRWebDownloadFile, (const char* urlPointer, const char* pathPointer), {
    const url = UTF8ToString(urlPointer);
    const path = UTF8ToString(pathPointer);
    const temporary = path + '.download-' + Date.now();
    const controller = new AbortController();
    const timer = setTimeout(function() { controller.abort(); }, 30000);
    let stream;
    try {
        const response = await fetch(url, {signal: controller.signal});
        if (!response.ok) throw new Error('HTTP ' + response.status);
        FS.mkdirTree(path.slice(0, path.lastIndexOf('/')) || '.');
        stream = FS.open(temporary, 'w');
        if (response.body) {
            const reader = response.body.getReader();
            for (;;) {
                const result = await reader.read();
                if (result.done) break;
                FS.write(stream, result.value, 0, result.value.length);
            }
        } else {
            const data = new Uint8Array(await response.arrayBuffer());
            FS.write(stream, data, 0, data.length);
        }
        FS.close(stream); stream = null;
        FS.rename(temporary, path);
        Module['srFlushStorage']?.();
        return 1;
    } catch (error) {
        console.error('Download failed', url, error);
        if (stream) FS.close(stream);
        try { FS.unlink(temporary); } catch (_) {}
        return 0;
    } finally { clearTimeout(timer); }
});

namespace SR_PLATFORM_NS {
    struct EmscriptenMainLoopData {
        bool(*mainLoop)(void*);
        void* pApplication;
    };

    static EmscriptenMainLoopData g_mainLoopData;
    static void(*g_shutdownCallback)() = nullptr;

    void SetWebApplicationShutdownCallback(void(*callback)()) {
        g_shutdownCallback = callback;
    }

    namespace {
        MouseState g_webMouseState;
        KeyboardState g_webKeyboardState;
        bool g_webInputInstalled = false;
        bool g_webCursorVisible = true;
        std::mutex g_webMouseDeltaMutex;
        SR_MATH_NS::FVector2 g_webAccumulatedMouseDelta;
        std::atomic<bool> g_webCursorLockActive = false;
        std::atomic<bool> g_webCursorLockRequested = false;

        /// Browser button ids are 0 - left, 1 - middle, 2 - right, while the engine expects
        /// 0 - left, 1 - right, 2 - middle.
        constexpr uint32_t WEB_MOUSE_BUTTON_REMAP[3] = { 0, 2, 1 };

        EM_BOOL WebMouseCallback(int eventType, const EmscriptenMouseEvent* pEvent, void* pUserData) {
            g_webMouseState.position = WebClientToCanvasPos(static_cast<float_t>(pEvent->clientX), static_cast<float_t>(pEvent->clientY));

            if (g_webCursorLockActive.load()) {
                std::lock_guard lock(g_webMouseDeltaMutex);
                g_webAccumulatedMouseDelta += SR_MATH_NS::FVector2(
                    static_cast<float_t>(pEvent->movementX), static_cast<float_t>(pEvent->movementY)
                );
            }

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

        EM_BOOL WebPointerLockCallback(int eventType, const EmscriptenPointerlockChangeEvent* pEvent, void* pUserData) {
            g_webCursorLockActive.store(pEvent->isActive);
            g_webCursorLockRequested.store(pEvent->isActive);
            std::lock_guard lock(g_webMouseDeltaMutex);
            g_webAccumulatedMouseDelta = {};
            return EM_FALSE;
        }

        EM_BOOL WebWheelCallback(int eventType, const EmscriptenWheelEvent* pEvent, void* pUserData) {
            double_t scale = 1.0;
            if (pEvent->deltaMode == DOM_DELTA_PIXEL) {
                scale = 0.01;
            } else if (pEvent->deltaMode == DOM_DELTA_LINE) {
                scale = 1.0 / 3.0;
            }

            SR_UTILS_NS::Input::Instance().SetMouseScroll(-pEvent->deltaX * scale, -pEvent->deltaY * scale);
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
            emscripten_set_wheel_callback_on_thread(SR_WEB_CANVAS_SELECTOR, nullptr, EM_FALSE, WebWheelCallback, targetThread);

            emscripten_set_keydown_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebKeyCallback, targetThread);
            emscripten_set_keyup_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebKeyCallback, targetThread);

            emscripten_set_blur_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, WebBlurCallback, targetThread);
            emscripten_set_pointerlockchange_callback_on_thread(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebPointerLockCallback, targetThread);
        #else
            emscripten_set_mousedown_callback(SR_WEB_CANVAS_SELECTOR, nullptr, EM_FALSE, WebMouseCallback);
            emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebMouseCallback);
            emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebMouseCallback);
            emscripten_set_wheel_callback(SR_WEB_CANVAS_SELECTOR, nullptr, EM_FALSE, WebWheelCallback);

            emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebKeyCallback);
            emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebKeyCallback);

            emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, WebBlurCallback);
            emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_FALSE, WebPointerLockCallback);
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

    Vector<WebTouchPoint> GetWebTouchPoints() {
        Vector<WebTouchPoint> points;
        const int32_t count = EM_ASM_INT({ return Math.min(32, Module['srTouches']?.length || 0); });
        for (int32_t i = 0; i < count; ++i) {
            auto& point = points.emplace_back();
            point.id = EM_ASM_INT({ return Module['srTouches'][$0].id; }, i);
            const float_t x = static_cast<float_t>(EM_ASM_DOUBLE({ return Module['srTouches'][$0].x; }, i));
            const float_t y = static_cast<float_t>(EM_ASM_DOUBLE({ return Module['srTouches'][$0].y; }, i));
            point.position = WebClientToCanvasPos(x, y);
            point.pressure = static_cast<float_t>(EM_ASM_DOUBLE({ return Module['srTouches'][$0].pressure; }, i));
        }
        return points;
    }

    std::vector<std::string> GetWebCommandLineArgs() {
        const int size = SRWebPrepareCommandLineArgs();
        if (size <= 0) {
            return {};
        }

        std::string tokens(static_cast<size_t>(size) + 1, '\0');
        SRWebTakeCommandLineArgs(tokens.data(), size);
        tokens.resize(static_cast<size_t>(size));

        std::vector<std::string> args;

        constexpr char separator = static_cast<char>(0x1F);

        size_t tokenStart = 0;

        while (tokenStart <= tokens.size()) {
            size_t tokenEnd = tokens.find(separator, tokenStart);

            if (tokenEnd == std::string::npos) {
                tokenEnd = tokens.size();
            }

            args.emplace_back(tokens, tokenStart, tokenEnd - tokenStart);

            tokenStart = tokenEnd + 1;
        }

        return args;
    }

    void MainLoopProxy() {
        char text[256];
        while (EM_ASM_INT({ return Module['srPollText']?.($0, $1, false) || 0; }, text, sizeof(text))) {
            SR_UTILS_NS::InputTextEvent event;
            event.SetText(text);
            SR_UTILS_NS::Input::Instance().AddTextEvent(std::move(event));
        }
        if (EM_ASM_INT({ return !!globalThis.srEngineFatalError; }) || !g_mainLoopData.mainLoop(g_mainLoopData.pApplication)) {
            emscripten_cancel_main_loop();
            const auto shutdown = g_shutdownCallback;
            g_shutdownCallback = nullptr;
            g_mainLoopData = {};
            if (shutdown) { shutdown(); }
            EM_ASM({ Module['srDisposeBrowser']?.(); });
            return;
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

    void AccumulateMouseDelta(const SR_MATH_NS::FVector2& delta) {
        std::lock_guard lock(g_webMouseDeltaMutex);
        g_webAccumulatedMouseDelta += delta;
    }

    SR_MATH_NS::FVector2 ConsumeAccumulatedMouseDelta() {
        std::lock_guard lock(g_webMouseDeltaMutex);
        const auto delta = g_webAccumulatedMouseDelta;
        g_webAccumulatedMouseDelta = {};
        return delta;
    }

    void SetCursorLockActive(bool active) {
        if (active) {
            if (!g_webCursorLockRequested.exchange(true)) {
                emscripten_request_pointerlock(SR_WEB_CANVAS_SELECTOR, EM_TRUE);
            }
        } else if (g_webCursorLockRequested.exchange(false)) {
            emscripten_exit_pointerlock();
        }
    }

    bool IsCursorLockActive() {
        return g_webCursorLockActive.load();
    }

    void Unzip(const SR_UTILS_NS::Path& source, const SR_UTILS_NS::Path& destination, bool replace) {
    #ifdef SR_COMMON_ZLIB
        Path physicalSource = source;
        VFS::Instance().ResolvePath(physicalSource, FileMode::Read);
        const auto archive = unzOpen64(physicalSource.c_str());
        if (!archive) { SR_ERROR("Cannot open ZIP archive {}", source); return; }
        int32_t status = unzGoToFirstFile(archive);
        std::array<uint8_t, 65536> buffer;
        while (status == UNZ_OK) {
            unz_file_info64 info{};
            if (unzGetCurrentFileInfo64(archive, &info, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK || info.size_filename > 4096) { break; }
            String name;
            name.resize(info.size_filename + 1);
            if (unzGetCurrentFileInfo64(archive, &info, name.data(), name.size(), nullptr, 0, nullptr, 0) != UNZ_OK) { break; }
            name.resize(info.size_filename);
            StringView remaining(name);
            bool valid = !remaining.empty() && remaining[0] != '/' && remaining.find('\\') == StringView::npos
                && remaining.find(':') == StringView::npos && remaining.find('\0') == StringView::npos
                && ((info.external_fa >> 16) & 0170000) != 0120000;
            while (valid && !remaining.empty()) {
                const auto separator = remaining.find('/');
                const auto component = remaining.substr(0, separator);
                valid = component != ".." && component != "." && !component.empty();
                if (separator == StringView::npos) { break; }
                remaining.remove_prefix(separator + 1);
            }
            if (!valid) { SR_ERROR("Unsupported ZIP entry {}", name); break; }
            const Path target = destination.Concat(name);
            if (name.back() == '/') {
                if (!target.CreateDirectories()) { break; }
            }
            else if (replace || !target.IsExists()) {
                if (!target.GetFolder().CreateDirectories() || unzOpenCurrentFile(archive) != UNZ_OK) { break; }
                bool success = true;
                {
                    auto file = VFS::Instance().OpenFile(target, FileMode::Write);
                    int32_t count = 0;
                    if (!file) { success = false; }
                    while (success && (count = unzReadCurrentFile(archive, buffer.data(), buffer.size())) > 0) {
                        success = file.Write(buffer.data(), count) == static_cast<uint64_t>(count);
                        emscripten_sleep(0);
                    }
                    success &= count >= 0;
                }
                success &= unzCloseCurrentFile(archive) == UNZ_OK;
                if (!success) { VFS::Instance().Delete(target.ToStringView()); break; }
            }
            status = unzGoToNextFile(archive);
        }
        unzClose(archive);
        if (status != UNZ_END_OF_LIST_OF_FILE) { SR_ERROR("ZIP extraction did not complete: {}", source); }
    #else
        SR_ERROR("ZIP extraction requires SR_COMMON_ZLIB.");
    #endif
    }

    void SetMousePos(const SR_MATH_NS::IVector2& pos) {
        /// Browsers do not allow warping the cursor, the pointer lock API would have to be used
        /// instead. Called every frame while the cursor is locked, so it must stay silent.
    }

    void OpenFile(const SR_UTILS_NS::Path& path, const std::string& args) {
        OpenWithAssociatedApp(path);
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
    SR_UTILS_NS::Vector<Path> FILE_CLIPBOARD;

    void TextToClipboard(const std::string &text) {
        CLIPBOARD_BUFFER = text;
        EM_ASM({ Module['srWriteClipboard']?.(UTF8ToString($0)); }, text.c_str());
    }

    void CopyFilesToClipboard(std::list<SR_UTILS_NS::Path> paths) {
        FILE_CLIPBOARD.clear();
        for (const auto& path : paths) { FILE_CLIPBOARD.emplace_back(path); }
    }

    void SetCurrentProcessDirectory(const SR_UTILS_NS::Path& directory) {
        if (chdir(directory.c_str()) != 0) { SR_ERROR("Failed to change browser working directory to {}", directory); }
    }

    void PasteFilesFromClipboard(const SR_UTILS_NS::Path &topath) {
        for (const auto& source : FILE_CLIPBOARD) {
            const Path target = topath.Concat(StringView(source.GetBaseNameAndExtView()));
            const auto sourceView = source.ToStringView();
            const auto targetView = target.ToStringView();
            const bool nested = source.IsDir() && targetView.starts_with(sourceView) && targetView.size() > sourceView.size()
                && targetView[sourceView.size()] == '/';
            if (target == source || nested) {
                SR_ERROR("Cannot paste a browser file into itself: {}", source);
                continue;
            }
            if (!VFS::Instance().Copy(source.ToStringView(), target.ToStringView())) {
                SR_ERROR("Failed to paste {} to {}", source, target);
            }
        }
    }

    std::string GetClipboardText() {
        const int32_t size = EM_ASM_INT({ return lengthBytesUTF8(Module['srReadClipboard']?.() || ''); });
        CLIPBOARD_BUFFER.resize(size + 1);
        EM_ASM({ stringToUTF8(Module['srReadClipboard']?.() || '', $0, $1); }, CLIPBOARD_BUFFER.data(), size + 1);
        CLIPBOARD_BUFFER.resize(size);
        return CLIPBOARD_BUFFER;
    }

    void InitializePlatform() {
        SR_PLATFORM_NS::WriteConsoleLog("Platform::InitializePlatform() : initializing Emscripten platform...\n");
        EnsureWebInputInstalled();
    }

    void DeInitializePlatform() {
        emscripten_exit_pointerlock();
        emscripten_html5_remove_all_event_listeners();
        g_webInputInstalled = false;
        g_webKeyboardState = {};
        g_webMouseState = {};
        g_webCursorLockActive = false;
        g_webCursorLockRequested = false;
        EM_ASM({ Module['srFlushStorage']?.(); });
    }

    void ClearClipboard() {
        CLIPBOARD_BUFFER.clear();
        FILE_CLIPBOARD.clear();
        EM_ASM({ Module['srWriteClipboard']?.(''); });
    }

    SR_MATH_NS::FVector2 GetMousePos() {
        return GetMouseState().position;
    }

    void Sleep(uint64_t milliseconds) {
    }

    uint64_t GetProcessUsedMemory() {
        // Browser process RSS is unavailable; report the committed WASM heap capacity.
        return emscripten_get_heap_size();
    }

    void SetThreadPriority(void *nativeHandle, ThreadPriority priority) {
        // Browser scheduling does not expose thread priorities.
    }

    void Terminate(bool isError) {
        std::exit(isError ? EXIT_FAILURE : EXIT_SUCCESS);
    }

    void OpenWithAssociatedApp(const Path &filepath) {
        Path physicalPath = filepath;
        VFS::Instance().ResolvePath(physicalPath, FileMode::Read);
        EM_ASM({ Module['srDownloadFile']?.(UTF8ToString($0)); }, physicalPath.c_str());
    }

    Path GetApplicationPath() {
        return Path();
    }

    Path GetApplicationDirectory() {
        return GetApplicationPath();
    }

    std::optional<Path> GetApplicationCachePath() {
        return Path("/Persistent/Cache");
    }

    std::optional<Path> GetApplicationDataPath() {
        return Path("/Persistent/Data");
    }

    std::optional<Path> GetApplicationLogPath() {
        return Path("/Persistent/Logs");
    }

    String GetApplicationName() {
        return "SREngine";
    }

    bool FileIsHidden(const Path &path) {
        const StringView name(path.GetBaseNameView());
        return !name.empty() && name[0] == '.';
    }

    void SelfOpen() {
        SR_ERROR("Process relaunch is unavailable in the browser. Use the page restart action.");
    }

    bool IsAbsolutePath(const StringView path) {
        return std::filesystem::path(std::string_view(path)).is_absolute();
    }

    SR_MATH_NS::UVector2 GetScreenResolution() {
        return {static_cast<uint32_t>(EM_ASM_INT({ return screen.width; })), static_cast<uint32_t>(EM_ASM_INT({ return screen.height; }))};
    }

    double_t GetScreenDPI() {
        return 96.0 * emscripten_get_device_pixel_ratio();
    }

    void SetEnvironmentVar(const std::string_view& name, const std::string_view& value) {

    }

    std::string ExecuteCommand(const std::string& command, const std::vector<std::string>& env) {
        SR_ERROR("Native command execution is unavailable in the browser.");
        return {};
    }

    bool DownloadFile(const std::string& url, const Path& outputPath) {
        Path physicalPath = outputPath;
        VFS::Instance().ResolvePath(physicalPath, FileMode::Write);
        return SRWebDownloadFile(url.c_str(), physicalPath.c_str()) != 0;
    }

    void ConfineCursor() {
        SetCursorLockActive(true);
    }

    void ReleaseCursorConfinement() {
        SetCursorLockActive(false);
    }

    void ShowWindow(void* pHandle, ShowWindowActionType action) {

    }

    PlatformType GetType() {
        return PlatformType::Emscripten;
    }

    void* LoadLibraryModule(const Path& path) {
        SR_ERROR("Native libraries cannot be loaded in the browser: {}", path);
        return nullptr;
    }

    bool UnloadLibraryModule(void* pLibrary) {
        SR_ERROR("Native library unloading is unavailable in the browser.");
        return false;
    }

    void* GetLibraryFunctionAddress(void* pLibrary, const char* pFunctionName) {
        SR_ERROR("Native library symbols are unavailable in the browser: {}", pFunctionName);
        return nullptr;
    }

    bool IsLibraryModuleLoaded(const Path& path) {
        return false;
    }

    std::vector<SR_MATH_NS::UVector2> GetScreenResolutions() {
        auto&& resolutions = std::vector<SR_MATH_NS::UVector2>();
        resolutions.emplace_back(1280, 720);
        return resolutions;
    }
}

