//
// Created by Monika on 14.09.2026.
//

#include <Utils/FileSystem/Archive.h>
#include <Utils/FileSystem/File.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Platform/Platform.h>

namespace SR_UTILS_NS {
    namespace Zip {
        constexpr uint32_t LocalFileSignature        = 0x04034b50;
        constexpr uint32_t CentralDirectorySignature = 0x02014b50;
        constexpr uint32_t EndOfDirectorySignature   = 0x06054b50;
        constexpr uint32_t DataDescriptorSignature   = 0x08074b50;

        constexpr uint16_t CompressionStored  = 0;
        constexpr uint16_t CompressionDeflate = 8;

        constexpr uint16_t FlagDataDescriptor = 1 << 3;
        constexpr uint16_t FlagEncrypted      = 1 << 0;

        struct LocalFileHeader {
            uint32_t signature;
            uint16_t versionNeeded;
            uint16_t flags;
            uint16_t compression;
            uint16_t modTime;
            uint16_t modDate;
            uint32_t crc32;
            uint32_t compressedSize;
            uint32_t uncompressedSize;
            uint16_t fileNameLength;
            uint16_t extraLength;
        };

        struct CentralDirectoryHeader {
            uint32_t signature;
            uint16_t versionMadeBy;
            uint16_t versionNeeded;
            uint16_t flags;
            uint16_t compression;
            uint16_t modTime;
            uint16_t modDate;
            uint32_t crc32;
            uint32_t compressedSize;
            uint32_t uncompressedSize;
            uint16_t fileNameLength;
            uint16_t extraLength;
            uint16_t commentLength;
            uint16_t diskNumber;
            uint16_t internalAttributes;
            uint32_t externalAttributes;
            uint32_t localHeaderOffset;
        };

        struct EndOfDirectory {
            uint32_t signature;
            uint16_t diskNumber;
            uint16_t centralDirectoryDisk;
            uint16_t diskEntries;
            uint16_t totalEntries;
            uint32_t centralDirectorySize;
            uint32_t centralDirectoryOffset;
            uint16_t commentLength;
        };

        uint16_t ReadU16(const uint8_t* data) {
            return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
        }

        uint32_t ReadU32(const uint8_t* data) {
            return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
        }

        static bool Inflate(const uint8_t* compressedData, size_t compressedSize, uint8_t* outputData, size_t outputSize) {
            z_stream stream{};

            stream.zalloc = SRZLibAlloc;
            stream.zfree = SRZLibFree;
            stream.next_in = const_cast<Bytef*>( reinterpret_cast<const Bytef*>(compressedData));
            stream.avail_in = static_cast<uInt>(compressedSize);
            stream.next_out = reinterpret_cast<Bytef*>(outputData);
            stream.avail_out = static_cast<uInt>(outputSize);

            /*
             * ZIP использует raw DEFLATE.
             *
             * Обычный inflateInit() здесь использовать нельзя,
             * потому что он ожидает zlib header.
             */
            int32_t result = inflateInit2(&stream, -MAX_WBITS);
            if (result != Z_OK) {
                return false;
            }

            result = inflate(&stream, Z_FINISH);
            const bool success = result == Z_STREAM_END && stream.total_out == outputSize;

            inflateEnd(&stream);

            return success;
        }
    }

    ArchiveType DetectArchiveType(const void* data, SizeType size) {
        if (!data || size < 4) {
            return ArchiveType::Unknown;
        }

        const uint8_t* bytes = static_cast<const uint8_t*>(data);

        /// ZIP: PK\x03\x04
        if (size >= 4 && bytes[0] == 0x50 && bytes[1] == 0x4B && bytes[2] == 0x03 && bytes[3] == 0x04) {
            return ArchiveType::Zip;
        }

        /// ZIP empty archive / central directory / spanning variants.
        if (size >= 4 && bytes[0] == 0x50 && bytes[1] == 0x4B &&
            (
                (bytes[2] == 0x05 && bytes[3] == 0x06) ||
                (bytes[2] == 0x07 && bytes[3] == 0x08)
            ))
        {
            return ArchiveType::Zip;
        }

        // 7z: 37 7A BC AF 27 1C
        if (size >= 6 && bytes[0] == 0x37 && bytes[1] == 0x7A && bytes[2] == 0xBC && bytes[3] == 0xAF && bytes[4] == 0x27 && bytes[5] == 0x1C) {
            return ArchiveType::SevenZip;
        }

        return ArchiveType::Unknown;
    }

