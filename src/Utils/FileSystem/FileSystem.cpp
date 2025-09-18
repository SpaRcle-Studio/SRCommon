//
// Created by Nikita on 16.11.2020.
//

#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Debug.h>
#include <Utils/Platform/Platform.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Common/Hashes.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    SR_COMMON_DLL_API void FileSystem::UnmapFile(const char *str) {

    }

    SR_COMMON_DLL_API const char *FileSystem::FileMapView(std::string path) {
        const char *data = nullptr;

        return data;
    }

    SR_COMMON_DLL_API char* FileSystem::Load(std::string path) {
        /// open file
        std::ifstream infile(path);
        if (!infile.is_open()) {
            SR_ERROR("FileSystem::Load() : failed open file!\n\tPath: "+path);
            return nullptr;
        }


        /// get length of file
        infile.seekg(0, std::ios::end);
        size_t length = infile.tellg();
        infile.seekg(0, std::ios::beg);

        char* buffer = new char[length];

        /// read file
        infile.read(buffer, length);

        return buffer;
    }

    SR_COMMON_DLL_API std::vector<std::string_view> FileSystem::ReadAllTextAsStringViewVector(const Path& path, std::string& buffer) {
        SR_TRACY_ZONE;
        std::vector<std::string_view> result;

        if (auto&& data = SR_PLATFORM_NS::ReadFile(path)) {
            buffer = data.value();
        }
        else {
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

    SR_COMMON_DLL_API std::string FileSystem::ReadAllText(const std::string& path) {
        SR_TRACY_ZONE;

        std::string data = std::string();
        std::ifstream stream(path, std::ios::in);
        if (!stream) {
            SR_ERROR("FileSystem::ReadAllText() : failed to open \"" + path + "\" file!");
            return data;
        }

        stream.seekg(0, std::ios::end);
        std::streampos bytes = stream.tellg();
        stream.seekg(0, std::ios::beg);
        data.reserve(bytes);

        if (stream.is_open()) {
            std::string line;
            bool first = false;
            while (getline(stream, line)) {
                if (!first) {
                    first = true;
                    data += line;
                }
                else {
                    data += "\n";
                    data += line;
                }
            }
            stream.close();
        }
        return data;
    }

    SR_COMMON_DLL_API std::vector<uint8_t> FileSystem::ReadFileAsVector(const std::string &path) {
        std::ifstream file(path, std::ifstream::binary | std::ios::in);

        if (file.fail()) {
            return std::vector<uint8_t>();
        }

        file.seekg(0, std::ios::end);
        std::streampos end = file.tellg();
        file.seekg(0, std::ios::beg);
        std::streampos start = file.tellg();
        size_t size = static_cast<size_t>(end - start);

        std::vector<uint8_t> result(size);

        file.read(reinterpret_cast<char*>(result.data()), size);

        return result;
    }

    SR_COMMON_DLL_API std::vector<char> FileSystem::ReadBinary(const std::string_view path) {
        /*std::ifstream ifd(path,  std::ios::binary |  std::ios::ate);
        int size = ifd.tellg();
        ifd.seekg(0,  std::ios::beg);
        std::vector<char> buffer;
        buffer.resize(size); // << resize not reserve
        ifd.read(buffer.data(), size);*/

        //std::ifstream input(path, std::ios::binary);
        //std::vector<uint32_t> buffer(std::istreambuf_iterator<char>(input), {});

        std::ifstream file(path.data(), std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            SR_ERROR("FileSystem::ReadBinary() : failed to open \"{}\"file!", path);
            return std::vector<char>();
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    SR_COMMON_DLL_API bool FileSystem::CreatePath(std::string path, uint32_t offset) {
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

    //std::string FileSystem::GetFullPath(const std::string& path) {
    //#ifdef SR_WIN32
    //    char fullFilename[MAX_PATH];
    //    GetFullPathName(path.c_str(), MAX_PATH, fullFilename, nullptr);
    //    return std::string(fullFilename);
    //#else
    //    return std::string();
    //#endif
    //}

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

    SR_COMMON_DLL_API bool FileSystem::WriteToFile(const std::string& path, const std::string_view& text) {
        std::ofstream stream(path);
        if (!stream.is_open()) {
            return false;
        }
        stream << text;
        stream.close();

        return true;
    }

    SR_COMMON_DLL_API std::string FileSystem::ReadBinaryAsString(const Path& path, bool checkError) {
        SR_TRACY_ZONE;

        std::ifstream file(path.ToStringRef(), std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            SR_UNUSED_VARIABLE(checkError);
            SRAssert2(!checkError, "FileSystem::ReadBinaryAsString() : failed to open \"" + path.ToStringRef() + "\" file!");
            return std::string();
        }

        size_t fileSize = (size_t) file.tellg();
        std::string buffer;
        buffer.resize(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    SR_COMMON_DLL_API uint64_t FileSystem::GetFileHash(const std::string& path) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(path);

        std::vector<char> buffer;
        {
            SR_TRACY_ZONE_N("Read file");

            FILE* f = fopen(path.c_str(), "rb");
            if (!f) {
                SR_WARN("FileSystem::GetFileHash() : failed to read file!\n\tPath: " + path);
                return SR_UINT64_MAX;
            }

            fseek(f, 0, SEEK_END);
            size_t size = ftell(f);
            fseek(f, 0, SEEK_SET);

            buffer.resize(size);
            fread(buffer.data(), 1, size, f);
            fclose(f);
        }

        {
            SR_TRACY_ZONE_N("Hash file");
            return SR_HASH_STR_VIEW(std::string_view(buffer.data(), buffer.size()));
        }
    }

    SR_COMMON_DLL_API uint64_t FileSystem::GetFolderHash(const Path& path, uint64_t deep) {
        if (deep == 0) {
            return 0;
        }

        SR_TRACY_ZONE;

        uint64_t hash = 0;

        for (auto&& subPath : Platform::GetInDirectory(path, Path::Type::Undefined)) {
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

    SR_COMMON_DLL_API std::shared_ptr<std::vector<uint8_t>> FileSystem::ReadFileAsBlob(const std::string &path) {
        return std::make_shared<std::vector<uint8_t>>(std::move(ReadFileAsVector(path)));
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

    SR_COMMON_DLL_API void FileSystem::ForEachFileInFolder(const Path& path, bool recursive, const Types::Function<void(const Path&)>& func) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(path.ToStringRef());

        /// scope for files
        {
            auto&& files = path.GetFiles();
            for (auto&& file : files) {
                func(file);
            }
        }

        if (recursive) {
            auto&& folders = path.GetFolders();
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

        static uint64_t hash = SR_UINT64_MAX;
        if (hash != SR_UINT64_MAX) {
            return hash;
        }

        auto&& appPath = SR_PLATFORM_NS::GetApplicationPath();
        if (!appPath.IsFile()) {
            SR_ERROR("FileSystem::GetExecutableAndModulesHash() : application path is not a file!");
            return 0;
        }

        hash = GetFileHash(appPath.ToStringRef());
        for (auto&& filePath : appPath.GetFolder().GetFiles()) {
            if (filePath.GetExtensionView() == "dll" || filePath.GetExtensionView() == "so" || filePath.GetExtensionView() == "dylib") {
                auto&& fileHash = GetFileHash(filePath.ToStringRef());
                if (fileHash != SR_UINT64_MAX) {
                    hash = CombineTwoHashes(hash, fileHash);
                }
            }
        }

        return hash;
    }

    void FileSystem::NormalizePathInPlace(std::string& path) {
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
        SR_UTILS_NS::SmallStack<size_t, 256> segments;

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
}