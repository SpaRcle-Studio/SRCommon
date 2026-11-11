//
// Created by Monika on 02.03.2026.
//

#ifndef SR_ENGINE_COMMON_VFS_H
#define SR_ENGINE_COMMON_VFS_H

#include <Utils/FileSystem/File.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/SortedVector.h>

namespace SR_UTILS_NS {
    class IVFSBackend;
    struct VFSEntry;

    struct SR_COMMON_DLL_API VFSMount {
        IVFSBackend* pBackend = nullptr;
        int32_t priority = 0;
        bool permanent = false;
    };

    /// @noCopyable @noMovable
    class SR_COMMON_DLL_API VFS : public Singleton<VFS>, public SRClass {
        SR_REGISTER_SINGLETON(VFS);
        SR_CLASS()
    public:
        SR_NODISCARD bool IsExists(StringView path) const;
        SR_NODISCARD bool IsWritable(StringView path) const;
        SR_NODISCARD bool IsFileExists(StringView path) const;
        SR_NODISCARD bool IsFolderExists(StringView path) const;
        SR_NODISCARD File OpenFile(StringView path, FileMode mode);
        SR_NODISCARD File OpenFile(const String& path, FileMode mode);
        SR_NODISCARD File OpenFile(const Path& path, FileMode mode);
        SR_NODISCARD bool HaveMount(StringView virtualPath) const;

        void CloseFile(PassKey<FileImpl>, FileImpl* pImpl);
        void RegisterHandle(PassKey<File>, FileImpl* pImpl);
        void Delete(StringView path);
        bool Copy(StringView from, StringView to);

        bool ResolvePath(String& path, FileMode mode) const;
        bool ResolvePath(Path& path, FileMode mode) const;

        void UnResolvePath(String& path) const;
        void UnResolvePath(Path& path) const;

        bool CreateDirectories(StringView path) const;

        void DeInitialize();
        bool IsSingletonCanBeDestroyed() const override;

        /// @method @evaluate
        void GetFilesInDirectory(Vector<Path>& files, StringView directory, bool recursive);

        void Enumerate(StringView directory, const SR_HTYPES_NS::Function<void(const VFSEntry&)>& callback, bool recursive) const;

        void Mount(StringView virtualPath, IVFSBackend* pBackend, int32_t priority, bool permanent = false);
        void Unmount(IVFSBackend* pBackend);

        void UnmountAll();

    private:
        mutable bool m_isEnumerating = false;
        mutable RawPointerHolder<IAllocator> m_enumerateAllocator;
        Vector<VFSMount> m_mounts;
        SR_UTILS_NS::SortedVector<FileImpl*> m_handles;

    };
}

#endif //SR_ENGINE_COMMON_VIRTUAL_FS_H
