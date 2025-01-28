//
// Created by innerviewer on 05.11.2023.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Debug.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
//#include <X11/extensions/XInput.h>
#include <X11/Xlib-xcb.h>

#include <xcb/randr.h>

#include <filesystem>

#include <spawn.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <Utils/Platform/XKeySymToKeyCode.h>

#include <sys/sendfile.h>
#include <X11/extensions/Xfixes.h>

namespace SR_PLATFORM_NS {
    static Display* gLinuxPlatformDisplayPtr = nullptr;

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
        if (!gLinuxPlatformDisplayPtr) {
            gLinuxPlatformDisplayPtr = XOpenDisplay(nullptr);
        }

        if (!gLinuxPlatformDisplayPtr) {
            SR_ERROR("Platform::SetMousePos() : failed to open display.");
            return;
        }

        Window root = DefaultRootWindow(gLinuxPlatformDisplayPtr);

        if (isVisible) {
            XFixesShowCursor(gLinuxPlatformDisplayPtr, root);
        }
        else {
            XFixesHideCursor(gLinuxPlatformDisplayPtr, root);
        }
    }

    void StdHandler() {
        SegmentationHandler(1);
    }

    uint16_t GetCurrentProcessId() {
        return ::getpid();
    }

    void InitSegmentationHandler() {
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
        if (!path.Exists() || !path.IsFile()) {
            SR_WARN("Platform::CanBeDeleted() : path does not exist or is not a file.");
            return false;
        }

        auto&& file = std::fstream(path.c_str(), std::ios::in);
        if (file.is_open()) {
            file.close();
            return true;
        }

        return false;
   }

    void OpenFile(const SR_UTILS_NS::Path& path, const std::string& args) {
        std::string command;
        if (path.IsAbs()) {
            command = path.ToStringRef() + " " + args;
        }
        else {
            command = "./" + path.ToStringRef() + " " + args;
        }

        system(command.c_str());
    }

    void Unzip(const SR_UTILS_NS::Path& source, const SR_UTILS_NS::Path& destination, bool replace) {
        std::string command;
        if (source.GetExtensionView() == "zip") {
            if (replace) {
                command = "unzip -q -o " + source.ToStringRef() + " -d " + destination.ToStringRef();
            }
            else {
                command = "unzip -q -n" + source.ToStringRef() + " -d " + destination.ToStringRef();
            }

            system(command.c_str());
        }
        else if (source.GetExtensionView() == "tar" || source.GetExtensionView() == "gz") {
            //TODO: Find a way to use 'replace' variable here.
            command += "tar -xf " + source.ToStringRef() + " -C " + destination.ToStringRef();
            system(command.c_str());
        }
        else {
            SR_WARN("Platform::Unzip() : unknown file extension. Path: '{}'", source.ToString());
        }
    }

    void CopyPermissions(const SR_UTILS_NS::Path& source, const SR_UTILS_NS::Path& destination) {
        if (!source.Exists() || !destination.Exists()) {
            SR_ERROR("Platform::CopyPermissions() : either source or destination path does not exist.");
            return;
        }

        auto&& currentHandle = open(source.c_str(), O_RDONLY);
        auto&& destinationHandle = open(destination.c_str(), O_RDONLY);

        if (currentHandle == -1 || destinationHandle == -1) {
            SR_ERROR("Platform::CopyPermissions() : failed to open file handles.");
            return;
        }

        struct stat fst;
        fstat(currentHandle, &fst);
        fchown(destinationHandle, fst.st_uid, fst.st_gid);
        fchmod(destinationHandle,fst.st_mode);

        close(currentHandle);
        close(destinationHandle);
    }

    void* GetInstance() {
        SRHaltOnce("Not implemented!");
        return nullptr;
    }

    void SetInstance(void* pInstance) {
        SRHaltOnce("Not implemented!");
    }

    void SetMousePos(const SR_MATH_NS::IVector2& pos) {
        if (!gLinuxPlatformDisplayPtr) {
            gLinuxPlatformDisplayPtr = XOpenDisplay(nullptr);
        }

        if (!gLinuxPlatformDisplayPtr) {
            SR_ERROR("Platform::SetMousePos() : failed to open display.");
            return;
        }

        Window root = DefaultRootWindow(gLinuxPlatformDisplayPtr);
        XSelectInput(gLinuxPlatformDisplayPtr, root, KeyReleaseMask);

        XWarpPointer(gLinuxPlatformDisplayPtr, None, root, 0, 0, 0, 0, pos.x, pos.y);

        XFlush(gLinuxPlatformDisplayPtr);
    }

    void OpenFile(const SR_UTILS_NS::Path& path) {
        std::string command;

        if (path.IsAbs()) {
            command = path.ToStringRef();
        }
        else {
            command = "./" + path.ToStringRef();
        }

        std::system(command.c_str());
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
        if (!path.IsFile()) {
            SR_WARN("Platform::WaitAndDelete() : path is not a file. Path: '{}'", path.ToString());
            return false;
        }

        SR_LOG("Platform::WaitAndDelete() : waiting for file to be deleted...");
        while (true) {
            if (IsFileDeletable(path)) {
                return Delete(path);
            }
        }
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

    MouseState GetMouseState() {
        if (!gLinuxPlatformDisplayPtr) {
            gLinuxPlatformDisplayPtr = XOpenDisplay(nullptr);
        }

        if (!gLinuxPlatformDisplayPtr) {
            SR_ERROR("Platform::GetMousePos() : failed to open display.");
            return { };
        }

        Window root = DefaultRootWindow(gLinuxPlatformDisplayPtr);
        Window root_return, child_return;
        int root_x_return, root_y_return, win_x_return, win_y_return;
        unsigned int mask_return;

        XQueryPointer(gLinuxPlatformDisplayPtr, root, &root_return, &child_return, &root_x_return, &root_y_return, &win_x_return, &win_y_return, &mask_return);

        MouseState mouseState;
        mouseState.position = { static_cast<float>(root_x_return), static_cast<float>(root_y_return) };

        mouseState.buttonStates[0] = mask_return & Button1Mask;
        mouseState.buttonStates[1] = mask_return & Button3Mask;
        mouseState.buttonStates[2] = mask_return & Button2Mask;
        mouseState.buttonStates[3] = mask_return & Button4Mask;
        mouseState.buttonStates[4] = mask_return & Button5Mask;

        return mouseState;
    }

    SR_MATH_NS::FVector2 GetMousePos() {
        if (!gLinuxPlatformDisplayPtr) {
            gLinuxPlatformDisplayPtr = XOpenDisplay(nullptr);
        }

        if (!gLinuxPlatformDisplayPtr) {
            SR_ERROR("Platform::GetMousePos() : failed to open display.");
            return { };
        }

        Window root = DefaultRootWindow(gLinuxPlatformDisplayPtr);
        Window child;
        int root_x, root_y, win_x, win_y;
        unsigned int mask;
        XQueryPointer(gLinuxPlatformDisplayPtr, root, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);

        return { static_cast<float>(root_x), static_cast<float>(root_y) };
    }

    bool GetSystemKeyboardState(uint8_t* pKeyCodes) {
        if (!gLinuxPlatformDisplayPtr) {
            gLinuxPlatformDisplayPtr = XOpenDisplay(nullptr);
        }

        if (!gLinuxPlatformDisplayPtr) {
            SR_ERROR("Platform::GetMousePos() : failed to open display.");
            return false;
        }

        char keys_return[32];
        XQueryKeymap(gLinuxPlatformDisplayPtr, keys_return);

        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 8; j++) {
                if (keys_return[i] & (1 << j)) {
                    int keycode = i * 8 + j;
                    KeySym keysym = XKeycodeToKeysym(gLinuxPlatformDisplayPtr, keycode, 0);

                    auto it = keysymToIndex.find(keysym);
                    if (it != keysymToIndex.end()) {
                        if (pKeyCodes[it->second] == 0) { // Is State::Dowm?
                            pKeyCodes[it->second] = 1; // Then set State::Pressed
                        }
                        else if (pKeyCodes[it->second] == 1) { // Is State::UnPressed?
                            pKeyCodes[it->second] = 2; // Then set State::Down
                        }
                    }
                }
            }
        }

        return true;
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
        SR_PLATFORM_NS::WriteConsoleError("Function \"Terminate\" has been called... >_<\n" + SR_UTILS_NS::GetStacktrace());
        SR_UTILS_NS::Breakpoint();
        std::terminate(); // TODO: std::terminate() or abort()?
    }

    void OpenWithAssociatedApp(const Path &filepath) {
        SRHaltOnce("Not implemented!");
    }

    bool Copy(const Path &from, const Path &to) {
        if (from.IsFile()) {
            /*/// TODO: Find another way to copy a file without using system() function WHILE preserving the current permissions.
            std::string command = "cp " + from.ToStringRef() + " " + to.ToStringRef();
            system(command.c_str());
            return true;*/

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

            CopyPermissions(from, to);

            return result != -1;
        }

        if (!from.IsDir()) {
            SR_WARN("Platform::Copy() : \"{}\" is not a directory!", from.c_str());
            return false;
        }

        CreateFolder(to.ToStringRef());

        for (auto&& item : from.GetAll()) {
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

    std::list<Path> GetAllInDirectory(const Path& dir) {
        std::list<Path> result;

        if (!IsExists(dir)) {
            return result;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir.ToStringRef())) {
            if (entry.is_directory() || entry.is_regular_file()) {
                result.emplace_back(entry.path());
            }
        }

        return result;
    }

    bool CreateFolder(const std::string& path) {
        if (path.empty()) {
            SR_WARN("Platform::CreateFolder() : path is empty!");
            return false;
        }

        const std::string command = "mkdir -p " + path;
        if (system(command.c_str()) != 0) {
            SR_WARN("Platform::CreateFolder() : failed to create folder!\n\tPath: {}", path);
            return false;
        }

        return true;
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

    Path GetApplicationPath() {
        return std::filesystem::canonical("/proc/self/exe").string();
        //return std::filesystem::current_path().string();
    }

    Path GetApplicationDirectory() {
        return GetApplicationPath().GetFolder();
    }

    Path GetApplicationName() {
        std::string sp;
        std::ifstream("/proc/self/comm") >> sp;

        return sp;
    }

    bool FileIsHidden(const Path &path) {
        return path.GetBaseNameView()[0] == '.';
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

    SR_MATH_NS::UVector2 GetScreenResolution() {
        SRHaltOnce("Not implemented!");
        return SR_MATH_NS::UVector2();
    }

    double_t GetScreenDPI() {
        SRHaltOnce("Not implemented!");
        return 0.0;
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

