//
// Created by Monika on 08.09.2026.
//

#include <Utils/FileSystem/File.h>
#include <Utils/FileSystem/VFS.h>

#include <Codegen/File.generated.hpp>

namespace SR_UTILS_NS {
    File::File(FileImpl* pImpl, StringView path, FileMode mode) {
        SR_TRACY_ZONE;
        m_pImpl = pImpl;
        m_pImpl->AddUse();
        VFS::Instance().RegisterHandle({}, m_pImpl);
        if (!m_pImpl->Open(path, mode)) {
            m_pImpl->RemoveUse();
            m_pImpl = nullptr;
        }
    }

    File::File(const File& other) {
        RemoveUse();
        m_pImpl = other.m_pImpl;
        AddUse();
    }

    File::File(File&& other) noexcept {
        RemoveUse();
        m_pImpl = other.m_pImpl;
        other.m_pImpl = nullptr;
    }

    File& File::operator=(const File& other) {
        if (this != &other) {
            RemoveUse();
            m_pImpl = other.m_pImpl;
            AddUse();
        }
        return *this;
    }

    File& File::operator=(File&& other) noexcept {
        if (this != &other) {
            RemoveUse();
            m_pImpl = other.m_pImpl;
            other.m_pImpl = nullptr;
        }
        return *this;
    }

    File::~File() {
        RemoveUse();
    }

    FileMode File::GetMode() const {
        return m_pImpl ? m_pImpl->GetMode() : FileMode::None;
    }

    uint64_t File::GetSize() const {
        SR_TRACY_ZONE;
        return m_pImpl ? m_pImpl->GetSize() : 0;
    }

    uint64_t File::GetPosition() const {
        SR_TRACY_ZONE;
        return m_pImpl ? m_pImpl->GetPosition() : 0;
    }

    uint64_t File::Read(void* data, uint64_t size) {
        SR_TRACY_ZONE;
        return m_pImpl ? m_pImpl->Read(data, size) : 0;
    }

    uint64_t File::Write(const void* data, uint64_t size) {
        SR_TRACY_ZONE;
        return m_pImpl ? m_pImpl->Write(data, size) : 0;
    }

    bool File::Seek(int64_t offset, SeekOrigin origin) {
        SR_TRACY_ZONE;
        return m_pImpl ? m_pImpl->Seek(offset, origin) : false;
    }

    StringView File::Data() {
        SR_TRACY_ZONE;
        return m_pImpl ? m_pImpl->Data() : nullptr;
    }

    const StringView File::Data() const {
        SR_TRACY_ZONE;
        return m_pImpl ? m_pImpl->Data() : nullptr;
    }

    void File::Read(String& buffer) {
        SR_TRACY_ZONE;
        if (!m_pImpl || !SR_MATH_NS::IsMaskIncludedSubMask(m_pImpl->GetMode(), FileMode::Read)) {
            SRHalt("File::Read() : file is not open for reading!");
            return;
        }

        uint64_t size = m_pImpl->GetSize();
        buffer.resize(size);
        uint64_t bytesRead = m_pImpl->Read(buffer.data(), size);
        buffer.resize(bytesRead);
    }

    bool File::IsOpen() const {
        return m_pImpl != nullptr;
    }

    void File::AddUse() {
        if (m_pImpl) {
            m_pImpl->AddUse();
        }
    }

    void File::RemoveUse() {
        if (m_pImpl) {
            m_pImpl->RemoveUse();
        }
    }

    File& File::operator<<(StringView str) {
        Write(str.data(), str.size());
        return *this;
    }

    void File::Flush() {
        if (m_pImpl) {
            m_pImpl->Flush();
        }
    }

