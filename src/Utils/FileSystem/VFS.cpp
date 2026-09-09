//
// Created by Monika on 02.03.2026.
//

#include <Utils/FileSystem/VFS.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Types/LockGuard.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Memory/MemoryLiterals.h>
#include <Utils/Memory/Allocator.h>

#include <Codegen/VFS.generated.hpp>

#include <filesystem>
#include <system_error>

namespace SR_UTILS_NS {
    static String gResolvedPathBuffer;
    static String gEnumerateResolvedPathBuffer;

    bool IVFSBackend::IsApplicable(StringView path) const {
        SR_TRACY_ZONE;

        if (!m_ignoredExtensions.empty()) {
            if (auto&& dotPos = path.find_last_of('.'); dotPos != StringView::npos) {
                StringView extension = path.substr(dotPos + 1);
                if (m_ignoredExtensions.contains(extension)) {
                    return false;
                }
            }
        }

        if (SR_PLATFORM_NS::IsAbsolutePath(path)) {
            if (path.starts_with(m_realPath)) {
                return true;
            }
            if (m_virtualPath.empty()) {
                return false;
            }
            return path.starts_with(m_virtualPath);
        }
        if (m_virtualPath.empty()) {
            return true;
        }
        return path.starts_with(m_virtualPath);
    }

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
        ResolveVirtualPath(directory, gEnumerateResolvedPathBuffer);
        if (gEnumerateResolvedPathBuffer.empty()) {
            SRHalt("DirectoryVFSBackend::Enumerate() : resolved path is empty!");
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
            relativePathBuffer.insert(0, GetVirtualPath());
            relativePathBuffer.insert(GetVirtualPath().size(), "/");

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

    bool VFS::IsExists(StringView path) const {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        return std::ranges::any_of(m_mounts, [&](const VFSMount& mount) {
            return mount.pBackend->IsApplicable(path) && mount.pBackend->GetType(path) != FSItemType::Undefined;
        });
    }

    bool VFS::IsFileExists(StringView path) const {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        return std::ranges::any_of(m_mounts, [&](const VFSMount& mount) {
            return mount.pBackend->IsApplicable(path) && mount.pBackend->GetType(path) == FSItemType::File;
        });
    }

    bool VFS::IsFolderExists(StringView path) const {
        SR_LOCK_GUARD;
        return std::ranges::any_of(m_mounts, [&](const VFSMount& mount) {
            return mount.pBackend->IsApplicable(path) && mount.pBackend->GetType(path) == FSItemType::Folder;
        });
    }

    File VFS::OpenFile(const Path& path, FileMode mode) {
        return OpenFile(path.ToStringView(), mode);
    }

    File VFS::OpenFile(const String& path, FileMode mode) {
        return OpenFile(StringView(path), mode);
    }

    File VFS::OpenFile(StringView path, FileMode mode) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        const bool isRead = SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Read);
        const bool isWrite = SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Write);

        for (auto&& mount : m_mounts) {
            if (isRead && !mount.pBackend->ReadSupports()) {
                continue;
            }

            if (isWrite && !mount.pBackend->WriteSupports()) {
                continue;
            }

            if (mount.pBackend->IsApplicable(path)) {
                if (isRead && mount.pBackend->GetType(path) != FSItemType::File) {
                    continue;
                }
                return mount.pBackend->OpenFile(path, mode);
            }
        }
        return File();
    }

    void VFS::CloseFile(PassKey<FileImpl>, FileImpl* pImpl) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        pImpl->Close();
        if (!m_handles.Remove(pImpl)) {
            SRHalt("VFS::CloseFile() : failed to remove handle!");
        }
        delete pImpl;
    }

    void VFS::RegisterHandle(PassKey<File>, FileImpl* pImpl) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        if (!m_handles.Add(pImpl)) {
            SRHalt("VFS::RegisterHandle() : failed to add handle!");
        }
    }

    void VFS::Enumerate(StringView directory, const SR_HTYPES_NS::Function<void(const VFSEntry&)>& callback, bool recursive) const {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        if (!m_enumerateAllocator) {
            m_enumerateAllocator = (IAllocator*)new MonotonicAllocator(32_KB);
        }

        SR_HTYPES_NS::SortedVector<StringView> paths(m_enumerateAllocator.Get());

        auto&& callbackWrapper = [&](const VFSEntry& entry) {
            if (paths.Contains(entry.relativePath)) {
                return;
            }

            String relativePath(m_enumerateAllocator.Get());
            relativePath = entry.relativePath;

            paths.Add(relativePath);
            callback(entry);
        };

        for (auto&& mount : m_mounts) {
            if (mount.pBackend->IsApplicable(directory)) {
                mount.pBackend->Enumerate(directory, callbackWrapper, recursive);
            }
        }

        paths = {};
        m_enumerateAllocator->ResetMemory();
    }

    void VFS::Delete(StringView path) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        SR_LOG("VFS::Delete() : deleting path \"{}\"...", path);

        for (auto&& mount : m_mounts) {
            if (!mount.pBackend->WriteSupports()) {
                continue;
            }
            if (mount.pBackend->IsApplicable(path)) {
                if (mount.pBackend->GetType(path) == FSItemType::Undefined) {
                    continue;
                }
                mount.pBackend->Delete(path);
            }
        }
    }

    void VFS::Mount(StringView virtualPath, IVFSBackend* pBackend, int32_t priority, bool permanent) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        SR_LOG("VFS::Mount() : mounting virtual path \"{}\" with priority {}...", virtualPath, priority);
        pBackend->SetVirtualPath({}, virtualPath);
        m_mounts.emplace_back(VFSMount{ pBackend, priority, permanent });
        std::stable_sort(m_mounts.begin(), m_mounts.end(), [](const VFSMount& lhs, const VFSMount& rhs) {
            return lhs.priority > rhs.priority;
        });
    }

    void VFS::Unmount(IVFSBackend* pBackend) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        SR_LOG("VFS::Unmount() : unmounting virtual path \"{}\"...", pBackend->GetVirtualPath());
        for (auto&& it = m_mounts.begin(); it != m_mounts.end(); ++it) {
            if (it->pBackend == pBackend) {
                m_mounts.erase(it);
                return;
            }
        }
        SRHalt("VFS::Unmount() : backend not found!");
    }

    void VFS::DeInitialize() {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        for (auto&& mount : m_mounts) {
            if (mount.pBackend) {
                delete mount.pBackend;
            }
        }

        if (!m_handles.empty()) {
            SRHalt("VFS::DeInitialize() : there are still open handles!");
            for (auto&& pHandle : m_handles) {
                pHandle->Close();
                delete pHandle;
            }
            m_handles.clear();
        }

        m_mounts.clear();
    }

    bool VFS::IsSingletonCanBeDestroyed() const {
        return false;
    }

    void VFS::UnmountAll() {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        Vector<IVFSBackend*> unmountable;
        unmountable.reserve(m_mounts.size());
        for (auto&& mount : m_mounts) {
            if (!mount.permanent) {
                unmountable.emplace_back(mount.pBackend);
            }
        }
        for (auto&& pBackend : unmountable) {
            Unmount(pBackend);
        }
    }

    bool VFS::HaveMount(StringView virtualPath) const {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        return std::ranges::any_of(m_mounts, [&](const VFSMount& mount) {
            return virtualPath == mount.pBackend->GetVirtualPath();
        });
    }

    void VFS::ResolvePath(String& path) const {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        static String virtualPath;
        virtualPath = path;

        for (auto&& mount : m_mounts) {
            if (mount.pBackend->IsApplicable(virtualPath)) {
                mount.pBackend->ResolveVirtualPath(virtualPath, path);
                if (SR_PLATFORM_NS::GetPathType(path) == FSItemType::Undefined) {
                    continue;
                }
                return;
            }
        }

        SR_ERROR("VFS::ResolvePath() : no applicable backend found for virtual path \"{}\"!", path);
        path.clear();
    }

    void VFS::ResolvePath(Path& path) const {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        static String resolvedPath;
        resolvedPath = path.ToStringView();
        ResolvePath(resolvedPath);
        path = resolvedPath;
    }

    bool VFS::Copy(StringView from, StringView to) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        auto&& copyFile = [&](StringView source, StringView destination) {
            auto&& sourceFile = OpenFile(source, FileMode::Read);
            auto&& destinationFile = OpenFile(destination, FileMode::Write);
            if (!sourceFile || !destinationFile) {
                SR_ERROR("VFS::Copy() : failed to open source or destination file!\n\tSource: {}\n\tDestination: {}", source, destination);
                return false;
            }
            char buffer[8_KiB];
            while (true) {
                auto bytesRead = sourceFile.Read(buffer, sizeof(buffer));
                if (bytesRead == 0) {
                    break;
                }
                if (destinationFile.Write(buffer, bytesRead) != bytesRead) {
                    SR_ERROR("VFS::Copy() : failed to write to destination file!\n\tDestination: {}", destination);
                    return false;
                }
            }
            SR_PLATFORM_NS::CopyPermissions(source, destination);
            return true;
        };

        if (IsFileExists(from)) {
            return copyFile(from, to);
        }
        else if (!IsFolderExists(from)) {
            bool haveErrors = false;
            Enumerate(from, [&](const VFSEntry& entry) {
                if (!haveErrors && entry.type == FSItemType::File) {
                    static String destinationPath;
                    destinationPath = to;
                    destinationPath += entry.relativePath.substr(from.size());
                    if (!copyFile(entry.fullPath, destinationPath)) {
                        haveErrors = true;
                    }
                }
            }, true);
            return !haveErrors;
        }
        SR_ERROR("VFS::Copy() : source path does not exist!\n\tSource: {}\n\tDestination: {}", from, to);
        return false;
    }

    bool VFS::CreateDirectories(StringView path) const {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        for (auto&& mount : m_mounts) {
            if (!mount.pBackend->WriteSupports()) {
                continue;
            }
            if (mount.pBackend->IsApplicable(path)) {
                mount.pBackend->ResolveVirtualPath(path, gResolvedPathBuffer);
                if (SR_PLATFORM_NS::CreateDirectories(gResolvedPathBuffer)) {
                    return true;
                }
                else {
                    SR_ERROR("VFS::CreateDirectories() : failed to create directories for path \"{}\"!", path);
                    return false;
                }
            }
        }

        SR_ERROR("VFS::CreateDirectories() : no applicable backend found for virtual path \"{}\"!", path);
        return false;
    }

    void VFS::UnResolvePath(String& path) const {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        for (auto&& mount : m_mounts) {
            if (mount.pBackend->IsApplicable(path)) {
                auto&& realPath = mount.pBackend->GetRealPath();
                if (path.starts_with(realPath)) {
                    path.remove_prefix(realPath.size());
                    if (!path.empty() && path.front() == '/') {
                        path.remove_prefix(1);
                    }
                    if (!mount.pBackend->GetVirtualPath().empty()) {
                        path.insert(0, mount.pBackend->GetVirtualPath());
                        path.insert(mount.pBackend->GetVirtualPath().size(), "/");
                    }
                }
                return;
            }
        }

        SR_ERROR("VFS::UnResolvePath() : no applicable backend found for path \"{}\"!", path);
        path.clear();
    }

    void VFS::UnResolvePath(Path& path) const {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        static String unresolvedPath;
        unresolvedPath = path.ToStringView();
        UnResolvePath(unresolvedPath);
        path = unresolvedPath;
    }

    FSItemType AndroidVFSBackend::GetType(StringView path) const {
        return FSItemType::Folder;
    }

    File AndroidVFSBackend::OpenFile(StringView path, FileMode mode) const {
        return File();
    }

    void AndroidVFSBackend::Enumerate(StringView directory, const IVFSBackend::EnumerateCallback& callback, bool recursive) const {

    }

    void AndroidVFSBackend::Delete(StringView path) const {
        SRHalt("AndroidVFSBackend::Delete() : not supported!");
    }

    void AndroidVFSBackend::ResolveVirtualPath(StringView virtualPath, String &outRealPath) const {
        outRealPath = virtualPath;
    }
}