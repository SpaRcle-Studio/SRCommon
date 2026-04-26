//
// Created by Monika on 26.04.2026.
//

#include <Utils/FileSystem/MappedFile.h>

namespace SR_UTILS_NS {
#if defined(SR_WIN32)
    MappedFile MappedFileImpl::Open(const Path& path, bool write) {
        MappedFile mappedFile;

        constexpr DWORD access = GENERIC_READ;
        constexpr DWORD share  = FILE_SHARE_READ;

        std::string_view resolvedPath;
        if (SR_PLATFORM_NS::g_platformHooks.pathResolver) {
            resolvedPath = SR_PLATFORM_NS::g_platformHooks.pathResolver(path.ToStringView());
        }
        else {
            resolvedPath = path.ToStringView();
        }

        mappedFile.m_pHandle = CreateFileA(
            resolvedPath.data(),
            access,
            share,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (mappedFile.m_pHandle == INVALID_HANDLE_VALUE) {
            return MappedFile();
        }

        LARGE_INTEGER size = { };
        if (!GetFileSizeEx(mappedFile.m_pHandle, &size)) {
            return MappedFile();
        }

        mappedFile.m_size = static_cast<uint64_t>(size.QuadPart);
        mappedFile.m_pMapping = CreateFileMappingA(
            mappedFile.m_pHandle,
            nullptr,
            PAGE_READONLY,
            0,
            0,
            nullptr
        );
        if (!mappedFile.m_pMapping) {
            return MappedFile();
        }

        DWORD viewAccess = write ? FILE_MAP_COPY : FILE_MAP_READ;
        mappedFile.m_pData = static_cast<char*>(MapViewOfFile(
            mappedFile.m_pMapping,
            viewAccess,
            0,
            0,
            0
        ));
        return mappedFile.m_pData ? std::move(mappedFile) : MappedFile();
    }

    void MappedFileImpl::Close(MappedFile& mappedFile) {
        if (mappedFile.m_pData) {
            UnmapViewOfFile(mappedFile.m_pData);
        }
        if (mappedFile.m_pMapping) {
            CloseHandle(mappedFile.m_pMapping);
        }
        if (mappedFile.m_pHandle) {
            CloseHandle(mappedFile.m_pHandle);
        }
    }
#elif defined(SR_LINUX) || defined(SR_ANDROID)
     MappedFile MappedFileImpl::Open(const Path& path) {
         MappedFile mappedFile;

         int flags = write ? O_RDWR : O_RDONLY;

         mappedFile.m_fd = open(path.c_str(), flags);
         if (mappedFile.m_fd < 0) {
             return MappedFile();
         }

         struct stat st{};
         if (fstat(mappedFile.m_fd, &st) < 0) {
             return MappedFile();
         }

         mappedFile.m_size = static_cast<size_t>(st.st_size);

         int prot = PROT_READ | (write ? PROT_WRITE : 0);
         int mapFlags = MAP_PRIVATE; // COW, безопаснее чем MAP_SHARED

         mappedFile.m_pData = mmap(
             nullptr,
             mappedFile.m_size,
             prot,
             mapFlags,
             mappedFile.m_fd,
             0
         );

         if (mappedFile.m_pData == MAP_FAILED) {
             mappedFile.m_pData = nullptr;
             return MappedFile();
         }

         return mappedFile;
     }

    void MappedFileImpl::Close(MappedFile& mappedFile) {
        if (mappedFile.m_pData) {
            munmap(mappedFile.m_pData, mappedFile.m_size);
        }

        if (mappedFile.m_fd >= 0) {
            close(mappedFile.m_fd);
        }
    }
#else
    /// buffered fallback for unsupported platforms
    MappedFile MappedFileImpl::Open(const Path& path) {
        MappedFile mappedFile;
        mappedFile.m_pHandle = new std::string();
        if (!FileSystem::ReadFile(path, *static_cast<std::string*>(mappedFile.m_pHandle))) {
            return MappedFile();
        }

        mappedFile.m_pData = static_cast<char*>(static_cast<std::string*>(mappedFile.m_pHandle)->data());
        mappedFile.m_size = static_cast<uint64_t>(static_cast<std::string*>(mappedFile.m_pHandle)->size());
        return mappedFile;
    }

    void MappedFileImpl::Close(MappedFile& mappedFile) {
        if (mappedFile.m_pHandle) {
            delete static_cast<std::string*>(mappedFile.m_pHandle);
        }
    }
#endif

    MappedFile::MappedFile(MappedFile&& other) noexcept
        : m_pData(SR_UTILS_NS::Exchange(other.m_pData, nullptr))
        , m_size(SR_UTILS_NS::Exchange(other.m_size, 0))
        , m_pHandle(SR_UTILS_NS::Exchange(other.m_pHandle, nullptr))
        , m_pMapping(SR_UTILS_NS::Exchange(other.m_pMapping, nullptr))
        , m_fd(SR_UTILS_NS::Exchange(other.m_fd, -1))
    { }

    MappedFile& MappedFile::operator=(MappedFile&& other) noexcept {
        if (this != &other) {
            Close();
            m_pData = SR_UTILS_NS::Exchange(other.m_pData, nullptr);
            m_size = SR_UTILS_NS::Exchange(other.m_size, 0);
            m_pHandle = SR_UTILS_NS::Exchange(other.m_pHandle, nullptr);
            m_pMapping = SR_UTILS_NS::Exchange(other.m_pMapping, nullptr);
            m_fd = SR_UTILS_NS::Exchange(other.m_fd, -1);
        }
        return *this;
    }

    MappedFile::~MappedFile() {
        Close();
    }

    MappedFile MappedFile::Open(const Path& path, bool write) {
        SR_TRACY_ZONE;
        if (!path.IsFile()) {
            SR_ERROR("MappedFile::Open() : path is not a file! Path: {}", path);
            return MappedFile();
        }
        return MappedFileImpl::Open(path, write);
    }

    void MappedFile::Close() {
        SR_TRACY_ZONE;
        MappedFileImpl::Close(*this);
        m_pData = nullptr;
        m_size = 0;
        m_pHandle = nullptr;
        m_pMapping = nullptr;
        m_fd = -1;
    }

    const char* MappedFile::GetData() const noexcept {
        return m_pData;
    }

    uint64_t MappedFile::GetSize() const noexcept {
        return m_size;
    }

    MappedFile::operator bool() const noexcept {
        return m_pData != nullptr && m_size > 0;
    }

    std::string_view MappedFile::GetDataView() const noexcept {
        if (!m_pData || m_size == 0) {
            return std::string_view();
        }
        return std::string_view(m_pData, m_size);
    }
}
