//
// Created by Monika on 26.04.2026.
//

#ifndef SR_ENGINE_COMMON_MAPPED_FILE_H
#define SR_ENGINE_COMMON_MAPPED_FILE_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS {
    class MappedFile;

    struct SR_COMMON_DLL_API MappedFileImpl {
        static MappedFile Open(const Path& path, bool write);
        static void Close(MappedFile &mappedFile);
    };

    class SR_COMMON_DLL_API MappedFile final : public NonCopyable {
        friend struct MappedFileImpl;
    private:
        MappedFile() = default;

    public:
        MappedFile(MappedFile&& other) noexcept;
        MappedFile& operator=(MappedFile&& other) noexcept;
        ~MappedFile() override;

        SR_NODISCARD operator bool() const noexcept;

    public:
        SR_NODISCARD static MappedFile Open(const Path& path, bool write = false);
        SR_NODISCARD const char* GetData() const noexcept;
        SR_NODISCARD uint64_t GetSize() const noexcept;
        SR_NODISCARD std::string_view GetDataView() const noexcept;

        void Close();

    private:
        char* m_pData = nullptr;
        uint64_t m_size = 0;

        /// unix
        int m_fd = -1;

        /// windows
        void* m_pHandle = nullptr;
        void* m_pMapping = nullptr;

    };
}

#endif //SR_ENGINE_COMMON_MAPPED_FILE_H
