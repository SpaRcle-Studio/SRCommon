//
// Created by Monika on 10.09.2026.
//

#ifndef SR_ENGINE_COMMON_DIRECTORY_VFS_BACKEND_H
#define SR_ENGINE_COMMON_DIRECTORY_VFS_BACKEND_H

#include <Utils/FileSystem/IVFSBackend.h>

namespace SR_UTILS_NS {
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

    };

    class ReadOnlyDirectoryVFSBackend : public DirectoryVFSBackend {
    public:
        explicit ReadOnlyDirectoryVFSBackend(StringView realPath)
            : DirectoryVFSBackend(realPath)
        { }

        SR_NODISCARD bool WriteSupports() const override { return false; }
    };

    class WriteOnlyDirectoryVFSBackend : public DirectoryVFSBackend {
    public:
        explicit WriteOnlyDirectoryVFSBackend(StringView realPath)
            : DirectoryVFSBackend(realPath)
        { }

        SR_NODISCARD bool ReadSupports() const override { return false; }
    };
}

#endif //SR_ENGINE_COMMON_DIRECTORY_VFS_BACKEND_H
