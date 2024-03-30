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

        template<class T> bool RegisterResource() {
            const char* pData = reinterpret_cast<const char*>(&T::data[0]);
            uint64_t dataSize = T::size;

            m_resources[T::path] = std::make_pair(dataSize, pData);

            return true;
        }

        bool ExportToFile(const SR_UTILS_NS::Path& path) {
            auto&& it = m_resources.find(path.ToStringRef());
            auto&& dataSize = it->second.first;
            auto&& pData = it->second.second;

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

            return true;
        }

    private:
        ska::flat_hash_map<std::string, std::pair<uint64_t, const char*>> m_resources;
    };
}

#endif //SRENGINE_RESOURCEEMBEDDER_H
