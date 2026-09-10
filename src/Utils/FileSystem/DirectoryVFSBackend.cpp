//
// Created by Monika on 10.09.2026.
//

#include <Utils/FileSystem/DirectoryVFSBackend.h>
#include <Utils/FileSystem/File.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Common/StringUtils.h>

#include <filesystem>
#include <system_error>

namespace SR_UTILS_NS {
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

        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(gEnumerateResolvedPathBuffer.view(), errorCode)) {
                if (errorCode) {
                    SR_ERROR("DirectoryVFSBackend::Enumerate() : failed to enumerate directory \"{}\"! Error: {}", gEnumerateResolvedPathBuffer, errorCode.message());
                    return;
                }
                processEntry(entry);
            }
        }
        else {
            for (const auto& entry : std::filesystem::directory_iterator(gEnumerateResolvedPathBuffer.view(), errorCode)) {
                if (errorCode) {
                    SR_ERROR("DirectoryVFSBackend::Enumerate() : failed to enumerate directory \"{}\"! Error: {}", gEnumerateResolvedPathBuffer, errorCode.message());
                    return;
                }
                processEntry(entry);
            }
        }
    }

    void DirectoryVFSBackend::Delete(StringView path) const {
        ResolveVirtualPath(path, gResolvedPathBuffer);
        if (gResolvedPathBuffer.empty()) {
            SRHalt("DirectoryVFSBackend::Delete() : resolved path is empty!");
            return;
        }
        std::string_view pathToDelete = gResolvedPathBuffer;
        SR_LOG("DirectoryVFSBackend::Delete() : deleting path \"{}\"...", pathToDelete);

        std::error_code errorCode;
        std::filesystem::remove_all(pathToDelete, errorCode);
        if (errorCode) {
            SR_ERROR("DirectoryVFSBackend::Delete() : failed to delete path \"{}\"! Error: {}", path, errorCode.message());
        }
    }
}