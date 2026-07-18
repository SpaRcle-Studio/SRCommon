//
// Created by Nikita on 16.11.2020.
//

#include <Utils/FileSystem/FileSystem.h>
#include <Utils/FileSystem/VirtualFS.h>
#include <Utils/FileSystem/MappedFile.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Common/Hashes.h>
#include <Utils/Common/StringAtomLiterals.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    SR_COMMON_DLL_API std::vector<std::string_view> FileSystem::ReadAllTextAsStringViewVector(const Path& path, String& buffer) {
        SR_TRACY_ZONE;
        std::vector<std::string_view> result;

        if (!SR_UTILS_NS::FileSystem::ReadFile(path, buffer)) {
            return result;
        }

        result.reserve(buffer.size() / 32); // Предварительно резервируем память для строк

        // Разбиваем buffer на строки, каждая строка — отдельный string_view
        const char* start = buffer.data();
        const char* end = buffer.data() + buffer.size();
        const char* line_start = start;

        for (const char* ptr = start; ptr < end; ++ptr) {
            if (*ptr == '\n') {
                size_t len = ptr - line_start;
                if (len > 0 && *(ptr - 1) == '\r') {
                    --len;  // Убираем \r для Windows-строк
                }
                result.emplace_back(line_start, len);
                line_start = ptr + 1;
            }
        }

        // Последняя строка (если файл не заканчивается на \n)
        if (line_start < end) {
            size_t len = end - line_start;
            result.emplace_back(line_start, len);
        }

        return result;
    }

    SR_COMMON_DLL_API bool FileSystem::CreatePath(std::string path, SizeType offset) {
        if (path.empty()) {
            return false;
        }

        if (path.back() != '/')
            path.append("/");

    #if defined(SR_LINUX) || defined(SR_ANDROID)
        return SR_PLATFORM_NS::CreateFolder(path);
    #else
        auto pos = path.find('/', offset);
        if (pos != std::string::npos) {
            auto&& dir = path.substr(0, pos);
            if (dir.back() != '/') {
                dir.append("/");

            }

            SR_PLATFORM_NS::CreateFolder(dir);
            return CreatePath(std::move(path), pos + 1);
        }

        return true;
    #endif
    }

    SR_COMMON_DLL_API bool FileSystem::ReadFile(const Path& path, String& buffer) {
        SR_TRACY_ZONE;
        return SR_PLATFORM_NS::ReadFile(path, buffer);
    }

    SR_COMMON_DLL_API bool FileSystem::IsFileExists(const Path& path) {
        SR_TRACY_ZONE;
        return path.IsFile();
    }

    SR_COMMON_DLL_API std::string FileSystem::NormalizePath(const std::string &path) {
        SR_TRACY_ZONE;

        auto newPath = StringUtils::MakePath(path);

        do {
            auto pos = newPath.find("/..");
            if (pos == std::string::npos)
                break;

            auto splash = newPath.rfind('/', pos - 1);
            if (splash == std::string::npos)
                break;

            newPath = newPath.erase(splash + 1, (pos - splash) + 3);
        } while(true);

        return newPath;
    }

    SR_COMMON_DLL_API bool FileSystem::WriteToFile(const Path& path, const std::string_view& text) {
        SR_TRACY_ZONE;
        std::ofstream stream(path.c_str());
        if (!stream.is_open()) {
            SR_ERROR("FileSystem::WriteToFile() : failed to open file for writing!\n\tPath: {}\n\tError: {}"_format(path, GetErrorString(errno)));
            return false;
        }
        stream << text;
        stream.close();

        return true;
    }

    SR_COMMON_DLL_API uint64_t FileSystem::GetFileHash(const Path& path) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT_VIEW(path.View());

        MappedFile mappedFile = MappedFile::Open(path);
        if (!mappedFile) {
            SR_WARN("FileSystem::GetFileHash() : failed to read file!\n\tPath: {}"_format(path));
            return SR_UINT64_MAX;
        }

        {
            SR_TRACY_ZONE_N("Hash file");
            return SR_HASH_STR_VIEW(mappedFile.GetDataView());
        }
    }

    uint64_t FileSystem::GetFolderHash(const Path& path, uint64_t deep) {
        if (deep == 0) {
            return 0;
        }

        SR_TRACY_ZONE;

        uint64_t hash = 0;

        Vector<Path> subPaths;
        Platform::GetInDirectory(path, Path::Type::Undefined, subPaths);
        for (auto&& subPath : subPaths) {
            if (subPath.IsHidden()) {
                continue;
            }

            if (subPath.IsFile()) {
                auto&& fileHash = GetFileHash(subPath);
                hash = CombineTwoHashes(hash, fileHash);
            }
            else if (subPath.IsDir()) {
                hash = SR_UTILS_NS::CombineTwoHashes(subPath.GetFolderHash(deep - 1), hash);
            }
        }

        return hash;
    }

    SR_COMMON_DLL_API std::shared_ptr<String> FileSystem::ReadFileAsBlob(const Path& path) {
        SR_TRACY_ZONE;

        std::shared_ptr<String> pBuffer = std::make_shared<String>();
        if (!SR_UTILS_NS::FileSystem::ReadFile(path, *pBuffer)) {
            SR_ERROR("FileSystem::ReadFileAsBlob() : failed to read file!\n\tPath: {}", path);
            return nullptr;
        }

        return pBuffer;
    }

    SR_COMMON_DLL_API uint64_t FileSystem::ReadHashFromFile(const Path& path) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(path.ToStringRef());

        std::ifstream file(path.ToString(), std::ios::binary);

        if (!file.is_open()) {
            return 0;
        }

        uint64_t hash = 0;
        file.read((char*)&hash, sizeof(uint64_t));
        file.close();

        return hash;
    }

    SR_COMMON_DLL_API bool FileSystem::WriteHashToFile(const Path& path, uint64_t hash) {
        SR_TRACY_ZONE;

        if (!path.Create()) {
            SR_ERROR("FileSystem::WriteHashToFile() : failed to create file!\n\tPath: " + path.ToString());
            return false;
        }

        std::ofstream file(path.ToString(), std::ios::binary);

        if (!file.is_open()) {
            SR_ERROR("FileSystem::WriteHashToFile() : failed to open file!\n\tPath: " + path.ToString());
            return false;
        }

        file.write((char*)&hash, sizeof(uint64_t));
        file.close();

        return true;
    }

    void FileSystem::ForEachFileInFolder(const Path& path, bool recursive, const Types::Function<void(const Path&)>& func) {
        SR_TRACY_ZONE;

        /// scope for files
        {
            static SR_THREAD_LOCAL Vector<Path> files;
            path.GetFiles(files);
            for (auto&& file : files) {
                func(file);
            }
        }

        if (recursive) {
            Vector<Path> folders;
            path.GetFolders(folders);
            for (auto&& folder: folders) {
                ForEachFileInFolder(folder, recursive, func);
            }
        }
    }

    std::vector<std::string> FileSystem::ReadAllLines(const Path &path) {
        std::ifstream file(path.c_str());
        std::vector<std::string> lines = { };
        while (file.good()) {
            std::string line;
            std::getline(file,line);
            lines.push_back(line);
        }
        return lines;
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