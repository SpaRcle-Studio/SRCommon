//
// Created by Monika on 17.03.2022.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Platform/Stacktrace.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Common/Breakpoint.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Debug.h>
#include <Utils/Common/StringAtomLiterals.h>
#include <Utils/Profile/TracyContext.h>

#include <Windows.h>
#include <Psapi.h>
#include <rpc.h>
#include <tchar.h>
#include <shellapi.h>
#include <commdlg.h>
#include <shlobj.h>
#include <ImageHlp.h>
#include <csignal>
#include <sddl.h>
#include <wininet.h>
#include <shobjidl.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tlhelp32.h>

#include <filesystem>

#ifdef SR_MINGW
    #include <ShObjIdl.h>
#endif

#include <Utils/Platform/WindowsMemoryAllocationHooks.h>

namespace SR_UTILS_NS::Platform {
    std::wstring ConvertToUnicode(const std::string& str) {
        UINT codePage = CP_ACP;
        DWORD flags = 0;
        int resultSize = MultiByteToWideChar
                (codePage     // CodePage
                        , flags        // dwFlags
                        , str.c_str()  // lpMultiByteStr
                        , str.length() // cbMultiByte
                        , NULL         // lpWideCharStr
                        , 0            // cchWideChar
                );
        std::vector<wchar_t> result(resultSize + 1);
        MultiByteToWideChar
                (codePage     // CodePage
                        , flags        // dwFlags
                        , str.c_str()  // lpMultiByteStr
                        , str.length() // cbMultiByte
                        , &result[0]   // lpWideCharStr
                        , resultSize   // cchWideChar
                );
        return &result[0];
    }

    void WriteConsoleLog(const std::string& msg) {
        std::lock_guard lock(g_platformLogMutex);
        std::cout << msg << std::flush;
    }

    void WriteConsoleError(const std::string& msg) {
        std::lock_guard lock(g_platformLogMutex);
        std::cerr << msg << std::flush;
    }

    void WriteConsoleWarn(const std::string& msg) {
        std::lock_guard lock(g_platformLogMutex);
        std::cout << msg << std::flush;
    }

    std::string GetLastErrorAsString()
    {
        //Get the error message ID, if any.
        DWORD errorMessageID = ::GetLastError();
        if(errorMessageID == 0) {
            return std::string(); //No error message has been recorded
        }
        LPSTR messageBuffer = nullptr;
        //Ask Win32 to give us the string version of that message ID.
        //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, errorMessageID, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        //Copy the error message into a std::string.
        std::string message(messageBuffer, size - 3);
        //Free the Win32's string's buffer.
        LocalFree(messageBuffer);
        return message;
    }

    void PrintErrorMessage(HRESULT hr) {
        LPVOID msgBuffer;
        DWORD dw = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, hr, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, NULL
        );

        if (dw) {
            SR_ERROR("PlatformWindows::PrintErrorMessage() : " + std::string((char*)msgBuffer));
            LocalFree(msgBuffer);
        }
        else {
            SR_ERROR("PlatformWindows::PrintErrorMessage() : unknown error code: " + std::to_string(hr));
        }
    }

    std::string ErrorCodeToString(const DWORD a_error_code)
    {
        // Get the last windows error message.
        char msg_buf[1025] = { 0 };
        // Get the error message for our os code.
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                          0,
                          a_error_code,
                          0,
                          msg_buf,
                          sizeof(msg_buf) - 1,
                          0))
        {
            // Remove trailing newline character.
            char* nl_ptr = 0;
            if (0 != (nl_ptr = strchr(msg_buf, '\n')))
            {
                *nl_ptr = '\0';
            }
            if (0 != (nl_ptr = strchr(msg_buf, '\r')))
            {
                *nl_ptr = '\0';
            }
            return std::string(msg_buf);
        }
        return std::string("Failed to get error message");
    }
}

namespace SR_UTILS_NS::Platform {
    void SegmentationHandler(int sig) {
        WriteConsoleError("Application crashed!\n" + SR_UTILS_NS::GetStacktrace());
        Breakpoint();
        exit(1);
    }

    void StdHandler() {
        SegmentationHandler(1);
    }

