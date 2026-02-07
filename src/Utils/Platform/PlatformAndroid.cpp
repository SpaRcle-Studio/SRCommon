//
// Created by Monika on 14.11.2022.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Debug.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Platform/Stacktrace.h>
#include <Utils/Common/Breakpoint.h>

#include <Utils/Platform/AndroidNativeAppGlue.h>

#include <android/log.h>
#include <android/native_activity.h>
#include <android/configuration.h>

/// чтобы компилировался PhysX под android
extern "C" uint32_t android_getCpuCount() {
    long cores = sysconf(_SC_NPROCESSORS_CONF);
    return (cores > 0) ? static_cast<uint32_t>(cores) : 1;
}

namespace SR_UTILS_NS::Platform {
    bool RemoveAssetsPrefix(std::string_view& path) {
        if (path.starts_with(":assets:")) {
            path = path.substr(8); // length of ":assets:"
            if (path.starts_with("/")) {
                path = path.substr(1);
            }
            return true;
        }
        return false;
    }

    static android_app* pAndroidInstance = nullptr;

    void InitSegmentationHandler() {

    }

    WindowProtocolType GetWindowProtocolType() {
        return WindowProtocolType::Mobile;
    }

    KeyboardState GetSystemKeyboardState() {
        return KeyboardState();
    }

    bool IsConsoleFocused() {
        return true; /// always focused
    }

    void SetInstance(void* pInstance) {
        pAndroidInstance = reinterpret_cast<android_app*>(pInstance);
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
        std::ifstream sf("/proc/self/status");
        std::string s;
        while (sf >> s)
        {
            if (s == "TracerPid:")
            {
                int pid;
                sf >> pid;
                return pid != 0;
            }
            std::getline(sf, s);
        }

        return false;
    }

    void* GetInstance() {
        return (void*)pAndroidInstance;
    }

    std::optional<std::string> ReadFile(const Path& path) {
        std::string_view pathView = path.ToStringView();
        if (!RemoveAssetsPrefix(pathView)) {
            // Открываем файл в бинарном режиме и сразу получаем размер
            std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
            if (!file) {
                return std::nullopt;
            }

            const std::streamsize size = file.tellg();

            std::string buffer;
            buffer.resize(static_cast<size_t>(size));
            file.seekg(0, std::ios::beg);
            if (!file.read(buffer.data(), size)) {
                return std::nullopt;
            }

            return buffer;
        }

        AAsset* pAsset = AAssetManager_open(
                pAndroidInstance->activity->assetManager,
                pathView.data(),
                AASSET_MODE_STREAMING
        );

        if (!pAsset) {
            SR_WARN("Platform::ReadFile() : asset not found!\n\tPath: {}", pathView);
            return std::nullopt;
        }

        const size_t len = AAsset_getLength(pAsset);
        std::string content(len, '\0');
        AAsset_read(pAsset, content.data(), len);
        AAsset_close(pAsset);

        return content;
    }

    void WriteMessage(int log, const std::string& msg) {
        const size_t maxLen = 1024;
        size_t len = msg.size();
        for (size_t i = 0; i < len; i += maxLen) {
            char buf[maxLen + 1];
            strncpy(buf, msg.data() + i, maxLen);
            buf[maxLen] = '\0';
            ((void)__android_log_print(log, "SpaRcle Engine", "%s", buf));
        }
    }

    void WriteConsoleLog(const std::string& msg) {
        WriteMessage(ANDROID_LOG_INFO, msg);
    }

    void WriteConsoleError(const std::string& msg) {
        WriteMessage(ANDROID_LOG_ERROR, msg);
    }

    void WriteConsoleWarn(const std::string& msg) {
        WriteMessage(ANDROID_LOG_WARN, msg);
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
        usleep(milliseconds * 1000);
    }

