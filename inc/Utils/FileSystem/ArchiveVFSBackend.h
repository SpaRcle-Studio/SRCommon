//
// Created by Monika on 14.09.2026.
//

#ifndef SR_ENGINE_COMMON_ARCHIVE_VFS_BACKEND_H
#define SR_ENGINE_COMMON_ARCHIVE_VFS_BACKEND_H

#include <Utils/FileSystem/IVFSBackend.h>
#include <Utils/Types/Vector.h>

namespace SR_UTILS_NS {
    enum class ArchiveVFSBackendMode {
        File, View, Memory
    };

    class SR_COMMON_DLL_API ArchiveVFSBackend : public IVFSBackend {
    public:
        ArchiveVFSBackend(StringView archive, ArchiveVFSBackendMode mode, uint64_t decompressedSize = 0);
        SR_NODISCARD bool IsValid() const { return m_isUnpacked; }

    public:
        SR_NODISCARD bool ReadSupports() const override { return true; }
        SR_NODISCARD bool WriteSupports() const override { return false; }
        SR_NODISCARD FSItemType GetType(StringView path) const override;
        SR_NODISCARD File OpenFile(StringView path, FileMode mode) const override;

        void Delete(StringView path) const override;
        void Enumerate(StringView directory, const EnumerateCallback& callback, bool recursive) const override;
        void ResolveVirtualPath(StringView virtualPath, String& outRealPath) const override;

    private:
        bool UnpackData();
        bool IndexData();

    private:
        uint64_t m_decompressedSize = 0;
        bool m_isUnpacked = false;
        ArchiveVFSBackendMode m_mode = ArchiveVFSBackendMode::File;
        String m_archivePath;
        String m_archiveDataMemory;
        StringView m_archiveData;
        struct Entry {
            String name;
            StringView data;
        };
        Vector<Entry> m_entries;

    };
}

#endif //SR_ENGINE_COMMON_ARCHIVE_VFS_BACKEND_H
