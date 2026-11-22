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
    struct VFSThreadLocal {
        struct EnumerationCache {
            MonotonicAllocator allocator{8_KB};
            EnumerationCache* pNested = nullptr;
            bool active = false;

            ~EnumerationCache() {
                delete pNested;
            }
        };

        EnumerationCache cache;
        String resolvePathVirtualPath;
        String resolvePathResolvedPath;
        String resolvedPathBuffer;
        String unresolvedPath;
    };
    static SR_THREAD_LOCAL VFSThreadLocal gVFSThreadLocal;

    bool VFS::IsExists(StringView path) const {
        SR_TRACY_ZONE;
        std::shared_lock lock(m_mountsMutex);
        return std::ranges::any_of(m_mounts, [&](const VFSMount& mount) {
            return mount.pBackend->IsApplicable(path) && mount.pBackend->GetType(path) != FSItemType::Undefined;
        });
    }

    bool VFS::IsFileExists(StringView path) const {
        SR_TRACY_ZONE;
        std::shared_lock lock(m_mountsMutex);
        return std::ranges::any_of(m_mounts, [&](const VFSMount& mount) {
            return mount.pBackend->IsApplicable(path) && mount.pBackend->GetType(path) == FSItemType::File;
        });
    }

    bool VFS::IsFolderExists(StringView path) const {
        SR_TRACY_ZONE;
        std::shared_lock lock(m_mountsMutex);
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

        const bool isRead = SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Read);
        const bool isWrite = SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Write);

        std::shared_lock readLock(m_mountsMutex, std::defer_lock);
        std::unique_lock writeLock(m_mountsMutex, std::defer_lock);
        if (isWrite && SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Lock)) {
            writeLock.lock();
        }
        else {
            readLock.lock();
        }

        IVFSBackend* pFirstWritable = nullptr;

        for (auto&& mount : m_mounts) {
            if (isRead && !mount.pBackend->ReadSupports()) {
                continue;
            }

            if (isWrite && !mount.pBackend->WriteSupports()) {
                continue;
            }

            if (mount.pBackend->IsApplicable(path)) {
                const auto type = mount.pBackend->GetType(path);
                if (isRead && type != FSItemType::File) {
                    continue;
                }
                if (isWrite && type != FSItemType::File) {
                    pFirstWritable = pFirstWritable ? pFirstWritable : mount.pBackend;
                    continue;
                }
                return mount.pBackend->OpenFile(path, mode);
            }
        }

        if (pFirstWritable) {
            return pFirstWritable->OpenFile(path, mode);
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
        auto&& pCache = &gVFSThreadLocal.cache;
        while (pCache->active) {
            if (!pCache->pNested) {
                pCache->pNested = new VFSThreadLocal::EnumerationCache();
            }
            pCache = pCache->pNested;
        }

        struct EnumerationScope {
            VFSThreadLocal::EnumerationCache& cache;

            ~EnumerationScope() {
                cache.allocator.ResetMemory();
                cache.active = false;
            }
        } scope{*pCache};
        pCache->active = true;

        struct EnumerationData {
            MonotonicAllocator& allocator;
            Vector<VFSEntry> entries{&allocator};
            SortedVector<StringView> paths{&allocator};
        } data{pCache->allocator};
        {
            std::shared_lock lock(m_mountsMutex);
            auto&& callbackWrapper = [&data](const VFSEntry& entry) {
                SR_TRACY_ZONE_N("Callback wrapper");
                if (data.paths.Contains(entry.relativePath)) {
                    return;
                }

                VFSEntry item;
                item.name = String(entry.name, &data.allocator);
                item.extension = String(entry.extension, &data.allocator);
                item.fullPath = String(entry.fullPath, &data.allocator);
                item.relativePath = String(entry.relativePath, &data.allocator);
                item.type = entry.type;
                data.paths.Add(item.relativePath);
                data.entries.emplace_back(item);
            };

            for (auto&& mount : m_mounts) {
                if (mount.pBackend->IsApplicable(directory)) {
                    mount.pBackend->Enumerate(directory, callbackWrapper, recursive);
                }
            }
        }

        for (const VFSEntry& entry : data.entries) {
            SR_TRACY_ZONE_N("Callback");
            callback(entry);
        }
    }

    void VFS::Delete(StringView path) {
        SR_TRACY_ZONE;
        std::unique_lock lock(m_mountsMutex);

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
        std::unique_lock lock(m_mountsMutex);

        String modeLog = "R{}/W{}"_format(pBackend->ReadSupports() ? "+" : "-", pBackend->WriteSupports() ? "+" : "-");

        StringView realPath = pBackend->GetRealPath();
        if (virtualPath.empty()) {
            if (realPath.empty()) {
                SR_LOG("VFS::Mount() : mounting {} root virtual path with priority {}...", modeLog, priority);
            }
            else {
                SR_LOG("VFS::Mount() : mounting {} root ({}) virtual path with priority {}...", modeLog, realPath, priority);
            }
        }
        else {
            if (realPath.empty()) {
                SR_LOG("VFS::Mount() : mounting {} virtual path \"{}\" with priority {}...", modeLog, virtualPath, priority);
            }
            else {
                SR_LOG("VFS::Mount() : mounting {} virtual path \"{}\" ({}) with priority {}...", modeLog, virtualPath, realPath, priority);
            }
        }

        pBackend->SetVirtualPath({}, virtualPath);
        m_mounts.emplace_back(VFSMount{ pBackend, priority, permanent });
        std::stable_sort(m_mounts.begin(), m_mounts.end(), [](const VFSMount& lhs, const VFSMount& rhs) {
            return lhs.priority > rhs.priority;
        });
    }

    void VFS::Unmount(IVFSBackend* pBackend) {
        SR_TRACY_ZONE;
        std::unique_lock lock(m_mountsMutex);
        SR_LOG("VFS::Unmount() : unmounting virtual path \"{}\" (\"{}\")...", pBackend->GetVirtualPath(), pBackend->GetRealPath());
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
        std::unique_lock lock(m_mountsMutex);
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
        std::unique_lock lock(m_mountsMutex);
        SR_LOG("VFS::UnmountAll() : unmounting all mounts...");
        for (auto&& iterator = m_mounts.begin(); iterator != m_mounts.end();) {
            if (iterator->permanent) {
                ++iterator;
                continue;
            }
            SR_LOG("VFS::UnmountAll() : unmounting \"{}\" mount...", iterator->pBackend->GetVirtualPath());
            iterator = m_mounts.erase(iterator);
        }
    }

    bool VFS::HaveMount(StringView virtualPath) const {
        SR_TRACY_ZONE;
        std::shared_lock lock(m_mountsMutex);
        return std::ranges::any_of(m_mounts, [&](const VFSMount& mount) {
            return virtualPath == mount.pBackend->GetVirtualPath();
        });
    }

    bool VFS::ResolvePath(String& path, FileMode mode, bool reportError) const {
        SR_TRACY_ZONE;
        std::shared_lock lock(m_mountsMutex);
        gVFSThreadLocal.resolvePathVirtualPath = path;

        for (auto&& mount : m_mounts) {
            const bool isRead = SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Read);
            const bool isWrite = SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Write);

            if (isRead && !mount.pBackend->ReadSupports()) {
                continue;
            }
            if (isWrite && !mount.pBackend->WriteSupports()) {
                continue;
            }

            if (mount.pBackend->IsApplicable(gVFSThreadLocal.resolvePathVirtualPath)) {
                mount.pBackend->ResolveVirtualPath(gVFSThreadLocal.resolvePathVirtualPath, path);
                if (SR_PLATFORM_NS::GetPathType(path) == FSItemType::Undefined) {
                    continue;
                }
                return true;
            }
        }

        if (reportError) {
            SR_ERROR("VFS::ResolvePath() : no applicable backend found for virtual path \"{}\"!", path);
        }
        path.clear();
        return false;
    }

    bool VFS::ResolvePath(Path& path, FileMode mode, bool reportError) const {
        SR_TRACY_ZONE;
        auto&& resolvedPath = gVFSThreadLocal.resolvePathResolvedPath;
        resolvedPath = path.ToStringView();
        const bool res = ResolvePath(resolvedPath, mode, reportError);
        path = resolvedPath;
        return res;
    }

    bool VFS::WriteFileAtomic(StringView path, StringView data) {
        SR_TRACY_ZONE;
        static std::atomic<uint64_t> sequence = 0;
        const String temporary = "{}.tmp.{}.{}.{}"_format(path, reinterpret_cast<uintptr_t>(this),
            std::chrono::steady_clock::now().time_since_epoch().count(), sequence.fetch_add(1));
        auto file = OpenFile(temporary, FileMode::Write);
        if (!file) {
            return false;
        }
        const bool written = file.Write(data.data(), data.size()) == data.size();
        file.Flush();
        const bool sized = file.GetSize() == data.size();
        file = File();
        if (written && sized && ReplaceFile(temporary, path)) {
            return true;
        }
        Delete(temporary);
        SR_ERROR("VFS::WriteFileAtomic() : failed to publish {}", path);
        return false;
    }

    bool VFS::ReplaceFile(StringView source, StringView destination) {
        SR_TRACY_ZONE;
        std::shared_lock lock(m_mountsMutex);
        for (auto&& mount : m_mounts) {
            if (mount.pBackend->WriteSupports() && mount.pBackend->IsApplicable(source)
                && mount.pBackend->IsApplicable(destination) && mount.pBackend->GetType(source) == FSItemType::File) {
                return mount.pBackend->ReplaceFile(source, destination);
            }
        }
        return false;
    }

    bool VFS::Copy(StringView from, StringView to) {
        SR_TRACY_ZONE;

        SR_LOG("VFS::Copy() : copying from \"{}\" to \"{}\"...", from, to);

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
            const Path physicalSource(sourceFile.GetPhysicalPath());
            const Path physicalDestination(destinationFile.GetPhysicalPath());

            /// файлы нужно закрыть до копирования прав, иначе Windows не даст открыть их повторно
            sourceFile = File();
            destinationFile = File();

            if (!physicalSource.empty() && !physicalDestination.empty()) {
                SR_PLATFORM_NS::CopyPermissions(physicalSource, physicalDestination);
            }

            return true;
        };

        if (IsFileExists(from)) {
            return copyFile(from, to);
        }
        else if (IsFolderExists(from)) {
            Vector<Pair<Path, Path>> files;
            Enumerate(from, [&](const VFSEntry& entry) {
                if (entry.type == FSItemType::File) {
                    String destinationPath(to);
                    destinationPath += entry.relativePath.substr(from.size());
                    files.emplace_back(Path(entry.fullPath), Path(destinationPath));
                }
            }, true);
            for (const auto& file : files) {
                if (!copyFile(file.first.ToStringView(), file.second.ToStringView())) {
                    return false;
                }
            }
            return true;
        }
        SR_ERROR("VFS::Copy() : source path does not exist!\n\tSource: {}\n\tDestination: {}", from, to);
        return false;
    }

    bool VFS::CreateDirectories(StringView path) const {
        SR_TRACY_ZONE;
        std::unique_lock lock(m_mountsMutex);

        for (auto&& mount : m_mounts) {
            if (!mount.pBackend->WriteSupports()) {
                continue;
            }
            if (mount.pBackend->IsApplicable(path)) {
                mount.pBackend->ResolveVirtualPath(path, gVFSThreadLocal.resolvedPathBuffer);
                if (SR_PLATFORM_NS::CreateDirectories(gVFSThreadLocal.resolvedPathBuffer)) {
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
        std::shared_lock lock(m_mountsMutex);

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
        auto&& unresolvedPath = gVFSThreadLocal.unresolvedPath;
        unresolvedPath = path.ToStringView();
        UnResolvePath(unresolvedPath);
        path = unresolvedPath;
    }

    bool VFS::IsWritable(StringView path) const {
        SR_TRACY_ZONE;
        std::shared_lock lock(m_mountsMutex);

        for (auto&& mount : m_mounts) {
            if (!mount.pBackend->WriteSupports()) {
                continue;
            }
            if (mount.pBackend->IsApplicable(path)) {
                if (mount.pBackend->GetType(path) != FSItemType::Undefined) {
                    return true;
                }
            }
        }

        return false;
    }

    void VFS::GetFilesInDirectory(Vector<Path>& files, StringView directory, bool recursive, bool relative) {
        SR_TRACY_ZONE;
        files.clear();
        Enumerate(directory, [&files, relative](const VFSEntry& entry) {
            if (entry.type == FSItemType::File) {
                files.emplace_back(relative ? entry.relativePath : entry.fullPath);
            }
        }, recursive);
    }
}
