//
// Created by Monika on 14.09.2026.
//

#include <Utils/FileSystem/ArchiveVFSBackend.h>
#include <Utils/FileSystem/File.h>
#include <Utils/Types/Set.h>
#include <zlib/zlib.h>

namespace SR_UTILS_NS {
    static constexpr StringView ResourceMagic = "SR_ENGINE_VFS_RESOURCES_V1";

    static bool ReadArchiveValue(StringView data, SizeType& position, void* value, SizeType size) {
        if (position > data.size() || size > data.size() - position) {
            return false;
        }
        memcpy(value, data.data() + position, size);
        position += size;
        return true;
    }

    ArchiveVFSBackend::ArchiveVFSBackend(StringView archive, ArchiveVFSBackendMode mode, uint64_t decompressedSize)
        : m_decompressedSize(decompressedSize), m_mode(mode)
    {
        if (mode == ArchiveVFSBackendMode::File) {
            m_archivePath = archive;
            File file(new StreamFileImpl(), archive, FileMode::Read);
            const uint64_t footerSize = ResourceMagic.size() + sizeof(uint64_t) * 2;
            if (!file || file.GetSize() < footerSize || !file.Seek(-static_cast<int64_t>(footerSize), SeekOrigin::End)) {
                return;
            }
            String footer;
            footer.resize(footerSize);
            if (file.Read(footer.data(), footerSize) != footerSize ||
                StringView(footer.data(), ResourceMagic.size()) != ResourceMagic) {
                return;
            }
            SizeType position = ResourceMagic.size();
            uint64_t compressedSize = 0;
            if (!ReadArchiveValue(footer, position, &m_decompressedSize, sizeof(uint64_t)) ||
                !ReadArchiveValue(footer, position, &compressedSize, sizeof(uint64_t)) ||
                compressedSize > file.GetSize() - footerSize || compressedSize > SR_UINT32_MAX ||
                m_decompressedSize == 0 || m_decompressedSize > SR_UINT32_MAX) {
                return;
            }
            if (!file.Seek(file.GetSize() - footerSize - compressedSize, SeekOrigin::Begin)) {
                return;
            }
            m_archiveDataMemory.resize(compressedSize);
            if (file.Read(m_archiveDataMemory.data(), compressedSize) != compressedSize) {
                return;
            }
            m_archiveData = m_archiveDataMemory;
        }
        else if (mode == ArchiveVFSBackendMode::Memory) {
            m_archiveDataMemory = archive;
            m_archiveData = m_archiveDataMemory;
        }
        else {
            m_archiveData = archive;
        }
        m_isUnpacked = UnpackData() && IndexData();
        if (!m_isUnpacked) {
            m_entries.clear();
        }
    }

    bool ArchiveVFSBackend::UnpackData() {
        if (m_decompressedSize == 0 || m_decompressedSize > SR_UINT32_MAX || m_archiveData.size() > SR_UINT32_MAX) {
            return false;
        }
        String data;
        data.resize(m_decompressedSize);
        uLongf size = m_decompressedSize;
        if (uncompress(reinterpret_cast<Bytef*>(data.data()), &size,
            reinterpret_cast<const Bytef*>(m_archiveData.data()), m_archiveData.size()) != Z_OK || size != m_decompressedSize) {
            return false;
        }
        m_archiveDataMemory = std::move(data);
        m_archiveData = m_archiveDataMemory;
        return true;
    }

    bool ArchiveVFSBackend::IndexData() {
        SizeType position = 0;
        uint16_t count = 0;
        if (!ReadArchiveValue(m_archiveData, position, &count, sizeof(count))) {
            return false;
        }
        for (uint32_t i = 0; i < count; ++i) {
            uint16_t nameSize = 0;
            uint64_t contentSize = 0;
            if (!ReadArchiveValue(m_archiveData, position, &nameSize, sizeof(nameSize)) ||
                nameSize == 0 || nameSize > m_archiveData.size() - position) {
                return false;
            }
            StringView name(m_archiveData.data() + position, nameSize);
            position += nameSize;
            if (name.starts_with('/') || name.contains("\\") || name.contains(":") || name == ".." ||
                name.starts_with("../") || name.contains("/../") || name.ends_with("/..") ||
                !ReadArchiveValue(m_archiveData, position, &contentSize, sizeof(contentSize)) ||
                contentSize > m_archiveData.size() - position) {
                return false;
            }
            m_entries.emplace_back(Entry{ String(name), StringView(m_archiveData.data() + position, contentSize) });
            position += contentSize;
        }
        return position == m_archiveData.size();
    }

    File ArchiveVFSBackend::OpenFile(StringView path, FileMode mode) const {
        if (SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Write)) {
            return File();
        }
        for (const auto& entry : m_entries) {
            if (entry.name == path) {
                return File(new MemoryFileImpl(String(entry.data)), path, mode);
            }
        }
        return File();
    }

    FSItemType ArchiveVFSBackend::GetType(StringView path) const {
        if (!m_isUnpacked) {
            return FSItemType::Undefined;
        }
        for (const auto& entry : m_entries) {
            if (entry.name == path) {
                return FSItemType::File;
            }
            if (path.empty() || (entry.name.starts_with(path) &&
                (path.ends_with("/") || entry.name[path.size()] == '/'))) {
                return FSItemType::Folder;
            }
        }
        return FSItemType::Undefined;
    }

    void ArchiveVFSBackend::Delete(StringView path) const {
        SRHalt("ArchiveVFSBackend::Delete() : not supported!");
    }

    void ArchiveVFSBackend::Enumerate(StringView directory, const EnumerateCallback& callback, bool recursive) const {
        String prefix(directory);
        if (!prefix.empty() && !prefix.ends_with('/')) {
            prefix += '/';
        }
        Set<String> visited;
        for (const auto& entry : m_entries) {
            if (!entry.name.starts_with(prefix)) {
                continue;
            }
            SizeType position = prefix.size();
            while (position < entry.name.size()) {
                const auto slash = entry.name.find('/', position);
                const bool folder = slash != String::npos;
                const String path = folder ? entry.name.substr(0, slash) : entry.name;
                if (!visited.contains(path)) {
                    visited.insert(path);
                    StringView name(path);
                    const auto lastSlash = name.find_last_of('/');
                    if (lastSlash != StringView::npos) {
                        name.remove_prefix(lastSlash + 1);
                    }
                    StringView extension;
                    const auto dot = name.find_last_of('.');
                    if (!folder && dot != StringView::npos) {
                        extension = name.substr(dot + 1);
                    }
                    VFSEntry item;
                    item.name = name;
                    item.extension = extension;
                    item.fullPath = path;
                    item.relativePath = path;
                    item.type = folder ? FSItemType::Folder : FSItemType::File;
                    callback(item);
                }
                if (!recursive || !folder) {
                    break;
                }
                position = slash + 1;
            }
        }
    }

    void ArchiveVFSBackend::ResolveVirtualPath(StringView virtualPath, String& outRealPath) const {
        outRealPath = virtualPath;
    }
}
