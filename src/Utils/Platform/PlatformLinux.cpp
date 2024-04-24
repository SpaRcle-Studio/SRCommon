//
// Created by innerviewer on 05.11.2023.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Debug.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xlib-xcb.h>

#include <xcb/randr.h>

#include <filesystem>

#include <spawn.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/sendfile.h>

namespace SR_UTILS_NS::Platform {
    void SegmentationHandler(int sig) {
        WriteConsoleError("Crash stacktrace: \n" + SR_UTILS_NS::GetStacktrace());
        Breakpoint();
        exit(1);
    }

    bool IsConsoleFocused() {
        SRHaltOnce("Not implemented!");
        return true;
    }

    void SetCursorVisible(bool isVisible) {
        SRHaltOnce("Not implemented!");
    }

    void StdHandler() {
        SegmentationHandler(1);
    }

    uint16_t GetCurrentProcessId() {
        return ::getpid();
    }

    void InitSegmentationHandler() {
        StacktraceInit();
        signal(SIGSEGV, SegmentationHandler);
        std::set_terminate(StdHandler);
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

    bool IsFileDeletable(const SR_UTILS_NS::Path& path) {
       SRHaltOnce("Not implemented!");
   }

    void OpenFile(const SR_UTILS_NS::Path& path, const std::string& args) {
        SRHaltOnce("Not implemented!");
    }

    void Unzip(const SR_UTILS_NS::Path& source, const SR_UTILS_NS::Path& destination) {
        SRHaltOnce("Not yet implemented!");
    }

    void SetInstance(void* pInstance) {
        SRHaltOnce("Not implemented!");
    }

    void SetMousePos(const SR_MATH_NS::IVector2& pos) {
        SRHaltOnce("Not implemented!");
    }

    void* GetInstance() {
        SRHaltOnce("Not implemented!");
        return nullptr;
    }

    void OpenFile(const SR_UTILS_NS::Path& path) {
        SRHaltOnce("Not implemented!");
    }

    std::optional<std::string> ReadFile(const Path& path) {
        std::ifstream ifs(path.c_str());

        if (!ifs.is_open()) {
            return { };
        }

        return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    }

    void WriteConsoleLog(const std::string& msg) {
        std::cout << msg << std::flush;
    }

    void WriteConsoleError(const std::string& msg) {
        std::cerr << msg << std::flush;;
    }

    void WriteConsoleWarn(const std::string& msg) {
        std::cerr << msg << std::flush;
    }

    bool WaitAndDelete(const SR_UTILS_NS::Path& path) {
        SRHaltOnce("Not implemented!");
        return false;
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
        SR_PLATFORM_NS::WriteConsoleLog("Platform::InitializePlatform() : initializing Linux platform...\n");
    }

    void ClearClipboard() {
        SRHaltOnce("Not implemented!");
    }

    SR_MATH_NS::FVector2 GetMousePos() {
        SRHaltOnce("Not implemented!");
        return SR_MATH_NS::FVector2();
    }

    void Sleep(uint64_t milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    uint64_t GetProcessUsedMemory() {
        std::ifstream proc_meminfo("/proc/self/status");
        uint64_t result = 0;
        if (proc_meminfo.good()) {
            std::string content((std::istreambuf_iterator<char>(proc_meminfo)), std::istreambuf_iterator<char>());

            static const auto&& getVal = [](const std::string &target, const std::string &content) {
                int result = -1;
                std::size_t start = content.find(target);
                if (start != std::string::npos) {
                    int begin = start + target.length();
                    std::size_t end = content.find("kB", start);
                    std::string substr = content.substr(begin, end - begin);
                    result = std::stoi(substr);
                }
                return result * 1024;
            };

            result  = getVal("VmRSS:", content);
        }

        return result;
    }

    void SetThreadPriority(void *nativeHandle, ThreadPriority priority) {
        SRHaltOnce("Not implemented!");
    }

    void Terminate() {
        SRHaltOnce("Not implemented!");
    }

    void OpenWithAssociatedApp(const Path &filepath) {
        SRHaltOnce("Not implemented!");
    }

    bool Copy(const Path &from, const Path &to) {
        if (from.IsFile()) {
            int source = open(from.c_str(), O_RDONLY, 0);
            int dest = open(to.c_str(), O_WRONLY | O_CREAT /*| O_TRUNC/**/, 0644);

            // struct required, rationale: function stat() exists also
            struct stat stat_source;
            fstat(source, &stat_source);

            auto&& result = sendfile(dest, source, 0, stat_source.st_size);

            close(source);
            close(dest);

            if (result == -1) {
                SR_WARN("Platform::Copy() : failed to copy!\n\tFrom: {}\n\tTo: {}", from.CStr(), to.CStr());
            }

            return result != -1;
        }

        if (!from.IsDir()) {
            SR_WARN("Platform::Copy() : \"{}\" is not directory!", from.c_str());
            return false;
        }

        CreateFolder(to.ToStringRef());

        for (auto&& item : GetInDirectory(from, Path::Type::Undefined)) {
            if (Copy(item, to.Concat(item.GetBaseNameAndExt()))) {
                continue;
            }

            return false;
        }

        return true;
    }

    std::list<Path> GetInDirectory(const Path &dir, Path::Type type) {
        std::list<Path> result;

        if (!IsExists(dir)) {
            return result;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir.ToStringRef())) {
            if ((entry.is_directory() && type == Path::Type::Folder) || (entry.is_regular_file() && type == Path::Type::File)) {
                result.emplace_back(entry.path());
            }
        }

        return result;
    }

    bool CreateFolder(const std::string& path) {
        return mkdir(path.c_str(), S_IRWXU);
    }

    bool Delete(const Path &path) {
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

        for (auto&& item : GetInDirectory(path, Path::Type::Undefined)) {
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

    Path GetApplicationPath() {
        return std::filesystem::current_path().string();
    }

    Path GetApplicationName() {
        std::string sp;
        std::ifstream("/proc/self/comm") >> sp;

        return sp;
    }

    bool FileIsHidden(const Path &path) {
        SRHaltOnce("Not implemented!");
        return false;
    }

    FileMetadata GetFileMetadata(const Path& file) {
        FileMetadata fileMetadata;
        struct stat result{};
        if (stat(file.c_str(), &result) == 0) {
            fileMetadata.lastWriteTime = result.st_mtime;
        }
        return fileMetadata;
    }

    void SelfOpen() {
        auto&& applicationPath = GetApplicationPath();

        pid_t pid;
        pid = fork();

        if (pid == 0) {
            execl(applicationPath.c_str(), nullptr);
        }
        else {
            SR_WARN("Platform::SelfOpen() : failed to create a new process.");
        }
    }

    bool IsAbsolutePath(const Path &path) {
        if (!path.empty() && path[0] == '/') {
            return true;
        }

        return false;
    }

    void OpenInNativeFileExplorer(const Path &path){
        SRHaltOnce("Not implemented!");
    }

    bool IsExists(const Path &path) {
        struct stat buffer{};
        return (stat(path.c_str(), &buffer) == 0);
    }

    std::vector<SR_MATH_NS::UVector2> GetScreenResolutions() {
        std::vector<SR_MATH_NS::UVector2> resolutions;
        if (auto&& pDisplay = XOpenDisplay(":0")) {
            XRRScreenResources *screen;
            XRRCrtcInfo *crtc_info;

            screen = XRRGetScreenResources (pDisplay, DefaultRootWindow(pDisplay));

            for (int32_t i = 0; i < ScreenCount(pDisplay); ++i) {
                crtc_info = XRRGetCrtcInfo (pDisplay, screen, screen->crtcs[i]);
                resolutions.emplace_back(crtc_info->width, crtc_info->height);
            }

            XCloseDisplay(pDisplay);
        }

        if (resolutions.empty()) {
            resolutions.emplace_back(SR_MATH_NS::UVector2(800, 800));
        }

        return resolutions;
    }

    PlatformType GetType() {
        return PlatformType::Linux;
    }
}