    uint64_t GetProcessUsedMemory() {
        std::ifstream status("/proc/self/status");
        if (!status.is_open())
            return 0;

        std::string key;
        uint64_t value;
        std::string unit;
        while (status >> key >> value >> unit) {
            if (key == "VmRSS:") {
                return value * 1024; // перевод из килобайт в байты
            }
        }

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

        std::string_view pathToView = to.ToStringView();
        if (RemoveAssetsPrefix(pathToView)) {
            SR_WARN("Platform::Copy() : can't write asset file!");
            return false;
        }

        if (from.IsFile()) {
            auto content = SR_PLATFORM_NS::ReadFile(from);
            if (!content.has_value()) {
                SR_WARN("Platform::Copy() : failed to read file {}", from.CStr());
                return false;
            }

            std::ofstream out(to.CStr(), std::ios::binary);
            if (!out.is_open()) {
                SR_WARN("Platform::Copy() : failed to open destination file {}", to.CStr());
                return false;
            }

            out.write(content->data(), static_cast<std::streamsize>(content->size()));
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
        return {};
    }

    bool CreateFolder(const std::string& path) {
        if (path.empty()) {
            SR_WARN("Platform::CreateFolder() : path is empty!");
            return false;
        }

        std::string current;
        std::stringstream ss(path);
        std::string segment;

        // разбиваем путь по '/'
        while (std::getline(ss, segment, '/')) {
            if (segment.empty())
                continue;

            current += "/";
            current += segment;

            if (mkdir(current.c_str(), 0777) != 0) {
                if (errno == EEXIST) {
                    continue; // уже есть, норм
                }
                else {
                    SR_WARN("Platform::CreateFolder() : failed to create folder!\n\tPath: {}", current);
                    return false;
                }
            }
        }

        return true;
    }

    bool Delete(const Path& path) {
        if (path.IsFile()) {
            const bool result = std::remove(path.CStr()) == 0;

            if (!result) {
                SR_WARN("Platform::Delete() : failed to delete file!\n\tPath: {}", path.CStr());
            }

            return result;
        }

        if (!path.IsDir()) {
            return false;
        }

        for (auto&& item : GetAllInDirectory(path)) {
            if (Delete(item)) {
                continue;
            }

            return false;
        }

        const bool result = rmdir(path.CStr()) == 0;

        if (!result) {
            SR_WARN("Platform::Delete() : failed to delete folder!\n\tPath: {}", path.CStr());
        }

        return result;
    }

    Path GetApplicationResourcesPath() {
        return ":assets:";
    }

    void InitializeHooks(const std::function<void(PlatformHooks& hooks)>& callback) {

    }

    Path::Type GetPathType(std::string_view path) {
        if (RemoveAssetsPrefix(path)) {
            if (path.empty()) {
                return Path::Type::Folder; // root of assets
            }

            // пробуем открыть как файл
            if (AAsset* asset = AAssetManager_open(pAndroidInstance->activity->assetManager, path.data(), AASSET_MODE_UNKNOWN)) {
                AAsset_close(asset);
                return Path::Type::File;
            }

            // пробуем открыть как директорию
            if (AAssetDir* dir = AAssetManager_openDir(pAndroidInstance->activity->assetManager, path.data())) {
                const char* name = AAssetDir_getNextFileName(dir);
                AAssetDir_close(dir);

                // если там что-то есть, значит это директория
                if (name) {
                    return Path::Type::Folder;
                }
            }

            return Path::Type::Undefined;
        }

        struct stat st{};
        if (stat(path.data(), &st) != 0) {
            return Path::Type::Undefined;
        }

        if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
            return Path::Type::File;
        }
        else if (S_ISDIR(st.st_mode)) {
            return Path::Type::Folder;
        }
        return Path::Type::Undefined;
    }

    Path GetApplicationPath() {
        /// У операционной системы Android нет папки с приложением,
        /// работа идет с упакованными ресурсами в assets
        return Path();
    }

    Path GetApplicationDirectory() {
        return GetApplicationPath();
    }

    std::optional<Path> GetApplicationCachePath() {
        return Path(pAndroidInstance->activity->internalDataPath);
    }


    std::optional<Path> GetApplicationLogPath() {
        return Path(pAndroidInstance->activity->externalDataPath);
    }

    Path GetApplicationName() {
        return "SREngine";
    }

    bool FileIsHidden(const Path &path) {
        SRHaltOnce("Not implemented!");
        return false;
    }

    void SelfOpen() {
        SRHaltOnce("Not implemented!");
    }

    bool IsAbsolutePath(const Path& path) {
        if (path.ToStringView().starts_with(":assets:")) {
            return true;
        }

        if (path.ToStringView().starts_with(pAndroidInstance->activity->internalDataPath)) {
            return true;
        }

        return false;
    }

    SR_MATH_NS::UVector2 GetScreenResolution() {
        SRHaltOnce("Not implemented!");
        return SR_MATH_NS::UVector2();
    }

    double_t GetScreenDPI() {
        SRHaltOnce("Not implemented!");
        return 0.0;
    }

    bool IsExists(const Path &path) {
        if (AAsset* asset = AAssetManager_open(pAndroidInstance->activity->assetManager, path.c_str(), AASSET_MODE_STREAMING)) {
            AAsset_close(asset);
            return true;
        }
        return false;
    }

    void SetEnvironmentVar(const std::string_view& name, const std::string_view& value) {
        setenv(name.data(), value.data(), 1);
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

    PlatformType GetType() {
        return PlatformType::Android;
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
        if (!pAndroidInstance) {
            SR_ERROR("PlatformAndroid::GetScreenResolutions() : pAndroidInstance is nullptr!");
            return { SR_MATH_NS::UVector2(400, 400) };
        }

        if (!pAndroidInstance->window) {
            SR_ERROR("PlatformAndroid::GetScreenResolutions() : ANativeWindow is nullptr!");
            return { SR_MATH_NS::UVector2(400, 400) };
        }

        auto&& resolutions = std::vector<SR_MATH_NS::UVector2>();

        resolutions.emplace_back(SR_MATH_NS::UVector2(
                static_cast<uint32_t>(ANativeWindow_getWidth(pAndroidInstance->window)),
                static_cast<uint32_t>(ANativeWindow_getHeight(pAndroidInstance->window))
        ));

        return resolutions;
    }
}