    uint16_t GetCurrentProcessId() {
        return ::GetCurrentProcessId();
    }

    void InitSegmentationHandler() {
        signal(SIGSEGV, SegmentationHandler);
        std::set_terminate(StdHandler);
    }

    WindowProtocolType GetWindowProtocolType() {
        return WindowProtocolType::WinAPI;
    }

    LONG WINAPI SRCustomExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo) {
        char buffer[256];
        std::snprintf(buffer, sizeof(buffer), "SRCustomExceptionHandler() : SEH exception: code = 0x%08lX", ExceptionInfo->ExceptionRecord->ExceptionCode);
        SR_PLATFORM_NS::WriteConsoleError(buffer);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    void SetRegistryValue(HKEY root, const char* subKey, const char* value) {
        HKEY hKey;
        LSTATUS status = RegCreateKeyExA(root, subKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
        if (status != ERROR_SUCCESS) {
            SR_PLATFORM_NS::WriteConsoleError("SetRegistryValue() : failed to create/open registry key {}! Error: {}"_format(subKey));
            return;
        }
        status = RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE*)value, (DWORD)strlen(value) + 1);
        if (status != ERROR_SUCCESS) {
            SR_PLATFORM_NS::WriteConsoleError("SetRegistryValue() : failed to set registry value for key {}! Error: {}"_format(subKey));
        }
        RegCloseKey(hKey);
    }

    void DeInitializePlatform() {
        DeInitMemoryHooks();
    }

    void InitializePlatform() {
        SR_TRACY_ZONE;
        SR_PLATFORM_NS::WriteConsoleLog("Platform::InitializePlatform() : initializing Windows platform...\n");

        InitMemoryHooks();

        HKEY hKey;
        LPCTSTR lpSubKey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\LastVisitedPidlMRU");

        /// Очистка последних открытых папок в диалогах открытия/сохранения файлов
        {
            const LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ, &hKey);

            if (lResult != ERROR_SUCCESS) {
                SR_PLATFORM_NS::WriteConsoleError("InitializePlatform() : failed to open registry key!");
                return;
            }
        }

        SetUnhandledExceptionFilter(SRCustomExceptionHandler);

        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

        DWORD dwIndex = 0;

        TCHAR szValueName[2048];
        DWORD dwValueNameSize = 2048;

        BYTE lpData[2048];
        DWORD dwDataSize = 2048;

        DWORD dwType;

        std::vector<std::pair<std::string, std::wstring>> values;

        while (true) {
            const LSTATUS code = RegEnumValue(hKey, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, lpData, &dwDataSize);
            if (code == ERROR_NO_MORE_ITEMS) {
                break;
            }

            if (dwType == REG_BINARY) {
                std::wstring strData;
                for (DWORD i = 0; i < dwDataSize; i += 2) {
                    const auto ch = static_cast<wchar_t>(lpData[i]);
                    strData.push_back(ch);
                }

                values.emplace_back(std::string(szValueName, szValueName + dwValueNameSize), strData);
            }

            dwValueNameSize = 2048;
            dwDataSize = 2048;
            ++dwIndex;
        }

        auto&& appName = SR_PLATFORM_NS::GetApplicationName();
        auto&& appNameW = ConvertToUnicode(appName.ToString());

        for (const auto& [name, data] : values) {
            if (data.find(appNameW) == 0) {
                LONG lResult = RegSetKeyValue(HKEY_CURRENT_USER, lpSubKey,  const_cast<char*>(name.c_str()), REG_BINARY, NULL, 0);

                if (lResult != ERROR_SUCCESS) {
                    SR_PLATFORM_NS::WriteConsoleError("InitializePlatform() : failed to delete register value!");
                }

                break;
            }
        }

        RegCloseKey(hKey);

        /// Регистрация расширения .srproject для открытия проектов в редакторе
        {
            std::string executablePath = GetApplicationPath().ToString();
            std::replace(executablePath.begin(), executablePath.end(), '/', '\\');
            std::string appPath = "\"{}\" \"%1\""_format(executablePath);

            // .srproject → SRProjectFile
            SetRegistryValue(HKEY_CURRENT_USER, "Software\\Classes\\.srproject", "SRProjectFile");
            // описание
            SetRegistryValue(HKEY_CURRENT_USER, "SRProjectFile", "SpaRcle Project File");
            // команда запуска
            std::string cmdKey = "Software\\Classes\\SRProjectFile\\shell\\open\\command";
            SetRegistryValue(HKEY_CURRENT_USER, cmdKey.c_str(), appPath.c_str());

            /// Уведомляем систему об изменении ассоциаций файлов, чтобы изменения вступили в силу
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        }
    }

    void SetInstance(void*) {

    }

    void* GetInstance() {
        return nullptr;
    }

    void* LoadLibraryModule(const Path& path) {
        std::string winAPIPath = path.ToString();
        std::replace(winAPIPath.begin(), winAPIPath.end(), '/', '\\');

        void* pLibrary = LoadLibrary(winAPIPath.c_str());
        if (!pLibrary) {
            SR_ERROR("PlatformWindows::LoadLibraryModule() : failed to load library: {}\n\tError: {}", path, GetLastErrorAsString());
            return nullptr;
        }

        return pLibrary;
    }

    bool UnloadLibraryModule(void* pLibrary) {
        if (!pLibrary) {
            SRHalt("PlatformWindows::UnloadLibraryModule() : library is nullptr!");
            return false;
        }

        if (!FreeLibrary(static_cast<HMODULE>(pLibrary))) {
            SR_ERROR("PlatformWindows::UnloadLibraryModule() : failed to unload library: {}\n\tError: {}", GetLastErrorAsString());
            return false;
        }

        return true;
    }

    void* GetLibraryFunctionAddress(void* pLibrary, const char* pFunctionName) {
        if (!pLibrary) {
            SRHalt("PlatformWindows::GetLibraryFunctionAddress() : library is nullptr!");
            return nullptr;
        }

        void* pFunction = reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(pLibrary), pFunctionName));
        if (!pFunction) {
            SR_ERROR("PlatformWindows::GetLibraryFunctionAddress() : failed to get function address: {}", pFunctionName);
            return nullptr;
        }

        return pFunction;
    }

    bool IsLibraryModuleLoaded(const SR_UTILS_NS::Path& path) {
      	SRHalt("PlatformWindows::IsLibraryModuleLoaded() : not suitable for Windows platform!");
        return false;
    }

    void TextToClipboard(const std::string &text) {
        if (text.empty()) {
            SR_WARN("Platform::TextToClipboard() : text is empty!");
            return;
        }

        HGLOBAL glob = GlobalAlloc(GMEM_FIXED, text.size() + 1);
        memcpy(glob, text.c_str(), text.size() + 1);

        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            SetClipboardData(CF_TEXT, glob);
            CloseClipboard();
        }
        else
            SR_ERROR("Platform::TextToClipboard() : failed to open clipboard!");
    }

    class COleInitialize {
    public:
        COleInitialize() : m_hr(OleInitialize(NULL)) { }
        ~COleInitialize() { if (SUCCEEDED(m_hr)) OleUninitialize(); }
        operator HRESULT() const { return m_hr; }
        HRESULT m_hr;
    };

    void SetCurrentProcessDirectory(const SR_UTILS_NS::Path& directory) {
        SetCurrentDirectory(directory.CStr());
    }

    ///функция для копирования файла/файлов в буфер обмена
    void CopyFilesToClipboard(std::list<SR_UTILS_NS::Path> paths) {

        // calculate *bytes* needed for memory allocation
        int clpSize = sizeof(DROPFILES);
        for (auto &&path:paths)
            clpSize += sizeof(TCHAR) * (_tcslen(path.ToString().c_str()) + 1); // + 1 => '\0'
        clpSize += sizeof(TCHAR); // two \0 needed at the end

        // allocate the zero initialized memory
        HDROP hdrop   = (HDROP)GlobalAlloc(GHND, clpSize);
        DROPFILES* df = (DROPFILES*)GlobalLock(hdrop);
        df->pFiles    = sizeof(DROPFILES); // string offset
#ifdef _UNICODE
        df->fWide     = TRUE; // unicode file names
#endif // _UNICODE

        // copy paths to the allocated memory
        TCHAR* dstStart = (TCHAR*)&df[1];
        for (auto &&path:paths)
        {
            // deprecated
            //_tcscpy(dstStart, path.ToString().c_str());

            _tcscpy_s(dstStart, path.ToString().length() + 1, path.ToString().c_str());

            dstStart = &dstStart[_tcslen(path.c_str()) + 1]; // + 1 => get beyond '\0'
        }
        GlobalUnlock(hdrop);

        // prepare the clipboard
        OpenClipboard(NULL);
        EmptyClipboard();
        SetClipboardData(CF_HDROP, hdrop);
        CloseClipboard();
    }

    bool IsRunningUnderDebugger() {
        return ::IsDebuggerPresent() == TRUE;
    }

    ///функция для получения файла/файлов из буфер обмена
    void PasteFilesFromClipboard(const SR_UTILS_NS::Path &topath) {

        if(!topath.IsDir()) {
            return;
        }

        if (IsClipboardFormatAvailable(CF_HDROP)) { ///CF_HDROP - формат списка файлов
            std::list<SR_UTILS_NS::Path> paths;

            OpenClipboard(NULL);
            HDROP hDrop = static_cast<HDROP>(GetClipboardData(CF_HDROP));
            CloseClipboard();

            const uint64_t size = 32768;
            std::string buffer;
            buffer.resize(size);
            for (int i = 0; i < DragQueryFileA(hDrop,0xFFFFFFFF,NULL,NULL); i++) {
                DragQueryFileA(hDrop,i,&buffer[0],size);
                auto path = SR_UTILS_NS::Path(buffer);
                Copy(path,topath.Concat(path.GetBaseNameAndExt()));
            }
        } else {
            return;
        }
    }

    KeyboardState GetSystemKeyboardState() {
        GetKeyState(0);
        uint8_t keycodes[256];
        if (::GetKeyboardState(&keycodes[0])) {
            KeyboardState keyboardState;
            for (size_t i = 0; i < 256; ++i) {
                keyboardState.keyStates[i] = keycodes[i] >> 7 != 0;
            }
            return keyboardState;
        }
        SR_ERROR("Platform::GetSystemKeyboardState() : failed to get keyboard state!");
        return KeyboardState();
    }

    std::string GetClipboardText() {
        std::string text{};

        if (OpenClipboard(NULL)) {
            HANDLE hData = GetClipboardData(CF_TEXT);

            if (hData == nullptr)
                return text;

            const uint64_t size = GlobalSize(hData);
            if (size > 0) {
                char *pszText = static_cast<char *>(GlobalLock(hData));

                text.resize(size);
                memcpy((void *) text.c_str(), pszText, size);

                GlobalUnlock(hData);
            }

            CloseClipboard();

            return text;
        }

        SR_ERROR("Platform::GetClipboardText() : failed to open clipboard!");

        return text;
    }

    void ClearClipboard() {
        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            CloseClipboard();
        }
        else
            SR_ERROR("Platform::ClearClipboard() : failed to open clipboard!");
    }

    SR_MATH_NS::FVector2 GetMousePos() {
        POINT p;
        GetCursorPos(&p);
        return Math::FVector2(p.x, p.y);
    }

    MouseState GetMouseState() {
        MouseState state;
        state.position = GetMousePos();
        state.buttonStates[0] = GetKeyState(VK_LBUTTON) & 0x8000;
        state.buttonStates[1] = GetKeyState(VK_RBUTTON) & 0x8000;
        state.buttonStates[2] = GetKeyState(VK_MBUTTON) & 0x8000;
        state.buttonStates[3] = GetKeyState(VK_XBUTTON1) & 0x8000;
        state.buttonStates[4] = GetKeyState(VK_XBUTTON2) & 0x8000;
        return state;
    }

    void Sleep(uint64_t milliseconds) {
        SR_TRACY_ZONE;
        ::Sleep(static_cast<DWORD>(milliseconds));
    }

    uint64_t GetProcessUsedMemory() {
#ifdef SR_MINGW
#else
        PROCESS_MEMORY_COUNTERS pmc;
        BOOL result = GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *) &pmc, sizeof(pmc));
        if (result) {
            return static_cast<uint64_t>(pmc.PeakWorkingSetSize);
        }
