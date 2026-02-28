//
// Created by innerviewer on 2024-03-24.
//

#ifndef SR_ENGINE_RESOURCEEMBEDDER_H
#define SR_ENGINE_RESOURCEEMBEDDER_H

#include <Utils/stdInclude.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Types/Map.h>
#include <Utils/Types/FlatHashMap.h>

namespace SR_UTILS_NS {
    class ResourceEmbedder {
        struct Resource {
            std::string_view compressedHex;
            uint64_t decompressedSize = 0;
        };
    public:
        static ResourceEmbedder& Instance();

    public:
        template<class T> bool RegisterResource() {
            Resource resource;
            resource.compressedHex = std::string_view(T::data, T::compressedSize);
            resource.decompressedSize = T::decompressedSize;
            m_resources[T::path] = resource;
            return true;
        }

        bool ExportAllResources();
        bool ExportAllResources(const SR_UTILS_NS::Path& newDirectory);

        static bool ExportToFile(std::string_view path, const Resource& resource, const SR_UTILS_NS::Path& newDirectory);
        bool ExportToFile(const SR_UTILS_NS::Path& path);

        static bool ExportToMemory(std::string_view data);

    private:
        SR_NODISCARD static std::string HexToBytes(const std::string_view& hex);
        SR_NODISCARD static std::string Decompress(const Resource& resource);

    private:
        SR_HTYPES_NS::FlatHashMap<std::string_view, Resource> m_resources;
    };
}

#endif //SR_ENGINE_RESOURCEEMBEDDER_H
