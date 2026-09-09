//
// Created by Monika on 12.07.2025.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Input/KeyCodes.h>
#include <Utils/Common/CLIManager.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Types/Vector.h>

#include <filesystem>
#include <sys/stat.h>
#include <cerrno>

#ifdef SR_WIN32
    #include <direct.h>
    #ifdef SR_MINGW
        #define SR_MKDIR(path) mkdir(path)
    #else
        #define SR_MKDIR(path) _mkdir(path)
    #endif
#elif defined(SR_LINUX) || defined(SR_APPLE) || defined(SR_EMSCRIPTEN) || defined(SR_ANDROID)
    #define SR_MKDIR(path) mkdir(path, 0777)
#else
    #error "Unsupported platform"
#endif

namespace SR_PLATFORM_NS {
    bool IsMobilePlatform() {
        switch (GetType()) {
        case PlatformType::Android:
        case PlatformType::Emscripten:
        case PlatformType::IOS:
            return true;
        default:
            return false;
        }
    }

    bool IsFileExists(StringView path) {
        return GetPathType(path) == FSItemType::File;
    }

    bool IsDirectoryExists(StringView path) {
        return GetPathType(path) == FSItemType::Folder;
    }

    bool IsSupportThreads() {
        return GetType() != PlatformType::Emscripten;
    }

    BuildType GetBuildType() {
    #if defined(SR_DEBUG)
        return BuildType::Debug;
    #elif defined(SR_RELEASE)
        return BuildType::Release;
    #else
        return BuildType::Unknown;
    #endif
    }

#ifndef SR_LINUX
    void CopyPermissions(const Path& from, const Path& to) {
        /// do nothing
    }

    void AccumulateMouseDelta(const SR_MATH_NS::FVector2& delta) {
        SRHalt("Platform::AccumulateMouseDelta() : not implemented!");
    }

    SR_MATH_NS::FVector2 ConsumeAccumulatedMouseDelta() {
        SRHalt("Platform::ConsumeAccumulatedMouseDelta() : not implemented!");
        return {};
    }

    void SetCursorLockActive(bool active) {
        SRHalt("Platform::SetCursorLockActive() : not implemented!");
    }

    bool IsCursorLockActive() {
        SRHalt("Platform::IsCursorLockActive() : not implemented!");
        return false;
    }
#endif

    bool CreateDirectories(StringView path) {
        if (path.empty()) {
            return false;
        }

        static SR_THREAD_LOCAL String current;
        current.clear();

        for (size_t i = 0; i < path.size(); ++i) {
            const char c = path[i];
            current += c;

            if (c != '/' && c != '\\' && i + 1 != path.size())
                continue;

            /// Не пытаемся создавать корень "C:/" и пустые компоненты.
            if (current.empty() || current == "/" || current == "\\")
                continue;

        #ifdef SR_WIN32
            if (current.size() == 3 && current[1] == ':' && (current[2] == '/' || current[2] == '\\')) {
                continue;
            }
        #endif

            if (SR_MKDIR(current.c_str()) != 0) {
                struct stat st{};
                if (stat(current.c_str(), &st) != 0 && errno != EEXIST)
                    return false;
            }
        }

        /// Последний компонент, если путь не заканчивался slash'ем.
        if (!current.empty() && current.back() != '/' && current.back() != '\\') {
            if (SR_MKDIR(current.c_str()) != 0) {
                struct stat st{};
                if (stat(current.c_str(), &st) != 0 && errno != EEXIST)
                    return false;
            }
        }

        return true;
    }

    void KeyboardState::Set(KeyCode key, const bool isPressed) {
        const bool current = keyStates[static_cast<uint8_t>(key)];
        if (current == isPressed) {
            return;
        }
        keyStates[static_cast<uint8_t>(key)] = isPressed;
    }

    bool KeyboardState::Get(KeyCode key) const {
        return keyStates[static_cast<uint8_t>(key)];
    }

    void SetOverriddenMouseState(const std::optional<MouseState>& mouseState) {
        g_overriddenMouseState.store(mouseState);
    }

    std::optional<MouseState> GetOverriddenMouseState() {
        return g_overriddenMouseState.load();
    }

    void SetOverriddenKeyboardState(const std::optional<KeyboardState>& keyboardState) {
        g_overriddenKeyboardState.store(keyboardState);
    }

    std::optional<KeyboardState> GetOverriddenKeyboardState() {
        return g_overriddenKeyboardState.load();
    }

    Path GetApplicationResourcesPath() {
        if (GetType() == PlatformType::Android) {
            return "Resources";
        }

        if (auto&& folderArg = CLIManager::Instance().GetOptionValue(CLIOptions::Resources); folderArg.has_value()) {
            auto&& folder = SR_UTILS_NS::Path(folderArg.value());

            if (!SR_PLATFORM_NS::IsDirectoryExists(folder)) {
                SR_INFO("Platform::GetApplicationResourcesPath() : specified resources folder does not exist! Path: {}", folder);
            }
            else {
                return folder;
            }
        }

        static const StringView potentialPaths[5] = { "Resources", "../Resources", "../../Resources", "../../../Resources", "../../../../Resources", };
        for (auto&& relativePath : potentialPaths) {
            auto&& fullPath = GetApplicationPath().Concat(relativePath);
            if (SR_PLATFORM_NS::IsDirectoryExists(fullPath)) {
                if (SR_PLATFORM_NS::IsDirectoryExists(fullPath.Concat("Engine"))) { /// Check is folder correct
                    return fullPath;
                }
            }
        }

        static const SR_UTILS_NS::Path defaultFlatpakPath = "/app/share/SREngine/Resources";
        if (SR_PLATFORM_NS::IsDirectoryExists(defaultFlatpakPath)) {
            return defaultFlatpakPath;
        }

        SR_LOG("Platform::GetApplicationResourcesPath() : resources folder was not found in any of the potential paths. Please specify the resources folder using --resources option or reinstall the application.");
        return SR_UTILS_NS::Path();
    }

#if !defined(SR_EMSCRIPTEN)
    void SetApplicationMainLoop(bool(*mainLoop)(void*), void* pApplication) {
        while (mainLoop(pApplication)) {
        #ifdef SR_THREADS_ALLOWED
            SR_PLATFORM_NS::Sleep(10);
        #endif
            continue;
        }
    }
#endif

#if !defined(SR_ANDROID)
    Path::Type GetPathType(StringView path) {
        SR_TRACY_ZONE;

    #ifdef SR_WIN32
        if (path.size() < 2 || path[1] != ':') {
            return Path::Type::Undefined;
        }
    #elif defined(SR_LINUX)
        if (path.empty() || path[0] != '/') {
            return Path::Type::Undefined;
        }
    #endif

    #if defined(SR_MSVC) || defined (SR_LINUX) || defined(SR_EMSCRIPTEN)
        struct stat s{};
        if(stat(path.data(), &s) == 0) {
            if (s.st_mode & S_IFDIR) {
                return Path::Type::Folder;
            } else if (s.st_mode & S_IFREG) {
                return Path::Type::File;
            }
        }

        return Path::Type::Undefined;
    #elif defined(SR_WIN32)
        DWORD attrib = GetFileAttributes(path.data());

        if ((attrib & FILE_ATTRIBUTE_DIRECTORY) != 0)
            return Path::Type::Folder;

        return Path::Type::File;
    #else
        SRHalt("Unsupported OS!");
        return Path::Type::Undefined;
    #endif
    }
#endif
} // namespace SR_PLATFORM_NS