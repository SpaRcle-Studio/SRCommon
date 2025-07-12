//
// Created by Monika on 12.07.2025.
//

#include <Utils/Platform/Platform.h>
#include <Utils/Common/CLIManager.h>
#include <Utils/Debug.h>

namespace SR_PLATFORM_NS {
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

        static const std::vector<std::string> potentialPaths = { "", "..", "../..", "../../..", "../../../.." };

        for (auto&& relativePath : potentialPaths) {
            auto&& fullPath = GetApplicationPath().Concat(relativePath);

        #ifdef SR_LINUX
            if (fullPath.View().size() == 1) {
                return SR_UTILS_NS::Path();
            }
        #endif

            fullPath = fullPath.Concat("Resources");
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
} // namespace SR_PLATFORM_NS