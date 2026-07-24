//
// Created by Monika on 12.03.2022.
//

#ifndef SR_ENGINE_MARSHAL_H
#define SR_ENGINE_MARSHAL_H

#include <Utils/Common/MarshalUtils.h>

#include <Utils/FileSystem/Path.h>
#include <Utils/Math/Vector2.h>
#include <Utils/Math/Vector3.h>
#include <Utils/Common/NonCopyable.h>
#include <Utils/Types/Stream.h>
#include <Utils/Common/StringFormat.h>

namespace SR_HTYPES_NS {
    class SR_COMMON_DLL_API Marshal : public Stream {
    public:
        using Super = Stream;
        using Ptr = Marshal*;

    public:
        Marshal();
        Marshal(const MappedFile& mappedFile);
        Marshal(std::ifstream& ifs); /** NOLINT */
        Marshal(const std::string& str); /** NOLINT */
        Marshal(const char* pData, uint64_t size, bool copy = true);
        ~Marshal();

    public:
        bool Save(const Path& path) const; /** NOLINT */
        SR_NODISCARD Marshal Copy() const;
        SR_NODISCARD Marshal::Ptr CopyPtr() const;

        SR_NODISCARD Marshal FullCopy() const;

        static Marshal Load(const Path& path);
        static Marshal::Ptr LoadPtr(const Path& path);
        static Marshal LoadFromMemory(const std::string& data);
        static Marshal LoadFromBase64(const std::string& base64);

        void Append(Marshal&& marshal);
        void Append(std::unique_ptr<Marshal>&& pMarshal);
        void Append(Marshal::Ptr& pMarshal);

        SR_NODISCARD Marshal ReadBytes(uint64_t count) noexcept;
        SR_NODISCARD Marshal::Ptr ReadBytesPtr(uint64_t count) noexcept;

        void WriteBlock(void* pData, uint64_t size);
        void ReadBlock(void* pDestination);
        void* ReadMapBlock();

        template<typename T> void Write(const T& value);
        template<typename T> void Write(const T& value, const T& def);
        template<typename T> T View(uint64_t offset) const;
        template<typename T> T TryRead();
        template<typename T> T Read();
        template<typename T> T Read(const T& def);

        uint16_t ReadShortStringLength();
        void ReadChars(char* pBuffer, uint64_t size);
    };

    template<typename T> void Marshal::Write(const T& value, const T& def) {
        if (value == def) {
            Write<bool>(true);
        }
        else {
            Write<bool>(false);
            Write<T>(value);
        }
    }

    template<typename T> T Marshal::View(uint64_t offset) const {
        T value = T();

        memcpy(&value, Super::View() + offset, sizeof(T));

        return value;
    }

    template<typename T> T Marshal::TryRead() {
        if constexpr (IsString<T>()) {
            return MarshalUtils::TryLoadShortStr(*this);
        }
        else if constexpr (std::is_same_v<T, UnicodeString>) {
            return MarshalUtils::TryLoadUnicodeString(*this);
        }
    }

    template<typename T> T Marshal::Read() {
        if constexpr (std::is_same_v<T, std::any>) {
            return MarshalUtils::LoadAny<std::any>(*this);
        }
        else if constexpr (std::is_same_v<T, UnicodeString>) {
            return MarshalUtils::LoadUnicodeString(*this);
        }
        else if constexpr (IsStringV<T>) {
            return MarshalUtils::LoadShortStr(*this);
        }
        else if constexpr (IsVectorV<T>) {
            return MarshalUtils::LoadVector<T>(*this);
        }
        else {
            return MarshalUtils::LoadValue<T>(*this);
        }
    }

    template<typename T> void Marshal::Write(const T &value) {
        if constexpr (std::is_same_v<T, std::any>) {
            MarshalUtils::SaveAny<std::any>(*this, value);
        }
        else if constexpr (std::is_same_v<T, StringAtom>) {
            MarshalUtils::SaveShortString(*this, value.ToStringRef());
        }
        else if constexpr (IsStringV<T>) {
            MarshalUtils::SaveShortString(*this, value);  //нужно вызывать Write<std::string>()
        }
        else if constexpr (std::is_same_v<T, UnicodeString>) {
            MarshalUtils::SaveUnicodeString(*this, value);
        }
        else if constexpr (IsVectorV<T>) {
            MarshalUtils::SaveVector(*this, value);
        }
        else {
            MarshalUtils::SaveValue(*this, value);
        }
    }

    template<typename T> T Marshal::Read(const T &def) {
        if (Read<bool>()) {
            return def;
        }

        return Read<T>();
    }
}

#endif //SR_ENGINE_MARSHAL_H
