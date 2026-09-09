//
// Created by Monika on 08.09.2026.
//

#ifndef SR_ENGINE_COMMON_FILE_H
#define SR_ENGINE_COMMON_FILE_H

#include <Utils/Common/PassKey.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/FileSystem/MappedFile.h>

namespace SR_UTILS_NS {
    class VFS;

    SR_ENUM_NS_STRUCT_T(FileMode, uint8_t,
        None = 0,
        Read = 1 << 0,
        Write = 1 << 1,
        Map = 1 << 2,
        ReadWrite = Read | Write,
        ReadMap = Read | Map,
        WriteMap = Write | Map,
        ReadWriteMap = Read | Write | Map
    );

    enum class SeekOrigin : uint8_t {
        Begin,
        Current,
        End
    };

    class FileImpl : public NonCopyable {
    public:
        virtual bool Open(StringView path, FileMode mode) = 0;
        virtual void Close() = 0;

        SR_NODISCARD FileMode GetMode() const { return m_mode; }
        SR_NODISCARD virtual uint64_t GetSize() const = 0;
        SR_NODISCARD virtual uint64_t GetPosition() const = 0;

        virtual uint64_t Read(void* data, uint64_t size) = 0;
        virtual uint64_t Write(const void* data, uint64_t size) = 0;

        virtual bool Seek(int64_t offset, SeekOrigin origin) = 0;
        virtual void Flush() { }

        SR_NODISCARD virtual StringView Data() = 0;

        void AddUse();
        void RemoveUse();
        SR_NODISCARD uint32_t GetUseCount() const;

    protected:
        FileMode m_mode = FileMode::None;

    private:
        std::atomic<uint32_t> m_useCount = 0;

    };

    class StreamFileImpl : public FileImpl {
    public:
        bool Open(StringView path, FileMode mode) override;
        void Close() override;

        SR_NODISCARD uint64_t GetSize() const override;
        SR_NODISCARD uint64_t GetPosition() const override;

        uint64_t Read(void* data, uint64_t size) override;
        uint64_t Write(const void* data, uint64_t size) override;

        bool Seek(int64_t offset, SeekOrigin origin) override;
        void Flush() override;

        SR_NODISCARD StringView Data() override;

    private:
        mutable std::fstream m_stream;

    };

    class MappedFileImpl : public FileImpl {
    public:
        bool Open(StringView path, FileMode mode) override;
        void Close() override;

        SR_NODISCARD uint64_t GetSize() const override;
        SR_NODISCARD uint64_t GetPosition() const override;

        uint64_t Read(void* data, uint64_t size) override;
        uint64_t Write(const void* data, uint64_t size) override;

        bool Seek(int64_t offset, SeekOrigin origin) override;

        SR_NODISCARD StringView Data() override;

    private:
        MappedFile m_mappedFile;
        uint64_t m_position = 0;

    };

    class File : public SRClass {
        SR_CLASS()
    public:
        File(FileImpl* pImpl, StringView path, FileMode mode);
        File() = default;
        File(const File& other);
        File(File&& other) noexcept;
        File& operator=(const File& other);
        File& operator=(File&& other) noexcept;
        ~File() override;

    public:
        SR_NODISCARD bool IsOpen() const;
        SR_NODISCARD FileMode GetMode() const;
        SR_NODISCARD uint64_t GetSize() const;
        SR_NODISCARD uint64_t GetPosition() const;

        File& operator<<(StringView str);

        SR_NODISCARD operator bool() const { return IsOpen(); }

        void Read(String& buffer);
        uint64_t Read(void* data, uint64_t size);
        uint64_t Write(const void* data, uint64_t size);

        void Flush();
        bool Seek(int64_t offset, SeekOrigin origin);

        SR_NODISCARD StringView Data();
        SR_NODISCARD const StringView Data() const;

    private:
        void AddUse();
        void RemoveUse();

    private:
        FileImpl* m_pImpl = nullptr;

    };
}

#endif //SR_ENGINE_COMMON_FILE_H
