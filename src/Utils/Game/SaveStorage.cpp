//
// Created by Codex on 25.11.2026.
//

#include <Utils/Game/SaveStorage.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/FileSystem/IVFSBackend.h>
#include <Utils/Platform/Platform.h>

#include <algorithm>
#include <ctime>

namespace SR_UTILS_NS::SaveStorage {
    static String gFaultPoint;

    bool SetFaultPoint(StringView point) {
        if (!point.empty() && point != "after_state" && point != "after_files" && point != "after_metadata" &&
            point != "before_marker" && point != "after_marker") {
            return false;
        }
        gFaultPoint = point;
        return true;
    }

    bool ConsumeFault(StringView point) {
        if (gFaultPoint != point) {
            return false;
        }
        gFaultPoint.clear();
        return true;
    }

    bool ValidId(StringView id) {
        if (id.empty() || id.size() > 64) {
            return false;
        }
        for (const char character : id) {
            if (!((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
                  (character >= '0' && character <= '9') || character == '_' || character == '-')) {
                return false;
            }
        }
        return true;
    }

    bool Read(const Path& path, String& data, uint64_t limit) {
        if (!path.IsFile()) {
            return false;
        }
        auto file = VFS::Instance().OpenFile(path, FileMode::Read);
        if (!file || file.GetSize() > limit) {
            return false;
        }
        const auto length = file.GetSize();
        return file.Read(data) && data.size() == length;
    }

    bool Write(const Path& path, StringView data) {
        return VFS::Instance().WriteFileAtomic(path.ToStringView(), data);
    }

    uint32_t Rotate(uint32_t value, uint32_t bits) {
        return (value >> bits) | (value << (32 - bits));
    }

    String Digest(StringView data) {
        SR_TRACY_ZONE;
        constexpr uint32_t constants[64] = {
            0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
            0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
            0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
            0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
            0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
            0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
            0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
            0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
        };
        uint32_t hash[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
        const uint64_t blocks = (data.size() + 9 + 63) / 64;
        const uint64_t bitLength = data.size() * 8;
        for (uint64_t block = 0; block < blocks; ++block) {
            uint32_t words[64] = {};
            for (uint32_t i = 0; i < 64; ++i) {
                const uint64_t offset = block * 64 + i;
                uint8_t byte = 0;
                if (offset < data.size()) {
                    byte = static_cast<uint8_t>(data[offset]);
                }
                else if (offset == data.size()) {
                    byte = 0x80;
                }
                else if (offset >= blocks * 64 - 8) {
                    byte = static_cast<uint8_t>(bitLength >> ((blocks * 64 - 1 - offset) * 8));
                }
                words[i / 4] |= static_cast<uint32_t>(byte) << (24 - (i % 4) * 8);
            }
            for (uint32_t i = 16; i < 64; ++i) {
                const uint32_t first = Rotate(words[i - 15], 7) ^ Rotate(words[i - 15], 18) ^ (words[i - 15] >> 3);
                const uint32_t second = Rotate(words[i - 2], 17) ^ Rotate(words[i - 2], 19) ^ (words[i - 2] >> 10);
                words[i] = words[i - 16] + first + words[i - 7] + second;
            }
            uint32_t a = hash[0], b = hash[1], c = hash[2], d = hash[3];
            uint32_t e = hash[4], f = hash[5], g = hash[6], h = hash[7];
            for (uint32_t i = 0; i < 64; ++i) {
                const uint32_t first = h + (Rotate(e, 6) ^ Rotate(e, 11) ^ Rotate(e, 25)) + ((e & f) ^ (~e & g)) + constants[i] + words[i];
                const uint32_t second = (Rotate(a, 2) ^ Rotate(a, 13) ^ Rotate(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
                h = g; g = f; f = e; e = d + first;
                d = c; c = b; b = a; a = first + second;
            }
            hash[0] += a; hash[1] += b; hash[2] += c; hash[3] += d;
            hash[4] += e; hash[5] += f; hash[6] += g; hash[7] += h;
        }
        String result;
        constexpr char digits[] = "0123456789abcdef";
        for (uint32_t word : hash) {
            for (int32_t shift = 28; shift >= 0; shift -= 4) {
                result.append(digits[(word >> shift) & 15]);
            }
        }
        return result;
    }

    String UtcNow() {
        const auto now = std::time(nullptr);
        std::tm calendar = {};
        if (!SR_GMTIME(now, calendar)) {
            return String();
        }
        char buffer[32] = {};
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &calendar);
        return String(buffer);
    }

    uint32_t Decimal(StringView text) {
        uint32_t value = 0;
        for (char digit : text) {
            value = value * 10 + digit - '0';
        }
        return value;
    }

    String LocalTime(StringView utc) {
        String text(utc);
        std::tm calendar = {};
        if (utc.size() != 20 || utc[4] != '-' || utc[7] != '-' || utc[10] != 'T' || utc[13] != ':' || utc[16] != ':' || utc[19] != 'Z') {
            return text;
        }
        for (uint32_t i = 0; i < 19; ++i) {
            if (i != 4 && i != 7 && i != 10 && i != 13 && i != 16 && (utc[i] < '0' || utc[i] > '9')) {
                return text;
            }
        }
        calendar.tm_year = static_cast<int32_t>(Decimal(utc.substr(0, 4))) - 1900;
        calendar.tm_mon = static_cast<int32_t>(Decimal(utc.substr(5, 2))) - 1;
        calendar.tm_mday = Decimal(utc.substr(8, 2));
        calendar.tm_hour = Decimal(utc.substr(11, 2));
        calendar.tm_min = Decimal(utc.substr(14, 2));
        calendar.tm_sec = Decimal(utc.substr(17, 2));
        const auto time = SR_TIMEGM(calendar);
        if (time == static_cast<std::time_t>(-1) || !SR_LOCALTIME(time, calendar)) {
            return text;
        }
        const String localized = SR_PLATFORM_NS::FormatLocalDateTime(calendar);
        return localized.empty() ? text : localized;
    }

    uint64_t GenerationId(StringView name) {
        if (!name.starts_with("gen_") || name.size() <= 4 || name.size() > 23) {
            return 0;
        }
        uint64_t result = 0;
        for (uint64_t i = 4; i < name.size(); ++i) {
            if (name[i] < '0' || name[i] > '9' || result > (SR_INT64_MAX - 9) / 10) {
                return 0;
            }
            result = result * 10 + name[i] - '0';
        }
        return result;
    }

    Vector<String> Generations(const Path& path) {
        Vector<String> result;
        if (!path.IsDir()) {
            return result;
        }
        VFS::Instance().Enumerate(path.ToStringView(), [&](const VFSEntry& entry) {
            if (entry.type == FSItemType::Folder && GenerationId(entry.name) > 0) {
                result.emplace_back(entry.name);
            }
        }, false);
        std::sort(result.begin(), result.end(), [](const String& left, const String& right) {
            return GenerationId(left) > GenerationId(right);
        });
        return result;
    }

    bool ValidFilePath(StringView path) {
        if (path.empty() || path.size() > 1024 || path.front() == '/' || path.back() == '/') {
            return false;
        }
        uint64_t segment = 0;
        for (uint64_t i = 0; i <= path.size(); ++i) {
            if (i == path.size() || path[i] == '/') {
                const auto name = path.substr(segment, i - segment);
                if (name.empty() || name.size() > 255 || name == "." || name == ".." ||
                    name.back() == '.' || name.back() == ' ') {
                    return false;
                }
                segment = i + 1;
                continue;
            }
            const auto character = static_cast<uint8_t>(path[i]);
            if (character < 32 || character == 127 || character == 92 || path[i] == ':' ||
                path[i] == '*' || path[i] == '?' || path[i] == '"' || path[i] == '<' || path[i] == '>' || path[i] == '|') {
                return false;
            }
        }
        return true;
    }

    String FilePathKey(StringView path) {
        String result(path);
        for (auto&& character : result) {
            if (character >= 'A' && character <= 'Z') {
                character = static_cast<char>(character - 'A' + 'a');
            }
        }
        return result;
    }

    bool ConflictingFilePaths(StringView left, StringView right) {
        const String first = FilePathKey(left);
        const String second = FilePathKey(right);
        return first == second || first.starts_with(second + "/") || second.starts_with(first + "/");
    }

    bool HasPublication(const Path& path) {
        if (path.Concat("current").IsFile()) {
            return true;
        }
        for (auto&& generation : Generations(path)) {
            if (path.Concat("published_" + generation).IsFile()) {
                return true;
            }
        }
        return false;
    }

    void SetText(JsonValue& object, StringView key, StringView text) {
        JsonValue value;
        value.SetString(text);
        object.Set(key, value);
    }

    void SetInteger(JsonValue& object, StringView key, int64_t integer) {
        JsonValue value;
        value.SetInt(integer);
        object.Set(key, value);
    }
}