#endif

        return -1;
    }

    bool IsFileDeletable(const SR_UTILS_NS::Path& path) {
        if (!path.Exists() || !path.IsFile()) {
            SR_WARN("Platform::CanBeDeleted() : path does not exist or is not a file.");
            return false;
        }

        if (auto&& file = std::ofstream(path.c_str())) {
            file.close();
            return true;
        }

        return false;
    }

    void SetSamePermissions(const SR_UTILS_NS::Path& path) {
        SRHaltOnce("Platform::SetSamePermissions() : is not implemented!");
    }

    void SetThreadPriority(void *nativeHandle, ThreadPriority priority) {
        int32_t winPriority = 0;

        switch (priority) {
            case ThreadPriority::SR_THREAD_PRIORITY_ABOVE_NORMAL:
                winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
                break;
            case ThreadPriority::SR_THREAD_PRIORITY_BELOW_NORMAL:
                winPriority = THREAD_PRIORITY_BELOW_NORMAL;
                break;
            case ThreadPriority::SR_THREAD_PRIORITY_HIGHEST:
                winPriority = THREAD_PRIORITY_HIGHEST;
                break;
            case ThreadPriority::SR_THREAD_PRIORITY_IDLE:
                winPriority = THREAD_PRIORITY_IDLE;
                break;
            case ThreadPriority::SR_THREAD_PRIORITY_LOWEST:
                winPriority = THREAD_PRIORITY_LOWEST;
                break;
            case ThreadPriority::SR_THREAD_PRIORITY_NORMAL:
                winPriority = THREAD_PRIORITY_NORMAL;
                break;
            case ThreadPriority::SR_THREAD_PRIORITY_TIME_CRITICAL:
                winPriority = THREAD_PRIORITY_TIME_CRITICAL;
                break;
            default:
                SRAssert(false);
                return;
        }

        auto&& result = ::SetThreadPriority(static_cast<HANDLE>(nativeHandle), winPriority);

        if (result == FALSE) {
            SR_ERROR("Platform::SetThreadPriority() : failed to set thread priority!");
        }
    }

    void Terminate(bool isError) {
        if (isError) {
            SR_PLATFORM_NS::WriteConsoleError("Function \"Terminate\" has been called... >_<\n" + SR_UTILS_NS::GetStacktrace());
            SR_UTILS_NS::Breakpoint();
        }
        else {
            SR_PLATFORM_NS::WriteConsoleLog("Function \"Terminate\" has been called...\n" + SR_UTILS_NS::GetStacktrace());
        }
#ifdef SR_ANDROID

#endif
#ifdef SR_MINGW

#else
        if (isError) {
            terminate();
        }
        else {
            exit(0);
        }
#endif
    }

    bool CreateFolder(const std::string& path) {
#ifdef SR_MINGW
        return mkdir(path.c_str());
#else
        return _mkdir(path.c_str());
#endif
    }

    bool IsConsoleFocused() {
        return GetForegroundWindow() == GetCurrentProcess();
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

    Path GetApplicationPath() {
        const std::size_t buf_len = 260;
        auto s = new TCHAR[buf_len];
        GetModuleFileName(GetModuleHandle(nullptr), s, buf_len);
        Path appPath(s);
        delete[] s;
        return appPath;
    }

    Path GetApplicationDirectory() {
        return GetApplicationPath().GetFolder();
    }

    std::optional<Path> GetApplicationCachePath() {
        return std::nullopt;
    }

    std::optional<Path> GetApplicationLogPath() {
        return std::nullopt;
    }

    Path GetApplicationName() {
        const std::size_t buf_len = 260;
        auto s = new TCHAR[buf_len];
        auto path_len = GetModuleFileName(GetModuleHandle(nullptr), s, buf_len);
        return Path(s).GetBaseNameAndExt();
    }

    bool FileIsHidden(const Path &path) {
        const DWORD attributes = GetFileAttributes(path.CStr());
        if (attributes & FILE_ATTRIBUTE_HIDDEN)
            return true;

        return false;
    }

    void OpenFile(const SR_UTILS_NS::Path& path, const std::string& args) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        /// start the program up
        CreateProcess(path.c_str(), /// the path
            const_cast<char*>(args.c_str()),
            NULL, /// Process handle not inheritable
            NULL, /// Thread handle not inheritable
            FALSE, /// Set handle inheritance to FALSE
            0, /// No creation flags
            NULL, /// Use parent's environment block
            NULL, /// Use parent's starting directory
            &si, /// Pointer to STARTUPINFO structure
            &pi  /// Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
        );

        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    void SelfOpen() {
        auto&& exe = SR_PLATFORM_NS::GetApplicationPath();
        OpenFile(exe, "");
    }

    void Unzip(const SR_UTILS_NS::Path& source, const SR_UTILS_NS::Path& destination, bool replace) {
		//TODO: Add support for the 'replace' argument.
        destination.CreateIfNotExists();
        std::string command = "tar -xf "+ source.ToString() + " -C " + destination.ToString();
        system(command.c_str());
    }

    FileMetadata GetFileMetadata(const Path& file) {
        FileMetadata fileMetadata;
        WIN32_FIND_DATA fd;
        HANDLE hFind = ::FindFirstFile(file.c_str(), &fd);
        if(hFind != INVALID_HANDLE_VALUE) {
            ///You must convert FILETIME to ULARGE_INTEGER to get a value for uint64_t
            ULARGE_INTEGER lastWriteTime{fd.ftLastWriteTime.dwLowDateTime, fd.ftLastWriteTime.dwHighDateTime};
            fileMetadata.lastWriteTime = lastWriteTime.QuadPart;
            ::FindClose(hFind);
        } else {
            fileMetadata.lastWriteTime = SR_UINT64_MAX; ///TODO: какое значение стоит назначить в случае, если не был получен handle файла?
        }
        return fileMetadata; ///TODO: std::move в будущем, когда FileMetadata станет больше?
    }

    SR_MATH_NS::UVector2 GetScreenResolution() {
        const HDC hdc = GetDC(NULL);
        const int32_t w = GetDeviceCaps(hdc, HORZRES);
        const int32_t h = GetDeviceCaps(hdc, VERTRES);
        ReleaseDC(NULL, hdc);
        return SR_MATH_NS::UVector2(w, h);
    }

    double_t GetScreenDPI() {
        const HDC hdc = GetDC(NULL);
        const int32_t ret = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(NULL, hdc);
        return ret;
    }

    bool IsAbsolutePath(const Path &path) {
        auto&& view = path.View();
        return view.size() >= 2 && view[1] == ':';
    }

    void OpenWithAssociatedApp(const Path &path){
        ShellExecuteA(NULL, "open", path.ToString().c_str(), NULL, NULL, SW_SHOWDEFAULT);
    }

    std::vector<SR_MATH_NS::UVector2> GetScreenResolutions() {
        auto&& resolutions = std::vector<SR_MATH_NS::UVector2>();

        resolutions.reserve(64);

        DEVMODE dm = { 0 };
        dm.dmSize = sizeof(dm);

        for (uint32_t iModeNum = 0; EnumDisplaySettings(NULL, iModeNum, &dm) != 0; ++iModeNum) {
            auto&& resolution = SR_MATH_NS::UVector2(dm.dmPelsWidth, dm.dmPelsHeight);

            if (std::find(resolutions.begin(), resolutions.end(), resolution) != resolutions.end()) {
                continue;
            }

            resolutions.emplace_back(resolution);
        }

        if (resolutions.empty()) {
            SR_LOG("Platform::GetScreenResolutions() : failed to get screen resolutions, returning default 400x400");
            resolutions.emplace_back(SR_MATH_NS::UVector2(400, 400));
        }

        return resolutions;
    }

    void SetMousePos(const SR_MATH_NS::IVector2& pos) {
        ::SetCursorPos(pos.x, pos.y);
    }

    void SetCursorVisible(bool isVisible) {
        ::ShowCursor(isVisible);

        // if (!isVisible) {
        //     ::SetCursor(nullptr);
        //     return;
        // }
        //
        // HINSTANCE hInstance = ::GetModuleHandle(NULL);  // get a handle to the app's instance
        // HCURSOR hCursor = ::LoadCursor(hInstance, MAKEINTRESOURCE(IDC_ARROW));  // load a cursor
        // ::SetCursor(hCursor);
    }

    static HWND gWindowsPlatformWindow = nullptr;

    void ConfineCursor() {
        if (!gWindowsPlatformWindow) {
            // Получаем дескриптор окна - можно заменить на своё окно
            gWindowsPlatformWindow = GetForegroundWindow();
        }

        RECT rect;
        if (GetWindowRect(gWindowsPlatformWindow, &rect)) {
            ClipCursor(&rect);
        }
    }

    void ReleaseCursorConfinement() {
        ClipCursor(nullptr);
    }

    PlatformType GetType() {
        return PlatformType::Windows;
    }

    void SetEnvironmentVar(const std::string_view& name, const std::string_view& value) {
        SetEnvironmentVariableA(name.data(), value.data());
    }

    std::string ExecuteCommand(const std::string& command, const std::vector<std::string>& env) {
        HANDLE hReadPipe, hWritePipe;
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
            return "Pipe creation failed";
        }

        // Получаем текущее окружение
        LPCH envStrings = GetEnvironmentStringsA();
        if (!envStrings) {
            return "Failed to get environment strings";
        }

        // Скопировать окружение в модифицируемую структуру
        std::vector<char> newEnv;
        LPCH ptr = envStrings;
        while (*ptr) {
            std::string entry(ptr);
            newEnv.insert(newEnv.end(), entry.begin(), entry.end());
            newEnv.push_back('\0');
            ptr += entry.size() + 1;
        }

        for (const auto& entry : env) {
            newEnv.insert(newEnv.end(), entry.begin(), entry.end());
            newEnv.push_back('\0');
        }
        newEnv.push_back('\0');
        newEnv.push_back('\0'); // двойной ноль — конец окружения

        STARTUPINFO si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi;
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;
        si.dwFlags |= STARTF_USESTDHANDLES;

        if (!CreateProcess(nullptr, (LPSTR)command.c_str(), nullptr, nullptr, TRUE, 0, newEnv.data(), nullptr, &si, &pi)) {
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            return "Process creation failed";
        }

        CloseHandle(hWritePipe); // Закрываем ненужную часть пайпа

        std::string result;
        char buffer[128];
        DWORD bytesRead;
        while (::ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            result += buffer;
        }

        CloseHandle(hReadPipe);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return result;
    }

    bool DownloadFile(const std::string& url, const Path& outputPath) {
        SR_LOG("Platform::DownloadFile() : downloading file from url: {}", url);

        if (!outputPath.Create()) {
            SR_ERROR("Platform::DownloadFile() : failed to create output path: {}", outputPath.ToStringRef());
            return false;
        }

        HINTERNET hInternet = InternetOpenA("SREngine", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            SR_ERROR("Platform::DownloadFile() : failed to open internet: {}", GetLastErrorAsString());
            return false;
        }

        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hUrl) {
            SR_ERROR("Platform::DownloadFile() : failed to open URL: {}", GetLastErrorAsString());
            InternetCloseHandle(hInternet);
            return false;
        }

        std::ofstream file(outputPath.ToStringRef(), std::ios::binary);
        char buffer[4096];
        DWORD bytesRead;

        while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead) {
            file.write(buffer, bytesRead);
        }

        file.close();
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);

        SR_LOG("Platform::DownloadFile() : file downloaded successfully to: {}", outputPath.ToStringRef());

        return true;
    }

    void ShowWindow(void* pHandle, ShowWindowActionType action) {
        switch (action) {
            case ShowWindowActionType::Show:
                ::ShowWindow(static_cast<HWND>(pHandle), SW_SHOW);
                break;
            case ShowWindowActionType::Hide:
                ::ShowWindow(static_cast<HWND>(pHandle), SW_HIDE);
                break;
            case ShowWindowActionType::Minimize:
                ::ShowWindow(static_cast<HWND>(pHandle), SW_MINIMIZE);
                break;
            case ShowWindowActionType::Maximize:
                ::ShowWindow(static_cast<HWND>(pHandle), SW_MAXIMIZE);
                break;
            case ShowWindowActionType::Restore:
                ::ShowWindow(static_cast<HWND>(pHandle), SW_RESTORE);
            default:
                SRHalt("Platform::ShowWindow() : unknown action type!");
                break;
        }
    }
}