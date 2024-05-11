//
// Created by innerviewer on 2024-04-07.
//

#include <Utils/Resources/ResourceEmbedder.h>

namespace SR_UTILS_NS {
    bool ResourceEmbedder::ExportAllResources() {
        return ExportAllResources(SR_UTILS_NS::Path());
    }

    bool ResourceEmbedder::ExportAllResources(SR_UTILS_NS::Path newDirectory) {
        bool result = true;
        if (!newDirectory.IsEmpty() && !newDirectory.Exists()) {
            if (!newDirectory.Create()) {
                SR_ERROR("ResourceEmbedder::ExportAllResources() : failed to create new directory.");
                result = false;
            }
        }

        for (auto&& it : m_resources) {
            EmbedResourceStructure resource = {it.first.c_str(), it.second.second, it.second.first};

            if (!ExportToFile(resource, newDirectory)) {
                result = false;
            }
        }

        return result;
    }

    bool ResourceEmbedder::ExportToFile(
        const SR_UTILS_NS::EmbedResourceStructure &resource,
        const SR_UTILS_NS::Path &newDirectory
    ) {
        SR_UTILS_NS::Path path = newDirectory.Concat(resource.path);
        if (newDirectory.IsEmpty()) {
            path = SR_UTILS_NS::Path(resource.path);
        }

        auto&& dataSize = resource.size;
        auto&& pData = resource.data;

        std::string buffer;
        buffer.resize(dataSize);
        memcpy(buffer.data(), pData, dataSize);

        if (!path.Exists()) {
        #ifdef SR_LINUX
            /// It is needed because on Linux there are files without extensions.
            if (!SR_PLATFORM_NS::CreateFolder(path.GetPrevious().GetFolder())) {
                SR_ERROR("ResourceEmbedder::ExportToFile() : failed to create path.");
                return false;
            }

        #else
            if (!path.Create()) {
                SR_ERROR("ResourceEmbedder::ExportToFile() : failed to create path.");
                return false;
            }
        #endif
        }

        std::ofstream file(path.CStr(), std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            SR_ERROR("ResourceEmbedder::ExportToFile() : failed to open file '{}'.", resource.path);
            return false;
        }

        file.write(buffer.data(), buffer.size());
        file.close();

        return true;
    }

    bool ResourceEmbedder::ExportToFile(const SR_UTILS_NS::Path &path) {
        auto&& it = m_resources.find(path.ToStringRef());
        return ExportToFile({path.CStr(), it->second.second, it->second.first}, SR_UTILS_NS::Path());
    }

    bool ResourceEmbedder::ExportToMemory(const EmbedResourceStructure &resource) {
		SRHalt("Not yet implemented!");
        return false;
    }
}