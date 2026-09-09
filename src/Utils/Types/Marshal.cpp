//
// Created by Monika on 12.03.2022.
//

#include <Utils/Types/Marshal.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_HTYPES_NS {
    Marshal::Marshal(std::ifstream& ifs)
        : Super(ifs)
    { }

    Marshal::Marshal(StringView str)
        : Super(str)
    { }

    Marshal::Marshal(const char *pData, uint64_t size, bool copy)
        : Super(pData, size, copy)
    { }

    void Marshal::Append(Marshal&& marshal) {
        if (marshal && marshal.Size() > 0) {
            Super::Write(marshal.Super::View(), marshal.Size());
        }
    }

    void Marshal::Append(Marshal::Ptr& pMarshal) {
        if (pMarshal && *pMarshal && pMarshal->Size() > 0) {
            Super::Write(pMarshal->Super::View(), pMarshal->Size());
        }

        SR_SAFE_DELETE_PTR(pMarshal);
    }

    void Marshal::Append(std::unique_ptr<Marshal>&& pMarshal) {
        if (pMarshal && *pMarshal && pMarshal->Size() > 0) {
            Super::Write(pMarshal->Super::View(), pMarshal->Size());
        }
    }

    bool Marshal::Save(const Path& path) const {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(path);

        auto&& file = VFS::Instance().OpenFile(path, FileMode::Write);
        if (!file) {
            return false;
        }
        file.Write(Super::View(), Size());
        return true;
    }

    Marshal Marshal::Load(const Path& path) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(path.ToStringRef());

        auto&& file = VFS::Instance().OpenFile(path, FileMode::Read);
        if (!file) {
            SR_ERROR("Marshal::Load() : failed to open file: {}", path);
            return Marshal();
        }

        const auto size = file.GetSize();
        char* buffer = Allocate(size);
        {
            SR_TRACY_ZONE_N("Read");
            file.Read(buffer, size);
        }
        return Marshal(buffer, size, false);
    }

    Marshal Marshal::Copy() const {
        return *this;
    }

    Marshal::Ptr Marshal::CopyPtr() const {
        return new Marshal(*this);
    }

    Marshal Marshal::LoadFromMemory(StringView data) {
        return Marshal(data);
    }

    Marshal Marshal::LoadFromBase64(StringView base64) {
        String decoded;
        StringUtils::Instance().Base64Decode(base64, decoded);
        return LoadFromMemory(decoded);
    }

    Marshal Marshal::ReadBytes(uint64_t count) noexcept {
        if (GetPosition() + count > GetCapacity()) {
            SRHalt("Invalid range!");
            return Marshal(); /// NOLINT
        }

        auto&& marshal = Marshal(Super::View() + GetPosition(), count);
        Skip(count);
        return marshal;
    }

    Marshal::Ptr Marshal::ReadBytesPtr(uint64_t count) noexcept {
        if (GetPosition() + count > GetCapacity()) {
            SRHalt("Invalid range!");
            return nullptr;
        }

        auto&& pMarshal = new Marshal(Super::View() + GetPosition(), count);

        Skip(count);

        return pMarshal;
    }

    Marshal Marshal::FullCopy() const {
        Marshal copy = *this;
        copy.SetPosition(GetPosition());
        return std::move(copy);
    }

    void Marshal::WriteBlock(void *pData, uint64_t size) {
        Write<uint64_t>(size);

        if (size == 0) {
            return;
        }

        write(pData, size);
    }

    void Marshal::ReadBlock(void *pDestination) {
        const auto size = Read<uint64_t>();

        if (size == 0) {
            return;
        }

        read(pDestination, size);
    }

    void* Marshal::ReadMapBlock() {
        const auto size = Read<uint64_t>();

        if (size == 0) {
            return nullptr;
        }

        return readMap(size);
    }

    uint16_t Marshal::ReadShortStringLength() {
        return Read<uint16_t>();
    }

    void Marshal::ReadChars(char *pBuffer, uint64_t size) {
        read(pBuffer, size);
    }

    Marshal::Marshal(const MappedFile &mappedFile)
        : Stream(mappedFile)
    { }

    Marshal::Marshal(const File& file)
        : Stream(file)
    { }

    Marshal::Marshal() = default;
    Marshal::~Marshal() = default;
}