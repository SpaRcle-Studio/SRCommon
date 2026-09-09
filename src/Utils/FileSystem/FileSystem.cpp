//
// Created by Nikita on 16.11.2020.
//

#include <Utils/FileSystem/FileSystem.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/FileSystem/MappedFile.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Common/Hashes.h>
#include <Utils/Common/StringAtomLiterals.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Types/Vector.h>

namespace SR_UTILS_NS {
    bool FileSystem::ReadFile(const Path& path, String& buffer) {
        SR_TRACY_ZONE;
        buffer.clear();
        if (auto&& file = VFS::Instance().OpenFile(path, FileMode::Read)) {
            file.Read(buffer);
            return true;
        }
        return false;
    }

    bool FileSystem::WriteToFile(StringView path, StringView data) {
        SR_TRACY_ZONE;
        auto&& file = VFS::Instance().OpenFile(path, FileMode::Write);
        if (!file) {
            SR_ERROR("FileSystem::WriteToFile() : failed to open file for writing!\n\tPath: {}\n\tError: {}"_format(path, GetErrorString(errno)));
            return false;
        }
        file.Write(data.data(), data.size());
        return true;
    }

    uint64_t FileSystem::GetFileHash(const Path& path) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT_VIEW(path.View());

        auto&& mappedFile = VFS::Instance().OpenFile(path, FileMode::ReadMap);
        if (!mappedFile) {
            SR_WARN("FileSystem::GetFileHash() : failed to read file!\n\tPath: {}"_format(path));
            return SR_UINT64_MAX;
        }