    bool StreamFileImpl::Open(StringView path, FileMode mode) {
        m_mode = mode;
        std::ios::openmode openMode = std::ios::binary;
        if (SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Read)) {
            openMode |= std::ios::in;
        }
        if (SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Write)) {
            openMode |= std::ios::out;
        }
        m_stream.open(path.data(), openMode);
        return m_stream.is_open();
    }

    void StreamFileImpl::Close() {
        if (m_stream.is_open()) {
            m_stream.close();
        }
    }

    uint64_t StreamFileImpl::GetSize() const {
        if (!m_stream.is_open()) {
            return 0;
        }
        auto currentPos = m_stream.tellg();
        m_stream.seekg(0, std::ios::end);
        auto size = m_stream.tellg();
        m_stream.seekg(currentPos);
        return static_cast<uint64_t>(size);
    }

    uint64_t StreamFileImpl::GetPosition() const {
        if (!m_stream.is_open()) {
            SRHalt("StreamFileImpl::Read() : file is not open!");
            return 0;
        }
        return static_cast<uint64_t>(m_stream.tellg());
    }

    uint64_t StreamFileImpl::Read(void* data, uint64_t size) {
        if (!m_stream.is_open()) {
            SRHalt("StreamFileImpl::Read() : file is not open!");
            return 0;
        }
        m_stream.read(static_cast<char*>(data), static_cast<std::streamsize>(size));
        return static_cast<uint64_t>(m_stream.gcount());
    }

    uint64_t StreamFileImpl::Write(const void* data, uint64_t size) {
        if (!m_stream.is_open()) {
            SRHalt("StreamFileImpl::Write() : file is not open!");
            return 0;
        }
        m_stream.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
        return size;
    }

    bool StreamFileImpl::Seek(int64_t offset, SeekOrigin origin) {
        if (!m_stream.is_open()) {
            SRHalt("StreamFileImpl::Seek() : file is not open!");
            return false;
        }
        std::ios::seekdir dir;
        switch (origin) {
            case SeekOrigin::Begin:
                dir = std::ios::beg;
                break;
            case SeekOrigin::Current:
                dir = std::ios::cur;
                break;
            case SeekOrigin::End:
                dir = std::ios::end;
                break;
            default:
                SRHalt("StreamFileImpl::Seek() : invalid seek origin!");
                return false;
        }
        m_stream.seekg(offset, dir);
        m_stream.seekp(offset, dir);
        return true;
    }

    StringView StreamFileImpl::Data() {
        SRHalt("StreamFileImpl::Data() : not supported for fstream files!");
        return StringView();
    }

    void StreamFileImpl::Flush() {
        if (m_stream.is_open()) {
            m_stream.flush();
        }
    }

    bool MappedFileImpl::Open(StringView path, FileMode mode) {
        m_mode = mode;
        MappedFile mappedFile = MappedFile::Open({}, path, SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Write));
        if (!mappedFile) {
            return false;
        }
        m_mappedFile = std::move(mappedFile);
        return true;
    }

    void MappedFileImpl::Close() {
        m_mappedFile.Close();
        m_position = 0;
    }

    uint64_t MappedFileImpl::GetSize() const {
        return m_mappedFile.GetSize();
    }

    uint64_t MappedFileImpl::GetPosition() const {
        return m_position;
    }

    uint64_t MappedFileImpl::Read(void *data, uint64_t size) {
        uint64_t remaining = m_mappedFile.GetSize() - m_position;
        uint64_t toRead = std::min(size, remaining);
        if (toRead == 0) {
            return 0;
        }

        std::memcpy(data, m_mappedFile.GetData() + m_position, toRead);
        m_position += toRead;
        return toRead;
    }

    uint64_t MappedFileImpl::Write(const void *data, uint64_t size) {
        uint64_t remaining = m_mappedFile.GetSize() - m_position;
        uint64_t toWrite = std::min(size, remaining);
        if (toWrite == 0) {
            return 0;
        }

        std::memcpy(const_cast<char*>(m_mappedFile.GetData()) + m_position, data, toWrite);
        m_position += toWrite;
        return toWrite;
    }

    bool MappedFileImpl::Seek(int64_t offset, SeekOrigin origin) {
        int64_t newPos = 0;
        switch (origin) {
            case SeekOrigin::Begin:
                newPos = offset;
                break;
            case SeekOrigin::Current:
                newPos = static_cast<int64_t>(m_position) + offset;
                break;
            case SeekOrigin::End:
                newPos = static_cast<int64_t>(m_mappedFile.GetSize()) + offset;
                break;
            default:
                SRHalt("MappedFileImpl::Seek() : invalid seek origin!");
                return false;
        }

        if (newPos < 0 || static_cast<uint64_t>(newPos) > m_mappedFile.GetSize()) {
            SRHalt("MappedFileImpl::Seek() : seek position out of bounds!");
            return false;
        }

        m_position = static_cast<uint64_t>(newPos);
        return true;
    }

    StringView MappedFileImpl::Data() {
        return StringView{ const_cast<char*>(m_mappedFile.GetData()), static_cast<SizeType>(m_mappedFile.GetSize()) };
    }

    MemoryFileImpl::MemoryFileImpl(String&& data)
        : m_data(std::move(data))
    { }

    bool MemoryFileImpl::Open(StringView path, FileMode mode) {
        m_mode = mode;
        m_position = 0;
        /// содержимое уже было передано в конструкторе, открывать нечего
        return true;
    }

    void MemoryFileImpl::Close() {
        m_data.clear();
        m_position = 0;
    }

    uint64_t MemoryFileImpl::GetSize() const {
        return m_data.size();
    }

    uint64_t MemoryFileImpl::GetPosition() const {
        return m_position;
    }

    uint64_t MemoryFileImpl::Read(void* data, uint64_t size) {
        const uint64_t remaining = m_data.size() - m_position;
        const uint64_t toRead = std::min(size, remaining);
        if (toRead == 0) {
            return 0;
        }
        std::memcpy(data, m_data.data() + m_position, toRead);
        m_position += toRead;
        return toRead;
    }

    uint64_t MemoryFileImpl::Write(const void* data, uint64_t size) {
        SRHalt("MemoryFileImpl::Write() : file is read-only!");
        return 0;
    }

    bool MemoryFileImpl::Seek(int64_t offset, SeekOrigin origin) {
        int64_t newPos = 0;
        switch (origin) {
            case SeekOrigin::Begin:
                newPos = offset;
                break;
            case SeekOrigin::Current:
                newPos = static_cast<int64_t>(m_position) + offset;
                break;
            case SeekOrigin::End:
                newPos = static_cast<int64_t>(m_data.size()) + offset;
                break;
            default:
                SRHalt("MemoryFileImpl::Seek() : invalid seek origin!");
                return false;
        }

        if (newPos < 0 || static_cast<uint64_t>(newPos) > m_data.size()) {
            SRHalt("MemoryFileImpl::Seek() : seek position out of bounds!");
            return false;
        }

        m_position = static_cast<uint64_t>(newPos);
        return true;
    }

    StringView MemoryFileImpl::Data() {
        return StringView(m_data.data(), m_data.size());
    }

    void FileImpl::RemoveUse() {
        if (SRVerify(m_useCount > 0)) {
            --m_useCount;
            if (m_useCount == 0) {
                VFS::Instance().CloseFile({}, this);
            }
        }
    }

    void FileImpl::AddUse() {
        ++m_useCount;
    }

    uint32_t FileImpl::GetUseCount() const {
        return m_useCount.load();
    }
}