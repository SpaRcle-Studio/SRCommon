//
// Created by Monika on 02.03.2026.
//

#include <Utils/FileSystem/VFS.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/FileSystem/IVFSBackend.h>
#include <Utils/Types/LockGuard.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Memory/MemoryLiterals.h>
#include <Utils/Memory/Allocator.h>

#include <Codegen/VFS.generated.hpp>

namespace SR_UTILS_NS {
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
                static String gVFSResolvedPathBuffer;
                mount.pBackend->ResolveVirtualPath(path, gVFSResolvedPathBuffer);
                if (SR_PLATFORM_NS::CreateDirectories(gVFSResolvedPathBuffer)) {
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
}