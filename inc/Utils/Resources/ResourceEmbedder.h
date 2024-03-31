//
// Created by innerviewer on 2024-03-24.
//

#ifndef SRENGINE_RESOURCEEMBEDDER_H
#define SRENGINE_RESOURCEEMBEDDER_H

#include <Utils/stdInclude.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Types/Map.h>

namespace SR_UTILS_NS {
    class ResourceEmbedder {
    public:
        static ResourceEmbedder& Instance() {
            static ResourceEmbedder instance;
            return instance;
        }

    public:
        struct EmbedResourceStructure {
            const char* path;
            const char* data;
            uint64_t size;
        };

        template<class T> bool RegisterResource() {
            const char* pData = reinterpret_cast<const char*>(&T::data[0]);
            uint64_t dataSize = T::size;

            m_resources[T::path] = std::make_pair(dataSize, pData);

            return true;
        }

        bool ExportAllResources() {
            for (auto&& it : m_resources) {
                ExportToFile(it.first);
            }

            return true;
        }

        static bool ExportToFile(const EmbedResourceStructure& resource) {
            SR_UTILS_NS::Path path = resource.path;
            auto&& dataSize = resource.size;
            auto&& pData = resource.data;

            std::string buffer;
            buffer.resize(dataSize);
            memcpy(buffer.data(), pData, dataSize);

            if (!path.Exists()) {
                path.Create();
            }

            std::ofstream file(path.CStr(), std::ios::out | std::ios::binary);

            if (!file.is_open()) {
                return false;
            }

            file.write(buffer.data(), buffer.size());
            file.close();
        }

        bool ExportToFile(const SR_UTILS_NS::Path& path) {
            auto&& it = m_resources.find(path.ToStringRef());

            return ExportToFile({path.CStr(), it->second.second, it->second.first});
        }

    private:
        ska::flat_hash_map<std::string, std::pair<uint64_t, const char*>> m_resources;
    };
}

#endif //SRENGINE_RESOURCEEMBEDDER_H
