//
// Created by Monika on 12.07.2025.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Input/KeyCodes.h>
#include <Utils/Common/CLIManager.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Types/Vector.h>

#include <Enum/KeyCode.hpp>

#include <filesystem>

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

    BuildType GetBuildType() {
    #if defined(SR_DEBUG)
        return BuildType::Debug;
    #elif defined(SR_RELEASE)
        return BuildType::Release;
    #else
        return BuildType::Unknown;
    #endif
    }

    void GetInDirectory(const Path& dir, Path::Type type, SR_UTILS_NS::Vector<Path>& out) {
        SR_TRACY_ZONE;
        out.clear();

        uint32_t count = 0;

        if (!dir.IsDir()) {
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir.View())) {
            const bool isDirectory = entry.is_directory();
            const bool isFile = entry.is_regular_file();
            if ((type == Path::Type::Folder && isDirectory) || (type == Path::Type::File && isFile) || type == Path::Type::Undefined) {
                count++;
            }
        }

        out.reserve(count);

        for (const auto& entry : std::filesystem::directory_iterator(dir.View())) {
            const bool isDirectory = entry.is_directory();
            const bool isFile = entry.is_regular_file();
            if ((type == Path::Type::Folder && isDirectory) || (type == Path::Type::File && isFile) || type == Path::Type::Undefined) {
                out.emplace_back(entry.path());
            }
        }
    }

    bool Copy(const Path& from, const Path& to) {
        SR_TRACY_ZONE;

    #ifdef SR_ANDROID
        std::string_view pathToView = to.ToStringView();
        if (RemoveAssetsPrefix(pathToView)) {
            SR_ERROR("Platform::Copy() : can't write asset file!");
            return false;
        }
    #endif

        if (from.IsFile()) {
            SR_UTILS_NS::String buffer;
            if (!SR_UTILS_NS::FileSystem::ReadFile(from, buffer)) {
                SR_ERROR("Platform::Copy() : failed to read file!\n\tPath: {}", from);
                return false;
            }

            to.Create();

            std::ofstream file(to.c_str(), std::ios::binary);
            if (!file.is_open()) {
                SR_ERROR("Platform::Copy() : failed to open file for writing!\n\tPath: {}\n\tReason = {} ({})", to, errno, std::system_category().message(errno));
                return false;
            }
            file.write(buffer.data(), buffer.size());
            CopyPermissions(from, to);
            return true;
        }

        if (!from.IsDir()) {
            SR_ERROR("Platform::Copy() : \"{}\" is not a directory!", from);
            return false;
        }

        to.Create();

        Vector<Path> items;
        GetInDirectory(from, Path::Type::Undefined, items);
        for (auto&& item : items) {
            if (Copy(item, to.Concat(item.GetBaseNameAndExt()))) {
                continue;
            }

            return false;
        }

        return true;
    }

    bool Delete(const Path& path) { /// TODO: Обезопасить от безвозвратного удаления файлов
        SR_TRACY_ZONE;

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

        SR_UTILS_NS::Vector<Path> items;
        GetInDirectory(path, Path::Type::Undefined, items);
        for (auto&& item : items) {
            if (Delete(item)) {
                continue;
            }

            return false;
        }

    #ifdef SR_WIN32
        const bool result = _rmdir(path.CStr()) == 0;
    #else
        const bool result = rmdir(path.CStr()) == 0;
    #endif

        if (!result) {
            SR_WARN("Platform::Delete() : failed to delete folder!\n\tPath: {}", path.CStr());
        }

        return result;
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
        #ifdef SR_THREADS_ALLOWED
            SR_PLATFORM_NS::Sleep(10);
        #endif
            continue;
        }
    }
#endif

#if !defined(SR_ANDROID)
    bool ReadFileOriginal(const Path& path, String& buffer) {
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

    bool ReadFile(const Path& path, String& buffer) {
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