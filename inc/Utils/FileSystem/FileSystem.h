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

        static bool IsAllowedPathSymbol(char c);
        static bool IsPathSeparator(char c);

        static bool WriteToFile(StringView path, StringView data);
        static void NormalizePathInPlace(String& path);
        static Vector<StringView> ReadAllLines(const SR_UTILS_NS::Path& path, String& buffer);

        static uint64_t ReadHashFromFile(const SR_UTILS_NS::Path& path);
        static bool WriteHashToFile(const SR_UTILS_NS::Path& path, uint64_t hash);

        static std::shared_ptr<String> ReadFileAsBlob(const Path& path);
        static bool ReadFile(const Path& path, String& buffer);

        static uint64_t GetFileHash(const Path& path);
        static uint64_t GetFolderHash(const Path& path);
    };
}


#endif //HELPER_FILESYSTEM_H
