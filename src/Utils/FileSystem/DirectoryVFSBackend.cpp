//
// Created by Monika on 10.09.2026.
//

#include <Utils/FileSystem/DirectoryVFSBackend.h>
#include <Utils/FileSystem/File.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Memory/Allocator.h>
#include <Utils/Memory/MemoryLiterals.h>

#include <filesystem>
#include <system_error>

namespace SR_UTILS_NS {
    bool IsDangerousDeletionPath(std::string_view rawPath) {
        SR_TRACY_ZONE;

        // 1. Пустые и подозрительно короткие пути
        if (rawPath.empty() || rawPath.size() < 4) {
            return true;
        }

        // 2. Встроенный NUL: исключаем неоднозначную интерпретацию
        if (rawPath.find('\0') != std::string_view::npos) {
            return true;
        }

        // 3. Управляющие символы
        for (unsigned char c : rawPath) {
            if (c < 0x20 || c == 0x7F) {
                return true;
            }
        }

        // 4. Нормализация
        std::error_code ec;
        std::filesystem::path path = std::filesystem::absolute(std::filesystem::path(rawPath), ec);

        if (ec || path.empty()) {
            return true;
        }

        path = path.lexically_normal();

        // 5. Корень файловой системы
        if (path == path.root_path()) {
            return true;
        }

        // 6. Запрет слишком коротких путей
        size_t depth = 0;

        for (const auto& component : path.relative_path()) {
            if (!component.empty() && component != ".") {
                ++depth;
            }
        }

        // C:/Folder и C:/Folder/Subfolder запрещены
        if (depth < 3) {
            return true;
        }

        // 7. Приведение к единому виду для сравнения
        std::string normalized = path.generic_string();

    #ifdef SR_WIN32
        std::transform(normalized.begin(), normalized.end(), normalized.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); }
        );
    #endif

        while (normalized.size() > 1 && normalized.back() == '/') {
            normalized.pop_back();
        }

        // 8. Системные и особо важные директории
        static constexpr std::string_view dangerousPaths[] = {
        #ifdef SR_WIN32
            "c:/windows", "c:/users",
            "c:/program files", "c:/program files (x86)", "c:/programdata", "c:/recovery",
            "c:/system volume information", "c:/work", "c:/games",
        #else
            "/bin", "/boot", "/dev", "/etc", "/home",
            "/lib", "/lib64", "/opt",
            "/proc", "/root", "/run",
            "/sbin", "/sys", "/usr", "/var",
        #endif
        };

        for (std::string_view dangerous : dangerousPaths) {
            if (normalized == dangerous) {
                return true;
            }
        }

        // 9. Запрет удаления текущей рабочей директории
        // и любого её родительского каталога
        std::filesystem::path cwd = std::filesystem::current_path(ec);
        if (ec) {
            return true;
        }

        cwd = cwd.lexically_normal();

        auto IsSameOrParent = [](const std::filesystem::path& parent, const std::filesystem::path& child) {
            auto p = parent.begin();
            auto c = child.begin();

            for (; p != parent.end(); ++p, ++c) {
                if (c == child.end()) {
                    return false;
                }

            #ifdef SR_WIN32
                std::string a = p->string();
                std::string b = c->string();
                std::transform(a.begin(), a.end(), a.begin(), [](unsigned char x) { return static_cast<char>(std::tolower(x)); });
                std::transform(b.begin(), b.end(), b.begin(), [](unsigned char x) { return static_cast<char>(std::tolower(x)); });
                if (a != b) {
                    return false;
                }
            #else
                if (*p != *c) {
                    return false;
                }
            #endif
            }

            return true;
        };

        if (IsSameOrParent(path, cwd)) {
            return true;
        }

        // 10. Проверка существующего пути с разрешением ссылок
        std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path, ec);
        if (ec || canonicalPath.empty()) {
            return true;
        }

        // Если путь после разрешения ссылок отличается,
        // проверяем его повторно без рекурсии.
        if (canonicalPath != path) {
            if (canonicalPath == canonicalPath.root_path()) {
                return true;
            }

            if (IsSameOrParent(canonicalPath, cwd)) {
                return true;
            }

            // Консервативно запрещаем пути с переадресацией.
            return true;
        }

        return false;
    }

    static String gResolvedPathBuffer;

    void DirectoryVFSBackend::ResolveVirtualPath(StringView virtualPath, String& outRealPath) const {
        if (virtualPath == GetVirtualPath()) {
            outRealPath = m_realPath;
            return;
        }

        if (SR_PLATFORM_NS::IsAbsolutePath(virtualPath)) {
            outRealPath = virtualPath;
            return;
        }

        virtualPath.remove_prefix(GetVirtualPath().size());

        outRealPath = m_realPath;
        if (!outRealPath.ends_with('/') && !virtualPath.starts_with('/')) {
            outRealPath += '/';
        }
        outRealPath += virtualPath;
    }

    FSItemType DirectoryVFSBackend::GetType(StringView path) const {
        ResolveVirtualPath(path, gResolvedPathBuffer);
        return SR_PLATFORM_NS::GetPathType(gResolvedPathBuffer);
    }

    File DirectoryVFSBackend::OpenFile(StringView path, FileMode mode) const {
        ResolveVirtualPath(path, gResolvedPathBuffer);
        if (SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Write)) {
            SR_PLATFORM_NS::CreateDirectories(StringUtils::GetDirToFileFromFullPath(gResolvedPathBuffer));
        }

        FileImpl* pImpl = nullptr;
        if (SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Map)) {
            pImpl = new MappedFileImpl();
        }
        else {
            pImpl = new StreamFileImpl();
        }

        if (auto&& file = File(pImpl, gResolvedPathBuffer, mode)) {
            return file;
        }
        return File();
    }

    void DirectoryVFSBackend::Enumerate(StringView directory, const IVFSBackend::EnumerateCallback& callback, bool recursive) const {
        SR_GLOBAL_LOCK;

        static String gEnumerateResolvedPathBuffer;
        ResolveVirtualPath(directory, gEnumerateResolvedPathBuffer);
        if (gEnumerateResolvedPathBuffer.empty()) {
            SRHalt("DirectoryVFSBackend::Enumerate() : resolved path is empty!");
            return;
        }

        /// бекенд может быть примонтирован поверх другого и содержать лишь часть дерева,
        /// поэтому отсутствие директории - штатная ситуация, а не ошибка
        if (SR_PLATFORM_NS::GetPathType(gEnumerateResolvedPathBuffer) != FSItemType::Folder) {
            return;
        }

        static String fullPathBuffer;
        static String relativePathBuffer;

        std::error_code errorCode;

        auto&& processEntry = [&](const std::filesystem::directory_entry& entry) {
            const bool isFile = entry.is_regular_file(errorCode);
            const bool isDirectory = entry.is_directory(errorCode);
            if (!isFile && !isDirectory) {
                return ;
            }

            using PathString = std::filesystem::path::string_type;
            const PathString& native = entry.path().native();
            std::basic_string_view<PathString::value_type> view(native.data(), native.size());

            if constexpr (std::is_same_v<PathString::value_type, char>) {
                fullPathBuffer = StringView(reinterpret_cast<const char*>(view.data()), view.size());
            }
            else {
                StringUtils::Instance().ConvertToUTF8(reinterpret_cast<const wchar_t*>(view.data()), view.size(), fullPathBuffer);
            }

            std::ranges::replace(fullPathBuffer, '\\', '/');

            relativePathBuffer = fullPathBuffer;
            if (!m_realPath.empty()) {
                relativePathBuffer.remove_prefix(m_realPath.size() + 1); // +1 to remove the trailing slash
            }
            /// у корневого бекенда виртуальный путь пуст, и ведущий слеш сделал бы путь абсолютным
            if (!GetVirtualPath().empty()) {
                relativePathBuffer.insert(0, GetVirtualPath());
                relativePathBuffer.insert(GetVirtualPath().size(), "/");
            }

            StringView name = relativePathBuffer;
            if (auto&& slashPos = name.find_last_of('/'); slashPos != StringView::npos) {
                name.remove_prefix(slashPos + 1);
            }

            StringView extension = name;
            if (auto&& dotPos = extension.find_last_of('.'); dotPos != StringView::npos) {
                extension.remove_prefix(dotPos + 1);
            }
            else {
                extension = {};
            }

            VFSEntry vfsEntry;
            vfsEntry.name = name;
            vfsEntry.extension = extension;
            vfsEntry.fullPath = fullPathBuffer;
            vfsEntry.relativePath = relativePathBuffer;
            vfsEntry.type = isFile ? FSItemType::File : FSItemType::Folder;

            callback(vfsEntry);
        };

        static SR_UTILS_NS::MonotonicAllocator gEnumerateAllocator(8_KB);
        gEnumerateAllocator.ResetMemory();
        SR_UTILS_NS::SetThreadLocalAllocator(&gEnumerateAllocator);

        Vector<std::filesystem::directory_entry> entries(&gEnumerateAllocator);
        entries.reserve(16);

        bool hasError = false;

        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(gEnumerateResolvedPathBuffer.view(), errorCode)) {
                if (errorCode) {
                    hasError = true;
                    break;
                }
                entries.emplace_back(entry);
            }
        }
        else {
            for (const auto& entry : std::filesystem::directory_iterator(gEnumerateResolvedPathBuffer.view(), errorCode)) {
                if (errorCode) {
                    hasError = true;
                    break;
                }
                entries.emplace_back(entry);
            }
        }

        SR_UTILS_NS::SetThreadLocalAllocator(nullptr);

        if (hasError) {
            SR_ERROR("DirectoryVFSBackend::Enumerate() : failed to enumerate directory \"{}\"! Error: {}", gEnumerateResolvedPathBuffer, errorCode.message());
            SR_UTILS_NS::SetThreadLocalAllocator(&gEnumerateAllocator);
            entries = {};
            SR_UTILS_NS::SetThreadLocalAllocator(nullptr);
            return;
        }

        for (const auto& entry : entries) {
            processEntry(entry);
        }

        SR_UTILS_NS::SetThreadLocalAllocator(&gEnumerateAllocator);
        entries = {};
        SR_UTILS_NS::SetThreadLocalAllocator(nullptr);
    }

    void DirectoryVFSBackend::Delete(StringView path) const {
        ResolveVirtualPath(path, gResolvedPathBuffer);
        if (gResolvedPathBuffer.empty()) {
            SRHalt("DirectoryVFSBackend::Delete() : resolved path is empty!");
            return;
        }
        std::string_view pathToDelete = gResolvedPathBuffer;
        SR_LOG("DirectoryVFSBackend::Delete() : deleting path \"{}\"...", pathToDelete);

        if (IsDangerousDeletionPath(pathToDelete)) {
            SRHaltTerminate("DirectoryVFSBackend::Delete() : destructive operation detected!\n\tPath: {}\n\tPath to delete: {}", path, pathToDelete);
            return;
        }

        std::error_code errorCode;
        std::filesystem::remove_all(pathToDelete, errorCode);
        if (errorCode) {
            SR_ERROR("DirectoryVFSBackend::Delete() : failed to delete path \"{}\"! Error: {}", path, errorCode.message());
        }
    }
}