        {
            SR_TRACY_ZONE_N("Hash file");
            return SR_HASH_STR_VIEW(mappedFile.Data());
        }
    }

    uint64_t FileSystem::GetFolderHash(const Path& path) {
        SR_TRACY_ZONE;
        uint64_t hash = 0;
        VFS::Instance().Enumerate(path, [&](const VFSEntry& entry) {
            if (entry.type == FSItemType::File) {
                auto&& fileHash = GetFileHash(entry.fullPath);
                hash = CombineTwoHashes(hash, fileHash);
            }
        }, true);
        return hash;
    }

    SR_COMMON_DLL_API std::shared_ptr<String> FileSystem::ReadFileAsBlob(const Path& path) {
        SR_TRACY_ZONE;
        std::shared_ptr<String> pBuffer = std::make_shared<String>();
        if (!FileSystem::ReadFile(path, *pBuffer)) {
            SR_ERROR("FileSystem::ReadFileAsBlob() : failed to read file!\n\tPath: {}", path);
            return nullptr;
        }
        return pBuffer;
    }

    SR_COMMON_DLL_API uint64_t FileSystem::ReadHashFromFile(const Path& path) {
        SR_TRACY_ZONE;
        if (auto&& file = VFS::Instance().OpenFile(path, FileMode::Read)) {
            uint64_t hash = 0;
            file.Read((char*)&hash, sizeof(uint64_t));
            return hash;
        }
        return 0;
    }

    SR_COMMON_DLL_API bool FileSystem::WriteHashToFile(const Path& path, uint64_t hash) {
        SR_TRACY_ZONE;
        if (auto&& file = VFS::Instance().OpenFile(path, FileMode::Write)) {
            file.Write((char*)&hash, sizeof(uint64_t));
            return true;
        }
        return false;
    }

    Vector<StringView> FileSystem::ReadAllLines(const Path& path, String& buffer) {
        SR_TRACY_ZONE;
        if (auto&& file = VFS::Instance().OpenFile(path, FileMode::Read)) {
            file.Read(buffer);
            Vector<StringView> lines;
            StringUtils::Instance().SplitViewByLines(buffer, lines);
            return lines;
        }
        return {};
    }

    uint64_t FileSystem::GetExecutableAndModulesHash() {
        SR_TRACY_ZONE;

        if (SR_PLATFORM_NS::IsMobilePlatform()) {
            return 0;
        }

        static uint64_t hash = SR_UINT64_MAX;
        if (hash != SR_UINT64_MAX) {
            return hash;
        }

        auto&& appPath = SR_PLATFORM_NS::GetApplicationPath();
        if (!appPath.IsFile()) {
            SR_ERROR("FileSystem::GetExecutableAndModulesHash() : application path is not a file!");
            return 0;
        }

        hash = GetFileHash(appPath);
        Vector<Path> files;
        appPath.GetFolder().GetFiles(files);
        for (auto&& filePath : files) {
            if (filePath.GetExtensionView() == "dll" || filePath.GetExtensionView() == "so" || filePath.GetExtensionView() == "dylib") {
                auto&& fileHash = GetFileHash(filePath);
                if (fileHash != SR_UINT64_MAX) {
                    hash = CombineTwoHashes(hash, fileHash);
                }
            }
        }

        return hash;
    }

    void FileSystem::NormalizePathInPlace(String& path) {
        SR_TRACY_ZONE;

        if (path.empty()) {
            return;
        }

        // заменяем все \ на /
        for (char& c : path) {
            if (c == '\\') c = '/';
        }

        size_t read = 0;   // откуда читаем сегменты
        size_t write = 0;  // куда пишем результат
        bool absolute = false;
        bool hasDrive = false;

        // обрабатываем префикс (C: или /)
        if (path.size() >= 2 && std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':') {
            // "C:" копируем как есть
            path[write++] = path[0];
            path[write++] = path[1];
            read = 2;
            hasDrive = true;
            if (read < path.size() && path[read] == '/') {
                read++;
                path[write++] = '/';
            }
        }
        else if (!path.empty() && path[0] == '/') {
            absolute = true;
            read = 1;
            path[write++] = '/';
        }

        // стек: индексы начала сегментов внутри path
        SR_THREAD_LOCAL static SR_UTILS_NS::SmallStack<uint16_t, 256> segments;
        segments.clear();

        while (read <= path.size()) {
            size_t start = read;
            size_t end = path.find('/', read);
            if (end == std::string::npos) end = path.size();
            size_t len = end - start;

            if (len > 0) {
                const char* token = path.data() + start;

                if (len == 1 && token[0] == '.') {
                    // "." → пропускаем
                }
                else if (len == 2 && token[0] == '.' && token[1] == '.') {
                    // ".." → откатываем последний сегмент
                    if (!segments.empty()) {
                        write = segments.back();
                        segments.pop();
                    }
                    else if (!absolute && !hasDrive) {
                        // относительный путь — оставляем ".."
                        if (write && path[write - 1] != '/') {
                            path[write++] = '/';
                        }
                        segments.push(write);
                        std::memmove(&path[write], token, len);
                        write += len;
                    }
                }
                else {
                    // обычный сегмент
                    if (write && path[write - 1] != '/') {
                        path[write++] = '/';
                    }
                    segments.push(write);
                    std::memmove(&path[write], token, len);
                    write += len;
                }
            }

            read = end + 1;
        }

        if (write == 0) {
            if (absolute) {
                path[0] = '/';
            } else {
                path[0] = '.';
            }
            write = 1;
        }
        path.resize(write);
    }

    bool FileSystem::IsAllowedPathSymbol(char c) {
        if (std::isalnum(c)) {
            return true; // A-Z, a-z, 0-9
        }
        switch (c) {
            case '/': case '\\': case ':': // разделители + диск (Windows)
            case '.': case '_': case '-':
            case ' ': case '(': case ')':
            case '[': case ']': case '{': case '}':
            case '+': case '=': case '!': case '@':
            case '#': case '$': case '%': case '&':
            case '\'': case '~': case '`': case '^':
            case ',': case ';':
                return true;
            default:
                return false;
        }
    }

    bool FileSystem::IsPathSeparator(char c) {
        return c == '/' || c == '\\';
    }
}