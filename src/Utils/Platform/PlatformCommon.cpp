//
// Created by Monika on 12.07.2025.
//

#include <Utils/Debug.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Common/CLIManager.h>

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
#ifndef SR_ANDROID
    Path::Type GetPathType(std::string_view path) {
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

    #if defined(SR_MSVC) || defined (SR_LINUX)
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
#endif
} // namespace SR_PLATFORM_NS