    RawPointerHolder<Archive> Archive::Load(StringView data) {
        SR_TRACY_ZONE;

        RawPointerHolder<Archive> pArchive = new Archive();
        pArchive->m_archiveData = data;

        ArchiveType type = DetectArchiveType(data.data(), data.size());
        switch (type) {
            case ArchiveType::Zip:
                if (pArchive->UnpackZip()) {
                    return pArchive;
                }
                break;
            case ArchiveType::SevenZip:
                SR_ERROR("Archive::Load() : 7z archives are not supported!");
                return RawPointerHolder<Archive>();
            default:
                SR_ERROR("Archive::Load() : unknown archive type!");
                return RawPointerHolder<Archive>();
        }

        SR_ERROR("Archive::Load() : failed to unpack archive!");
        return RawPointerHolder<Archive>();
    }

    RawPointerHolder<Archive> Archive::Load(File file) {
        SR_TRACY_ZONE;
        if (SR_MATH_NS::IsMaskIncludedSubMask(file.GetMode(), FileMode::Map)) {
            return Load(file.Data());
        }
        else {
            String data;
            if (file.Read(data)) {
                return Load(data);
            }
        }
        SR_ERROR("Archive::Load() : failed to read file!");
        return RawPointerHolder<Archive>();
    }

    bool Archive::UnpackZip() {
        SR_TRACY_ZONE;

    #ifndef SR_COMMON_ZLIB
        SRHalt("Archive::UnpackZip() : zlib is not supported!");
        return false;
    #endif

        const SizeType archiveSize = m_archiveData.size();
        const auto pData = (const uint8_t*)m_archiveData.data();

        String pathTemp;
        Vector<StringView> pathComponents;
        Vector<uint8_t> decompressedData;

        if (!pData || archiveSize < sizeof(Zip::EndOfDirectory)) {
            SR_WARN("Archive::UnpackZip() : archive data is too small!");
            return false;
        }

        /**
        * ------------------------------------------------------------
        * 1. Ищем End Of Central Directory
        * ------------------------------------------------------------
        *
        * EOCD находится в конце ZIP, но перед ним может быть
        * комментарий длиной до 65535 байт.
        */

        constexpr SizeType eocdSize = 22;

        SizeType eocdOffset = std::numeric_limits<SizeType>::max();
        const SizeType searchStart = archiveSize > (eocdSize + 65535) ? archiveSize - eocdSize - 65535 : 0;

        for (SizeType i = archiveSize - eocdSize;; --i) {
            if (Zip::ReadU32(pData + i) == Zip::EndOfDirectorySignature) {
                eocdOffset = i;
                break;
            }
            if (i == searchStart) {
                break;
            }
        }

        if (eocdOffset == std::numeric_limits<SizeType>::max()) {
            SR_WARN("Archive::UnpackZip() : End Of Central Directory not found!");
            return false;
        }

        const uint8_t* eocd = pData + eocdOffset;
        const uint16_t totalEntries = Zip::ReadU16(eocd + 10);
        const uint32_t centralDirectorySize = Zip::ReadU32(eocd + 12);
        const uint32_t centralDirectoryOffset = Zip::ReadU32(eocd + 16);

        /** ZIP64 здесь пока не поддерживаем. */

        if (totalEntries == 0xFFFF || centralDirectorySize == 0xFFFFFFFF || centralDirectoryOffset == 0xFFFFFFFF) {
            SR_WARN("Archive::UnpackZip() : ZIP64 is not supported!");
            return false;
        }

        if (static_cast<uint64_t>(centralDirectoryOffset) + static_cast<uint64_t>(centralDirectorySize) > archiveSize) {
            SR_WARN("Archive::UnpackZip() : central directory is out of bounds!");
            return false;
        }

        /**
         * ------------------------------------------------------------
         * 2. Читаем Central Directory
         * ------------------------------------------------------------
         */

        const uint8_t* central = pData + centralDirectoryOffset;

        m_entries.resize(totalEntries);

        {
            SizeType totalDataSize = 0;
            SizeType centralPosition = 0;

            for (uint16_t entryIndex = 0; entryIndex < totalEntries; ++entryIndex) {
                if (centralPosition + 46 > centralDirectorySize) {
                    SR_WARN("Archive::UnpackZip() : central directory entry {} is out of bounds!", entryIndex);
                    return false;
                }

                const uint8_t* header = central + centralPosition;

                if (Zip::ReadU32(header) != Zip::CentralDirectorySignature) {
                    SR_WARN("Archive::UnpackZip() : central directory entry {} has invalid signature!", entryIndex);
                    return false;
                }

                const uint32_t uncompressedSize = Zip::ReadU32(header + 24);
                const uint16_t fileNameLength = Zip::ReadU16(header + 28);
                const uint16_t extraLength = Zip::ReadU16(header + 30);
                const uint16_t commentLength = Zip::ReadU16(header + 32);

                const SizeType centralEntrySize = 46 + fileNameLength + extraLength + commentLength;

                if (centralPosition + centralEntrySize > centralDirectorySize) {
                    SR_WARN("Archive::UnpackZip() : central directory entry {} is out of bounds!", entryIndex);
                    return false;
                }

                const char* fileName = reinterpret_cast<const char*>(header + 46);

                /// ZIP directory entry.
                const bool isDirectory = fileNameLength > 0 && (fileName[fileNameLength - 1] == '/' || fileName[fileNameLength - 1] == '\\');

                if (!isDirectory) {
                    totalDataSize += uncompressedSize;
                    totalDataSize += fileNameLength + 1;
                }

                centralPosition += centralEntrySize;
            }
            m_unpackedData.resize(totalDataSize);
        }

        SizeType centralPosition = 0;
        for (uint16_t entryIndex = 0; entryIndex < totalEntries; ++entryIndex) {
            if (centralPosition + 46 > centralDirectorySize) {
                SR_WARN("Archive::UnpackZip() : central directory entry {} is out of bounds!", entryIndex);
                return false;
            }

            const uint8_t* header = central + centralPosition;

            if (Zip::ReadU32(header) != Zip::CentralDirectorySignature) {
                SR_WARN("Archive::UnpackZip() : central directory entry {} has invalid signature!", entryIndex);
                return false;
            }

            SR_MAYBE_UNUSED const uint16_t flags = Zip::ReadU16(header + 8);
            const uint16_t compression = Zip::ReadU16(header + 10);
            const uint32_t crc = Zip::ReadU32(header + 16);
            const uint32_t compressedSize = Zip::ReadU32(header + 20);
            const uint32_t uncompressedSize = Zip::ReadU32(header + 24);
            const uint16_t fileNameLength = Zip::ReadU16(header + 28);
            const uint16_t extraLength = Zip::ReadU16(header + 30);
            const uint16_t commentLength = Zip::ReadU16(header + 32);
            const uint32_t localHeaderOffset = Zip::ReadU32(header + 42);
            const SizeType centralEntrySize = 46 + fileNameLength + extraLength + commentLength;

            if (centralPosition + centralEntrySize > centralDirectorySize) {
                SR_WARN("Archive::UnpackZip() : central directory entry {} is out of bounds!", entryIndex);
                return false;
            }

            const char* fileName = reinterpret_cast<const char*>(header + 46);

            /**
            * --------------------------------------------------------
            * Защита от ../ и абсолютных путей.
            * Нельзя позволять ZIP писать куда угодно.
            * --------------------------------------------------------
            */

            pathTemp.resize(fileNameLength);
            memcpy(pathTemp.data(), fileName, fileNameLength);

            if (pathTemp.empty()) {
                centralPosition += centralEntrySize;
                continue;
            }

            /**
            * ZIP использует '/' независимо от платформы.
            */

            StringUtils::Instance().ReplaceChars(pathTemp, '\\', '/');

            if (SR_PLATFORM_NS::IsAbsolutePath(pathTemp)) {
                SR_WARN("Archive::UnpackZip() : zip entry \"{}\" is absolute path!", pathTemp);
                return false;
            }

            StringUtils::Instance().SplitView(pathTemp, "/", pathComponents);
            for (const auto& component : pathComponents) {
                if (component == "..") {
                    SR_WARN("Archive::UnpackZip() : zip entry \"{}\" contains \"..\" component!", pathTemp);
                    return false;
                }
            }

            /**
            * --------------------------------------------------------
            * 3. Получаем Local File Header
            * --------------------------------------------------------
            */

            if (static_cast<uint64_t>(localHeaderOffset) + 30 > archiveSize) {
                SR_WARN("Archive::UnpackZip() : local file header for entry \"{}\" is out of bounds!", pathTemp);
                return false;
            }

            const uint8_t* local = pData + localHeaderOffset;
            if (Zip::ReadU32(local) != Zip::LocalFileSignature) {
                SR_WARN("Archive::UnpackZip() : local file header for entry \"{}\" has invalid signature!", pathTemp);
                return false;
            }

            const uint16_t localFileNameLength = Zip::ReadU16(local + 26);
            const uint16_t localExtraLength = Zip::ReadU16(local + 28);
            const size_t compressedDataOffset = static_cast<size_t>(localHeaderOffset) + 30 + localFileNameLength + localExtraLength;

            if (static_cast<uint64_t>(compressedDataOffset) + compressedSize > archiveSize) {
                SR_WARN("Archive::UnpackZip() : compressed data for entry \"{}\" is out of bounds!", pathTemp);
                return false;
            }

            const uint8_t* compressedData = pData + compressedDataOffset;

            /**
            * --------------------------------------------------------
            * 4. Если это directory entry.
            * --------------------------------------------------------
            */

            if (!pathTemp.empty()) {
                const char lastCharacter = pathTemp[pathTemp.size() - 1];
                if (lastCharacter == '/') {
                    Entry& entry = m_entries[entryIndex];
                    m_unpackedData += pathTemp;
                    entry.name = StringView(m_unpackedData.data() + m_unpackedData.size() - pathTemp.size(), pathTemp.size());
                    entry.isDirectory = true;
                    centralPosition += centralEntrySize;
                    continue;
                }
            }

            /**
            * --------------------------------------------------------
            * 5. Распаковываем
            * --------------------------------------------------------
            */

            decompressedData.resize(uncompressedSize);
            bool success = false;
            if (compression == Zip::CompressionStored) {
                if (compressedSize != uncompressedSize) {
                    SR_WARN("Archive::UnpackZip() : zip entry \"{}\" has mismatched compressed and uncompressed sizes!", pathTemp);
                    return false;
                }
                if (uncompressedSize != 0) {
                    std::memcpy(decompressedData.data(), compressedData, uncompressedSize);
                }
                success = true;
            }
            else if (compression == Zip::CompressionDeflate) {
                if (uncompressedSize != 0) {
                    success = Zip::Inflate(compressedData, compressedSize, decompressedData.data(), uncompressedSize);
                }
                else {
                    /**
                    * Пустой DEFLATE-файл тоже допустим.
                    */
                    success = Zip::Inflate(compressedData, compressedSize, nullptr, 0);
                }
            }
            else {
                /**
                * BZIP2, LZMA, ZSTD и прочее здесь не поддерживаются.
                */
                SR_WARN("Archive::UnpackZip() : zip entry \"{}\" uses unsupported compression method {}!", pathTemp, compression);
                return false;
            }

            if (!success) {
                SR_WARN("Archive::UnpackZip() : failed to decompress zip entry \"{}\"!", pathTemp);
                return false;
            }

            /**
            * --------------------------------------------------------
            * 6. Проверяем CRC32
            * --------------------------------------------------------
            */

        #ifdef SR_COMMON_ZLIB
            const uint32_t calculatedCRC = crc32(crc32(0L, Z_NULL, 0), decompressedData.data(), static_cast<uInt>(decompressedData.size()));
            if (calculatedCRC != crc) {
                return false;
            }
        #endif

            Entry& entry = m_entries[entryIndex];
            m_unpackedData += pathTemp;
            entry.name = StringView(m_unpackedData.data() + m_unpackedData.size() - pathTemp.size(), pathTemp.size());
            entry.data = StringView((const char*)decompressedData.data(), decompressedData.size());

            centralPosition += centralEntrySize;
        }

        return true;
    }

    void Archive::Enumerate(const Archive::EnumerateCallback& callback) const {
        for (const auto& entry : m_entries) {
            callback(entry);
        }
    }
}
