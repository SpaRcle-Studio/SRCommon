//
// Created by Monika on 18.01.2023.
//

#ifndef SR_ENGINE_STREAM_H
#define SR_ENGINE_STREAM_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class MappedFile;
}

namespace SR_HTYPES_NS {
    class SR_COMMON_DLL_API Stream {
    public:
        Stream();
        Stream(const MappedFile& mappedFile);  /** NOLINT */
        Stream(std::ifstream& ifs);  /** NOLINT */
        Stream(StringView str);  /** NOLINT */
        Stream(const char* pData, uint64_t size, bool copy = true);

        Stream(const Stream& other) noexcept;
        Stream(Stream&& other) noexcept;

        ~Stream();

    public:
        static char* Allocate(uint64_t size);
        static void Free(char* pData);

    public:
        Stream& SR_FASTCALL operator=(const Stream& other) noexcept;
        Stream& SR_FASTCALL operator=(Stream&& other) noexcept;

        operator bool() const noexcept; /** NOLINT */

    public:
        SR_NODISCARD bool Valid() const noexcept;

        std::pair<char*, uint64_t> SR_FASTCALL Detach() noexcept;

        SR_NODISCARD StringView ToStringView() const noexcept;
        SR_NODISCARD String ToString() const noexcept;
        SR_NODISCARD String ToBase64() const noexcept;

        SR_NODISCARD uint64_t GetPosition() const noexcept;
        SR_NODISCARD uint64_t Size() const noexcept;
        SR_NODISCARD uint64_t GetCapacity() const noexcept;

        SR_NODISCARD const char* View() const noexcept;

        Stream& SR_FASTCALL Write(const void* pSrc, uint64_t count) noexcept;
        Stream& SR_FASTCALL Read(void* pDst, uint64_t count) noexcept;
        void* SR_FASTCALL ReadMap(uint64_t count) noexcept;
        Stream& SR_FASTCALL TryRead(void* pDst, uint64_t count) noexcept;

        Stream& SR_FASTCALL write(const void* pSrc, uint64_t count) noexcept;
        Stream& SR_FASTCALL read(void* pDst, uint64_t count) noexcept;
        void* SR_FASTCALL readMap(uint64_t count) noexcept;

        void SetData(const char* pData, uint64_t size);
        void SetPosition(uint64_t position);

        void SR_FASTCALL Reserve(uint64_t capacity);

        void Skip(uint64_t count);

    private:
        uint64_t m_size = 0;
        uint64_t m_pos = 0;
        uint64_t m_capacity = 0;

        char* m_data = nullptr;
        bool m_isOwner = true;

    };
}

#endif //SR_ENGINE_STREAM_H
