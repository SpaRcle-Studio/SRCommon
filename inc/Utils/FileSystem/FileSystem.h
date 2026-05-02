//
// Created by Nikita on 16.11.2020.
//

#ifndef HELPER_FILESYSTEM_H
#define HELPER_FILESYSTEM_H

#include <Utils/FileSystem/Path.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Types/Function.h>

namespace SR_UTILS_NS {
    class SR_COMMON_DLL_API FileSystem {
    public:
        FileSystem() = delete;
        ~FileSystem() = delete;
    public:
        static uint64_t GetExecutableAndModulesHash();

        static bool CreatePath(std::string path, uint32_t offset = 0);

        static bool IsAllowedPathSymbol(char c);
        static bool IsPathSeparator(char c);

        static bool WriteToFile(const Path& path, const std::string_view& text);
        static std::string NormalizePath(const std::string& path);
        static void NormalizePathInPlace(String& path);
        static std::vector<std::string> ReadAllLines(const SR_UTILS_NS::Path& path);

        static uint64_t ReadHashFromFile(const SR_UTILS_NS::Path& path);
        static bool WriteHashToFile(const SR_UTILS_NS::Path& path, uint64_t hash);

        static std::shared_ptr<String> ReadFileAsBlob(const Path& path);
        static bool ReadFile(const Path& path, String& buffer);
        static bool IsFileExists(const Path& path);

        static std::vector<std::string_view> ReadAllTextAsStringViewVector(const Path& path, String& buffer);

        static void ForEachFileInFolder(const Path& path, bool recursive, const SR_HTYPES_NS::Function<void(const Path&)>& func);

        static uint64_t GetFileHash(const Path& path);
        static uint64_t GetFolderHash(const Path& path, uint64_t deep = SR_UINT64_MAX);

        static const char* FileMapView(std::string path);
        static void UnmapFile(const char* str);
    };
}


#endif //HELPER_FILESYSTEM_H
