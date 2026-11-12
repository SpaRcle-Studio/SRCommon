#include <Utils/FileSystem/ArchiveExtract.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/Platform/Platform.h>
#include <zlib/zlib.h>

namespace SR_UTILS_NS {
    namespace {
        StringView TarField(const char* bytes, SizeType size) {
            SizeType length = 0;
            while (length < size && bytes[length]) {
                ++length;
            }
            return StringView(bytes, length);
        }

        uint64_t TarOctal(StringView field) {
            uint64_t result = 0;
            for (char character : field) {
                if (character >= '0' && character <= '7') {
                    result = result * 8 + character - '0';
                }
            }
            return result;
        }

        void ReadPax(StringView data, String& name, String& link) {
            while (!data.empty()) {
                const auto space = data.find(' ');
                if (space == StringView::npos) {
                    return;
                }
                const String lengthText(data.substr(0, space));
                const auto length = static_cast<SizeType>(strtoull(lengthText.c_str(), nullptr, 10));
                if (length <= space + 1 || length > data.size()) {
                    return;
                }
                const auto record = data.substr(space + 1, length - space - 2);
                if (record.starts_with("path=")) {
                    name = record.substr(5);
                }
                else if (record.starts_with("linkpath=")) {
                    link = record.substr(9);
                }
                data.remove_prefix(length);
            }
        }

        bool InflateGzip(File& source, String& data) {
            z_stream stream{};
            stream.zalloc = SRZLibAlloc;
            stream.zfree = SRZLibFree;
            if (inflateInit2(&stream, MAX_WBITS + 16) != Z_OK) {
                return false;
            }
            uint8_t input[65536];
            char output[65536];
            int32_t result = Z_OK;
            while (result == Z_OK) {
                if (stream.avail_in == 0) {
                    stream.avail_in = static_cast<uInt>(source.Read(input, sizeof(input)));
                    stream.next_in = input;
                    if (!stream.avail_in) {
                        break;
                    }
                }
                stream.next_out = reinterpret_cast<Bytef*>(output);
                stream.avail_out = sizeof(output);
                result = inflate(&stream, Z_NO_FLUSH);
                data.append(output, sizeof(output) - stream.avail_out);
            }
            inflateEnd(&stream);
            return result == Z_STREAM_END;
        }
    }

    bool ExtractTarGzip(const Path& source, const Path& destination, const ArchiveExtractOptions& options) {
        auto&& vfs = VFS::Instance();
        auto input = vfs.OpenFile(source, FileMode::Read);
        String data;
        if (!input || !InflateGzip(input, data) || !vfs.CreateDirectories(destination.ToStringView())) {
            return false;
        }
        struct Link {
            Path from;
            Path to;
        };
        Vector<Link> links;
        String longName;
        String longLink;
        SizeType position = 0;
        bool terminated = false;
        const String rootPrefix = destination.ToString() + "/";
        while (position + 512 <= data.size()) {
            const char* header = data.data() + position;
            if (header[0] == '\0') {
                terminated = true;
                break;
            }
            const uint64_t size = TarOctal(TarField(header + 124, 12));
            if (size > data.size() - position - 512) {
                return false;
            }
            uint64_t checksum = 0;
            for (SizeType i = 0; i < 512; ++i) {
                checksum += i >= 148 && i < 156 ? 32 : static_cast<uint8_t>(header[i]);
            }
            if (checksum != TarOctal(TarField(header + 148, 8))) {
                return false;
            }
            const StringView contents(header + 512, size);
            position += 512 + static_cast<SizeType>((size + 511) / 512 * 512);
            const char type = header[156];
            if (type == 'x') {
                ReadPax(contents, longName, longLink);
                continue;
            }
            if (type == 'g') {
                continue;
            }
            if (type == 'L' || type == 'K') {
                (type == 'L' ? longName : longLink) = TarField(contents.data(), contents.size());
                continue;
            }
            const bool hasLongName = !longName.empty();
            String name = hasLongName ? std::move(longName) : String(TarField(header, 100));
            const auto prefix = TarField(header + 345, 155);
            if (!prefix.empty() && !hasLongName && TarField(header + 257, 6) == "ustar") {
                name = String(prefix) + "/" + name;
            }
            String linkName = longLink.empty() ? String(TarField(header + 157, 100)) : std::move(longLink);
            longName.clear();
            longLink.clear();
            if (options.stripRoot) {
                const auto slash = name.find('/');
                if (slash == String::npos) {
                    continue;
                }
                name = name.substr(slash + 1);
            }
            if (!options.stripPrefix.empty()) {
                if (!name.starts_with(options.stripPrefix)) {
                    continue;
                }
                name = name.substr(options.stripPrefix.size());
            }
            if (name.empty()) {
                continue;
            }
            if (name.starts_with("/") || name.contains("\\") || name.contains(":") || name.find('\0') != String::npos) {
                return false;
            }
            const Path target = destination.Concat(name);
            if (!target.ToStringView().starts_with(rootPrefix)) {
                return false;
            }
            if (type == '5') {
                if (!vfs.CreateDirectories(target.ToStringView())) {
                    return false;
                }
            }
            else if (type == '1' || type == '2') {
                if (linkName.starts_with("/") || linkName.contains(":") || linkName.contains("\\")) {
                    return false;
                }
                if (type == '1' && options.stripRoot) {
                    const auto slash = linkName.find('/');
                    if (slash == String::npos) {
                        return false;
                    }
                    linkName = linkName.substr(slash + 1);
                }
                const Path from = type == '1' ? destination.Concat(linkName) : target.GetPrevious().Concat(linkName);
                if (!from.ToStringView().starts_with(rootPrefix)) {
                    return false;
                }
                links.emplace_back(Link{from, target});
            }
            else if (type == '0' || type == '\0') {
                if (!vfs.CreateDirectories(target.GetPrevious().ToStringView())) {
                    return false;
                }
                auto output = vfs.OpenFile(target, FileMode::Write);
                if (!output || output.Write(contents.data(), contents.size()) != contents.size()) {
                    return false;
                }
                output = {};
                if ((TarOctal(TarField(header + 100, 8)) & 0111) != 0) {
                    Platform::CopyPermissions(Platform::GetApplicationPath(), target);
                }
            }
            else {
                return false;
            }
        }
        for (SizeType pass = 0; pass < links.size(); ++pass) {
            for (auto& link : links) {
                if (!link.to.empty() && vfs.IsExists(link.from.ToStringView())) {
                    if (!vfs.Copy(link.from.ToStringView(), link.to.ToStringView())) {
                        return false;
                    }
                    link.to.clear();
                }
            }
        }
        return terminated && std::all_of(links.begin(), links.end(), [](const Link& link) {
            return link.to.empty();
        });
    }
}
