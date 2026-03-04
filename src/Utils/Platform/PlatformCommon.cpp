//
// Created by Monika on 12.07.2025.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Input/KeyCodes.h>
#include <Utils/Debug.h>
#include <Utils/Common/CLIManager.h>
#include <Utils/Profile/TracyContext.h>

#include <Enum/KeyCode.hpp>

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

#ifndef SR_LINUX
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

#if !defined(SR_EMSCRIPTEN)
    void SetApplicationMainLoop(bool(*mainLoop)(void*), void* pApplication) {
        while (mainLoop(pApplication)) {
            continue;
        }
    }
#endif

#if !defined(SR_ANDROID)
    bool ReadFileOriginal(const Path& path, std::string& buffer) {
        SR_TRACY_ZONE;

        // Открываем файл в бинарном режиме и сразу получаем размер
        std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
        if (!file) {
            return false;
        }

        const std::streamsize size = file.tellg();

        buffer.resize(static_cast<size_t>(size));
        file.seekg(0, std::ios::beg);
        if (!file.read(buffer.data(), size)) {
            return false;
        }
        return true;
    }

    bool ReadFile(const Path& path, std::string& buffer) {
        SR_TRACY_ZONE;

        if (g_platformHooks.readFileHook) {
            return g_platformHooks.readFileHook(path, buffer);
        }
        else {
            return ReadFileOriginal(path, buffer);
        }
    }

    Path::Type GetPathTypeOriginal(std::string_view path) {
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

    Path::Type GetPathType(std::string_view path) {
        if (g_platformHooks.getFileTypeHook) {
            return g_platformHooks.getFileTypeHook(path);
        }
        else {
            return GetPathTypeOriginal(path);
        }
    }

    Path GetApplicationResourcesPath() {
        if (auto&& folderArg = CLIManager::Instance().GetOptionValue(CLIOptions::Resources); folderArg.has_value()) {
            auto&& folder = SR_UTILS_NS::Path(folderArg.value());

            if (!folder.Exists(SR_UTILS_NS::Path::Type::Folder)) {
                SR_INFO("Platform::GetApplicationResourcesPath() : specified resources folder does not exist! Path: {}", folder);
            }
            else {
                return folder;
            }
        }

        static const std::vector<std::string> potentialPaths = {
            "Resources", "../Resources", "../../Resources", "../../../Resources", "../../../../Resources",
        };

        for (auto&& relativePath : potentialPaths) {
            auto&& fullPath = GetApplicationPath().Concat(relativePath);
            if (fullPath.Exists(SR_UTILS_NS::Path::Type::Folder)) {
                if (fullPath.Concat("Engine").Exists(SR_UTILS_NS::Path::Type::Folder)) { /// Check is folder correct
                    return fullPath;
                }
            }
        }

        SR_UTILS_NS::Path defaultFlatpakPath = "/app/share/SREngine/Resources";
        if (defaultFlatpakPath.Exists(SR_UTILS_NS::Path::Type::Folder)) {
            return defaultFlatpakPath;
        }

        SR_LOG("Platform::GetApplicationResourcesPath() : resources folder was not found in any of the potential paths. Please specify the resources folder using --resources option or reinstall the application.");

        return SR_UTILS_NS::Path();
    }

    void InitializeHooks(const std::function<void(PlatformHooks& hooks)>& callback) {
        PlatformHooks hooks;
        hooks.originalReadFile = &ReadFileOriginal;
        hooks.originalGetPathType = &GetPathTypeOriginal;
        callback(hooks);
        g_platformHooks = hooks;
    }
#endif
} // namespace SR_PLATFORM_NS