//
// Created by innerviewer on 2024-03-24.
//

#ifndef SRENGINE_RESOURCEEMBEDDER_H
#define SRENGINE_RESOURCEEMBEDDER_H

#include <Utils/stdInclude.h>
#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS {
    class ResourceEmbedder {
    public:
        template<class T> static bool ExportToFile(const SR_UTILS_NS::Path& path) {
            const char* pData = reinterpret_cast<const char*>(&T::data[0]);
            uint64_t dataSize = T::size;

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

    };
}

#endif //SRENGINE_RESOURCEEMBEDDER_H
