//
// Created by Monika on 01.03.2026.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Platform/Stacktrace.h>
#include <Utils/Common/Breakpoint.h>
#include <Utils/FileSystem/FileSystem.h>

#include <emscripten/emscripten.h>
#include <filesystem>

namespace SR_PLATFORM_NS {
    struct EmscriptenMainLoopData {
        bool(*mainLoop)(void*);
        void* pApplication;
    };

    static EmscriptenMainLoopData g_mainLoopData;

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
        return KeyboardState();
    }

    bool IsConsoleFocused() {
        return true; /// always focused
    }

    void SetInstance(void* pInstance) {

    }

    void SetCursorVisible(bool isVisible) {

    }

    FileMetadata GetFileMetadata(const Path& file) {
        FileMetadata fileMetadata;
        return fileMetadata;
    }

    MouseState GetMouseState() {
        MouseState state;
        return state;
    }

    std::list<Path> GetAllInDirectory(const Path& dir) {
        std::list<Path> result;
        return result;
    }

    void Unzip(const SR_UTILS_NS::Path& source, const SR_UTILS_NS::Path& destination, bool replace) {
        SRHaltOnce("Not yet implemented!");
    }

    void SetMousePos(const SR_MATH_NS::IVector2& pos) {

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

    void TextToClipboard(const std::string &text) {
        SRHaltOnce("Not implemented!");
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
        SRHaltOnce("Not implemented!");
        return std::string();
    }

    void InitializePlatform() {
        SR_PLATFORM_NS::WriteConsoleLog("Platform::InitializePlatform() : initializing Android platform...\n");
    }

    void ClearClipboard() {
        SRHaltOnce("Not implemented!");
    }

    SR_MATH_NS::FVector2 GetMousePos() {
        return SR_MATH_NS::FVector2();
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
        SRHaltOnce("Not implemented!");
    }

    void OpenWithAssociatedApp(const Path &filepath) {
        SRHaltOnce("Not implemented!");
    }

    bool Copy(const Path& from, const Path& to) {
        SR_TRACY_ZONE;
        if (from.IsFile()) {
            SR_UTILS_NS::String buffer;
            if (!SR_UTILS_NS::FileSystem::ReadFile(from, buffer)) {
                SR_WARN("Platform::Copy() : failed to read file {}", from.CStr());
                return false;
            }

            std::ofstream out(to.CStr(), std::ios::binary);
            if (!out.is_open()) {
                SR_WARN("Platform::Copy() : failed to open destination file {}", to.CStr());
                return false;
            }

            out.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            return out.good();
        }

        if (!from.IsDir()) {
            SR_WARN("Platform::Copy() : \"{}\" is not directory!", from.CStr());
            return false;
        }

        to.Create();

        for (auto&& item : GetInDirectory(from, Path::Type::Undefined)) {
            if (Copy(item, to.Concat(item.GetBaseNameAndExt())))
                continue;

            return false;
        }

        return true;
    }

    std::list<Path> GetInDirectory(const Path &dir, Path::Type type) {
        SRHaltOnce("Not implemented!");
        return {};
    }

    bool CreateFolder(const std::string& path) {
        return std::filesystem::create_directories(path);
    }

    bool Delete(const Path& path) {
        SRHaltOnce("Not implemented!");
        return false;
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

    Path GetApplicationName() {
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

