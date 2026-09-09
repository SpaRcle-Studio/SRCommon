//
// Created by Monika on 02.03.2026.
//

#ifndef SR_ENGINE_COMMON_VFS_H
#define SR_ENGINE_COMMON_VFS_H

#include <Utils/FileSystem/File.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Types/FlatHashMap.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/SortedVector.h>

namespace SR_UTILS_NS {
    struct VFSEntry {
        StringView name;
        StringView extension;
        StringView fullPath;
        StringView relativePath;
        FSItemType type = FSItemType::Undefined;
    };

    struct VFSMount;

    class IVFSBackend : public NonCopyable {
    public:
        IVFSBackend() = default;
        IVFSBackend(StringView realPath)
            : m_realPath(realPath)
        { }

    public:
        using EnumerateCallback = SR_HTYPES_NS::Function<void(const VFSEntry&)>;

        SR_NODISCARD virtual bool ReadSupports() const = 0;
        SR_NODISCARD virtual bool WriteSupports() const = 0;
        SR_NODISCARD virtual FSItemType GetType(StringView path) const = 0;
        SR_NODISCARD virtual File OpenFile(StringView path, FileMode mode) const = 0;

        virtual void Enumerate(StringView directory, const EnumerateCallback& callback, bool recursive) const = 0;
        virtual void ResolveVirtualPath(StringView virtualPath, String& outRealPath) const = 0;
        virtual void Delete(StringView path) const = 0;

        SR_NODISCARD bool IsApplicable(StringView path) const;

        void SetVirtualPath(PassKey<VFS>, StringView virtualPath) { m_virtualPath = virtualPath; }
        SR_NODISCARD StringView GetVirtualPath() const { return m_virtualPath; }

        SR_NODISCARD StringView GetRealPath() const { return m_realPath; }

        void AddIgnoredExtension(StringView extension) { m_ignoredExtensions.insert(extension); }

    protected:
        String m_virtualPath;
        String m_realPath;
        Set<String> m_ignoredExtensions;

    };

    class DirectoryVFSBackend : public IVFSBackend {
    public:
        explicit DirectoryVFSBackend(StringView realPath)
            : IVFSBackend(realPath)
        { }

    public:
        SR_NODISCARD bool ReadSupports() const override { return true; }
        SR_NODISCARD bool WriteSupports() const override { return true; }
        SR_NODISCARD FSItemType GetType(StringView path) const override;
        SR_NODISCARD File OpenFile(StringView path, FileMode mode) const override;

        void Enumerate(StringView directory, const EnumerateCallback& callback, bool recursive) const override;
        void Delete(StringView path) const override;
        void ResolveVirtualPath(StringView virtualPath, String& outRealPath) const override;

    private:

    };

    class ReadOnlyDirectoryVFSBackend : public DirectoryVFSBackend {
    public:
        explicit ReadOnlyDirectoryVFSBackend(StringView realPath)
            : DirectoryVFSBackend(realPath)
        { }

        SR_NODISCARD bool WriteSupports() const override { return false; }
    };

    struct VFSMount {
        IVFSBackend* pBackend = nullptr;
        int32_t priority = 0;
        bool permanent = false;
    };

    class AndroidVFSBackend : public IVFSBackend {
    public:
        SR_NODISCARD bool ReadSupports() const override { return true; }
        SR_NODISCARD bool WriteSupports() const override { return false; }
        SR_NODISCARD FSItemType GetType(StringView path) const override;
        SR_NODISCARD File OpenFile(StringView path, FileMode mode) const override;

        void Delete(StringView path) const override;
        void Enumerate(StringView directory, const EnumerateCallback& callback, bool recursive) const override;
        void ResolveVirtualPath(StringView virtualPath, String& outRealPath) const override;

    };

    /// @noCopyable @noMovable
    class VFS : public Singleton<VFS>, public SRClass {
        SR_REGISTER_SINGLETON(VFS);
        SR_CLASS()
    public:
        SR_NODISCARD bool IsExists(StringView path) const;
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

        void ResolvePath(String& path) const;
        void ResolvePath(Path& path) const;
        void UnResolvePath(String& path) const;
        void UnResolvePath(Path& path) const;

        bool CreateDirectories(StringView path) const;

        void DeInitialize();
        bool IsSingletonCanBeDestroyed() const override;

        void Enumerate(StringView directory, const SR_HTYPES_NS::Function<void(const VFSEntry&)>& callback, bool recursive) const;

        void Mount(StringView virtualPath, IVFSBackend* pBackend, int32_t priority, bool permanent = false);
        void Unmount(IVFSBackend* pBackend);

        void UnmountAll();

    private:
        mutable RawPointerHolder<IAllocator> m_enumerateAllocator;
        Vector<VFSMount> m_mounts;
        SR_HTYPES_NS::SortedVector<FileImpl*> m_handles;

    };
}

#endif //SR_ENGINE_COMMON_VIRTUAL_FS_H
