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
            return ExportAllResources(SR_UTILS_NS::Path());
        }

        bool ExportAllResources(SR_UTILS_NS::Path newDirectory) {
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

        static bool ExportToFile(const EmbedResourceStructure& resource, const SR_UTILS_NS::Path& newDirectory) {
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
                if (!path.Create()) {
                    SR_ERROR("ResourceEmbedder::ExportToFile() : failed to create path.");
                    return false;
                }
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

        bool ExportToFile(const SR_UTILS_NS::Path& path) {
            auto&& it = m_resources.find(path.ToStringRef());
            return ExportToFile({path.CStr(), it->second.second, it->second.first}, SR_UTILS_NS::Path());
        }

    private:
        ska::flat_hash_map<std::string, std::pair<uint64_t, const char*>> m_resources;
    };
}

#endif //SRENGINE_RESOURCEEMBEDDER_H
