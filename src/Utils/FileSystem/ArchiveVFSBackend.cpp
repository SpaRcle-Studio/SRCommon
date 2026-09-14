//
// Created by Monika on 14.09.2026.
//

#include <Utils/FileSystem/ArchiveVFSBackend.h>
#include <Utils/FileSystem/File.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    ArchiveVFSBackend::ArchiveVFSBackend(StringView archive, ArchiveVFSBackendMode mode, uint64_t decompressedSize)
        : IVFSBackend()
    {
        switch (mode) {
            case ArchiveVFSBackendMode::File:
                m_archivePath = archive;
                break;
            case ArchiveVFSBackendMode::View:
                m_archiveData = archive;
                m_decompressedSize = decompressedSize;
                break;
            case ArchiveVFSBackendMode::Memory:
                m_archiveDataMemory = archive;
                m_archiveData = m_archiveDataMemory;
                m_decompressedSize = decompressedSize;
                break;
        }
    }

    File ArchiveVFSBackend::OpenFile(StringView path, FileMode mode) const {
        return File();
    }

    FSItemType ArchiveVFSBackend::GetType(StringView path) const {
        return FSItemType::Undefined;
    }

    void ArchiveVFSBackend::Delete(StringView path) const {
        SRHalt("ArchiveVFSBackend::Delete() : not supported!");
    }

    void ArchiveVFSBackend::Enumerate(StringView directory, const IVFSBackend::EnumerateCallback& callback, bool recursive) const {

    }

    void ArchiveVFSBackend::ResolveVirtualPath(StringView virtualPath, String& outRealPath) const {

    }

    bool ArchiveVFSBackend::UnpackData() {
        if (m_isUnpacked) {
            return true;
        }

        SR_TRACY_ZONE;

    #ifdef SR_COMMON_ZLIB
        z_stream strm;
        strm.zalloc = SRZLibAlloc;
        strm.zfree = SRZLibFree;
        strm.opaque = Z_NULL;
        int32_t ret = inflateInit(&strm);
        if (ret != Z_OK) {
            SR_ERROR("ArchiveVFSBackend::UnpackData(): inflateInit failed, error code: {}\n"_format(ret));
            return false;
        }

        String decompressedData;
        decompressedData.resize(m_decompressedSize);

        strm.avail_in = m_archiveData.size();
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(m_archiveData.data()));
        strm.avail_out = m_decompressedSize;
        strm.next_out = reinterpret_cast<Bytef*>(decompressedData.data());

        ret = inflate(&strm, Z_FINISH);
        if (ret != Z_STREAM_END) {
            inflateEnd(&strm);
            SR_ERROR("ArchiveVFSBackend::UnpackData(): failed to decompress data, error code: {}\n"_format(ret));
            return false;
        }

        m_decompressedSize = strm.total_out;
        inflateEnd(&strm);

        m_archiveDataMemory = std::move(decompressedData);
        m_archiveData = m_archiveDataMemory;
        m_isUnpacked = true;
        return true;
    #else
        SRHalt("ArchiveVFSBackend::UnpackData(): zlib support is not enabled!");
        return false;
    #endif
    }
}
