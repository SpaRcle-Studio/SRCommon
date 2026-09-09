//
// Created by innerviewer on 2024-04-07.
//

#include <Utils/Resources/ResourceEmbedder.h>
#include <Utils/Common/StringAtomLiterals.h>
#include <Utils/Common/LexicalCast.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/Platform/Platform.h>

namespace SR_UTILS_NS {
    bool ResourceEmbedder::ExportAllResources() {
        return ExportAllResources(SR_UTILS_NS::Path());
    }

    bool ResourceEmbedder::ExportAllResources(const SR_UTILS_NS::Path& newDirectory) {
        bool result = true;
        SR_LOG("ResourceEmbedder::ExportAllResources() : exporting {} resources to '{}'", m_resources.size(), newDirectory);

        for (auto&& [path, data] : m_resources) {
            if (!ExportToFile(path, data, newDirectory)) {
                result = false;
            }
        }

        return result;
    }

    bool ResourceEmbedder::ExportToFile(std::string_view path, const Resource& resource, const SR_UTILS_NS::Path& newDirectory) {
        Path resourcePath = newDirectory.Concat(path);
        if (newDirectory.empty()) {
            resourcePath = path;
        }

        auto&& file = VFS::Instance().OpenFile(resourcePath, FileMode::Write);
        if (!file) {
            SR_ERROR("ResourceEmbedder::ExportToFile() : failed to open file '{}'!", resourcePath);
            return false;
        }

        std::string decompressedData = Decompress(resource);
        file.Write(decompressedData.data(), static_cast<int64_t>(decompressedData.size()));

        auto&& applicationPath = SR_PLATFORM_NS::GetApplicationPath();
        SR_PLATFORM_NS::CopyPermissions(applicationPath, resourcePath);

        return true;
    }

    bool ResourceEmbedder::ExportToFile(const SR_UTILS_NS::Path& path) {
        auto&& pIt = m_resources.find(path.View());
        if (pIt == m_resources.end()) {
            SR_ERROR("ResourceEmbedder::ExportToFile() : resource '{}' not found!", path);
            return false;
        }
        return ExportToFile(pIt->first, pIt->second, SR_UTILS_NS::Path());
    }

    bool ResourceEmbedder::ExportToMemory(std::string_view data) {
		SRHalt("Not yet implemented!");
        return false;
    }

    std::string ResourceEmbedder::HexToBytes(const std::string_view& hex) {
        if (hex.length() % 2 != 0) {
            throw std::runtime_error("Hex string length must be even!");
        }

        std::string result;
        result.reserve(hex.length() / 2);

        for (size_t i = 0; i < hex.length(); i += 2) {
            char byte = SR_UTILS_NS::HexCharToUInt8(hex[i], hex[i + 1]);
            result.push_back(byte);
        }

        return result;
    }

    std::string ResourceEmbedder::Decompress(const Resource& resource) {
    #ifdef SR_COMMON_ZLIB
        std::string compressedData = HexToBytes(resource.compressedHex);

        uLongf decompressedSize = resource.decompressedSize;
        std::vector<char> decompressedData(decompressedSize);

        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        int ret = inflateInit(&strm);
        if (ret != Z_OK) {
            SR_PLATFORM_NS::WriteConsoleError("InflateInit failed, error code: {}\n"_format(ret));
            SR_PLATFORM_NS::Terminate();
        }

        strm.avail_in = compressedData.size();
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressedData.data()));
        strm.avail_out = decompressedSize;
        strm.next_out = reinterpret_cast<Bytef*>(decompressedData.data());

        ret = inflate(&strm, Z_FINISH);
        if (ret != Z_STREAM_END) {
            inflateEnd(&strm);
            SR_PLATFORM_NS::WriteConsoleError("Failed to decompress data, error code: {}\n"_format(ret));
            SR_PLATFORM_NS::Terminate();
        }

        decompressedSize = strm.total_out;
        inflateEnd(&strm);

        return {decompressedData.begin(), decompressedData.begin() + decompressedSize };
    #else
        SRHalt("Decompression is not supported! Please define SR_COMMON_ZLIB to enable it.");
        return std::string();
    #endif
    }

    ResourceEmbedder& ResourceEmbedder::Instance() {
        static ResourceEmbedder instance;
        return instance;
    }
}