//
// Created by innerviewer on 2024-03-24.
//

#ifndef SR_ENGINE_RESOURCEEMBEDDER_H
#define SR_ENGINE_RESOURCEEMBEDDER_H

#include <Utils/stdInclude.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Types/Map.h>

namespace SR_UTILS_NS {
    struct EmbedResourceStructure {
        const char* path;
        const char* data;
        uint64_t size;
    };

    class ResourceEmbedder {
    public:
        static ResourceEmbedder& Instance() {
            static ResourceEmbedder instance;
            return instance;
        }

    public:
        template<class T> bool RegisterResource() {
            const char* pData = reinterpret_cast<const char*>(&T::data[0]);
            uint64_t dataSize = T::size;

            m_resources[T::path] = std::make_pair(dataSize, pData);

            return true;
        }

        bool ExportAllResources();
        bool ExportAllResources(SR_UTILS_NS::Path newDirectory);

        static bool ExportToFile(const EmbedResourceStructure& resource, const SR_UTILS_NS::Path& newDirectory);
        bool ExportToFile(const SR_UTILS_NS::Path& path);

        static bool ExportToMemory(const EmbedResourceStructure& resource);

    private:
        ska::flat_hash_map<std::string, std::pair<uint64_t, const char*>> m_resources;
    };
}

#endif //SR_ENGINE_RESOURCEEMBEDDER_H
