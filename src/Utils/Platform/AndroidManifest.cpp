//
// Created by OpenAI Codex on 12.11.2026.
//

#include <Utils/Platform/AndroidManifest.h>
#include <Utils/Types/Vector.h>

namespace SR_UTILS_NS {
    namespace {
        constexpr uint32_t NoString = 0xffffffff;
        constexpr StringView AndroidNamespace = "http://schemas.android.com/apk/res/android";

        uint16_t Read16(StringView data, SizeType offset) {
            return static_cast<uint8_t>(data[offset]) | (static_cast<uint8_t>(data[offset + 1]) << 8);
        }

        uint32_t Read32(StringView data, SizeType offset) {
            return Read16(data, offset) | (static_cast<uint32_t>(Read16(data, offset + 2)) << 16);
        }

        void Write32(String& data, SizeType offset, uint32_t value) {
            for (uint32_t i = 0; i < 4; ++i) {
                data[offset + i] = static_cast<char>(value >> (i * 8));
            }
        }

        void Append16(String& data, uint16_t value) {
            data += static_cast<char>(value);
            data += static_cast<char>(value >> 8);
        }

        bool ValidPackage(StringView package) {
            if (package.empty() || package.size() > 255 || !package.contains(".")) {
                return false;
            }
            bool first = true;
            for (const char value : package) {
                if (value == '.') {
                    if (first) {
                        return false;
                    }
                    first = true;
                    continue;
                }
                const bool letter = (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z');
                if (!letter && (first || (value != '_' && (value < '0' || value > '9')))) {
                    return false;
                }
                first = false;
            }
            return !first;
        }

        // Existing pool entries and resource IDs retain their indices. Only new strings are appended.
        class ManifestEditor {
        public:
            explicit ManifestEditor(StringView data)
                : m_data(data)
            { }

            bool Rewrite(StringView package, String& output) {
                if (m_data.size() < 8 || m_data.size() > SR_UINT32_MAX || Read16(m_data, 0) != 3 ||
                    Read16(m_data, 2) != 8 || Read32(m_data, 4) != m_data.size())
                {
                    return false;
                }
                SizeType offset = 8;
                while (offset < m_data.size()) {
                    if (!Chunk(offset)) {
                        return false;
                    }
                    if (Read16(m_data, offset) == 1) {
                        if (m_pool != 0 || !LoadPool(offset)) {
                            return false;
                        }
                    }
                    else if (Read16(m_data, offset) == 0x102) {
                        if (m_pool == 0 || !RewriteElement(offset, package)) {
                            return false;
                        }
                    }
                    offset += Read32(m_data, offset + 4);
                }
                if (!m_hasPackage) {
                    return false;
                }
                String pool;
                if (!SavePool(pool)) {
                    return false;
                }
                output = m_data.substr(0, m_pool);
                output += pool;
                output += m_data.substr(m_pool + m_poolSize);
                if (output.size() > SR_UINT32_MAX) {
                    return false;
                }
                Write32(output, 4, static_cast<uint32_t>(output.size()));
                return true;
            }

        private:
            bool Chunk(SizeType offset) const {
                if (offset > m_data.size() || m_data.size() - offset < 8) {
                    return false;
                }
                const uint32_t size = Read32(m_data, offset + 4);
                const uint16_t header = Read16(m_data, offset + 2);
                return header >= 8 && size >= header && size <= m_data.size() - offset;
            }

            bool LoadPool(SizeType offset) {
                m_pool = offset;
                m_poolSize = Read32(m_data, offset + 4);
                m_header = Read16(m_data, offset + 2);
                if (m_header < 28) {
                    return false;
                }
                m_count = Read32(m_data, offset + 8);
                m_styles = Read32(m_data, offset + 12);
                m_utf8 = (Read32(m_data, offset + 16) & 0x100) != 0;
                m_stringsStart = Read32(m_data, offset + 20);
                m_stylesStart = Read32(m_data, offset + 24);
                const uint64_t offsetsEnd = m_header + (static_cast<uint64_t>(m_count) + m_styles) * 4;
                return offsetsEnd <= m_stringsStart && m_stringsStart <= m_poolSize &&
                    (m_stylesStart == 0 ? m_styles == 0 : m_stylesStart >= m_stringsStart && m_stylesStart <= m_poolSize);
            }

            bool Length(SizeType& offset, SizeType end, uint32_t& length) const {
                const uint32_t unit = m_utf8 ? 1 : 2;
                if (offset > end || end - offset < unit) {
                    return false;
                }
                length = m_utf8 ? static_cast<uint8_t>(m_data[offset]) : Read16(m_data, offset);
                offset += unit;
                const uint32_t high = m_utf8 ? 0x80 : 0x8000;
                if (length & high) {
                    if (end - offset < unit) {
                        return false;
                    }
                    length = ((length & (high - 1)) << (m_utf8 ? 8 : 16)) |
                        (m_utf8 ? static_cast<uint8_t>(m_data[offset]) : Read16(m_data, offset));
                    offset += unit;
                }
                return true;
            }

            bool Text(uint32_t index, String& result) const {
                if (index == NoString) {
                    result.clear();
                    return true;
                }
                if (index >= m_count) {
                    return false;
                }
                SizeType offset = m_pool + m_stringsStart + Read32(m_data, m_pool + m_header + index * 4ull);
                const SizeType end = m_pool + (m_stylesStart ? m_stylesStart : m_poolSize);
                uint32_t length = 0;
                if (!Length(offset, end, length)) {
                    return false;
                }
                if (m_utf8 && !Length(offset, end, length)) {
                    return false;
                }
                const uint64_t bytes = static_cast<uint64_t>(length) * (m_utf8 ? 1 : 2);
                if (offset > end || bytes + (m_utf8 ? 1 : 2) > end - offset) {
                    return false;
                }
                result.clear();
                if (m_utf8) {
                    result = m_data.ToStringView().substr(offset, length);
                    return true;
                }
                // Manifest identifiers are ASCII. Non-ASCII UI strings are preserved without decoding.
                for (uint32_t i = 0; i < length; ++i) {
                    const uint16_t value = Read16(m_data, offset + i * 2ull);
                    if (value > 127) {
                        return false;
                    }
                    result += static_cast<char>(value);
                }
                return true;
            }

            bool SetText(SizeType attribute, StringView text) {
                if (text.size() > 0x7fff || m_extra.size() >= NoString - m_count) {
                    return false;
                }
                // Only package and class identifiers are added, so both encodings use ASCII lengths.
                for (const auto value : text) {
                    if (static_cast<uint8_t>(value) > 127) {
                        return false;
                    }
                }
                const auto index = static_cast<uint32_t>(m_count + m_extra.size());
                m_extra.emplace_back(text);
                Write32(m_data, attribute + 8, index);
                Write32(m_data, attribute + 16, index);
                m_data[attribute + 15] = 3;
                return true;
            }

            bool RewriteElement(SizeType offset, StringView package) {
                if (Read16(m_data, offset + 2) < 16) {
                    return false;
                }
                const SizeType extension = offset + Read16(m_data, offset + 2);
                const SizeType end = offset + Read32(m_data, offset + 4);
                if (end - extension < 20) {
                    return false;
                }
                String tag;
                if (!Text(Read32(m_data, extension + 4), tag)) {
                    return false;
                }
                const uint16_t start = Read16(m_data, extension + 8);
                const uint16_t size = Read16(m_data, extension + 10);
                const uint16_t count = Read16(m_data, extension + 12);
                if (start < 20 || size < 20 || static_cast<uint64_t>(start) + static_cast<uint64_t>(count) * size > end - extension) {
                    return false;
                }
                for (uint32_t i = 0; i < count; ++i) {
                    const SizeType attribute = extension + start + i * size;
                    String name;
                    String ns;
                    if (!Text(Read32(m_data, attribute + 4), name) || !Text(Read32(m_data, attribute), ns)) {
                        return false;
                    }
                    if (tag == "manifest" && name == "package" && ns.empty()) {
                        if (m_hasPackage || static_cast<uint8_t>(m_data[attribute + 15]) != 3 ||
                            !Text(Read32(m_data, attribute + 16), m_oldPackage) || !ValidPackage(m_oldPackage))
                        {
                            return false;
                        }
                        m_hasPackage = SetText(attribute, package);
                        if (!m_hasPackage) {
                            return false;
                        }
                    }
                    else if (ns == AndroidNamespace && static_cast<uint8_t>(m_data[attribute + 15]) == 3) {
                        const bool component = tag == "application" || tag == "activity" || tag == "activity-alias" ||
                            tag == "service" || tag == "receiver" || tag == "provider" || tag == "instrumentation";
                        const bool className = (component && name == "name") || name == "targetActivity" ||
                            name == "parentActivityName" || name == "backupAgent" || name == "appComponentFactory" || name == "manageSpaceActivity";
                        const bool appId = name == "authorities" || name == "permission" || name == "readPermission" ||
                            name == "writePermission" || name == "taskAffinity" || name == "targetPackage" ||
                            (name == "name" && (tag == "permission" || tag == "permission-tree" || tag == "permission-group" || tag == "uses-permission"));
                        if (!className && !appId) {
                            continue;
                        }
                        String value;
                        if (!m_hasPackage || !Text(Read32(m_data, attribute + 16), value)) {
                            return false;
                        }
                        if (className && !value.empty() && (value.front() == '.' || !value.contains("."))) {
                            const String qualified = m_oldPackage + (value.front() == '.' ? "" : ".") + value;
                            if (!SetText(attribute, qualified)) {
                                return false;
                            }
                        }
                        else if (appId && (value == m_oldPackage || value.starts_with(m_oldPackage + "."))) {
                            if (!SetText(attribute, String(package) + value.ToStringView().substr(m_oldPackage.size()))) {
                                return false;
                            }
                        }
                    }
                }
                return true;
            }

            bool SavePool(String& pool) const {
                const uint64_t newStart = m_stringsStart + m_extra.size() * 4ull;
                if (newStart > SR_UINT32_MAX) {
                    return false;
                }
                pool = m_data.substr(m_pool, m_header + m_count * 4ull);
                pool.resize(pool.size() + m_extra.size() * 4ull);
                pool += m_data.ToStringView().substr(m_pool + m_header + m_count * 4ull,
                    m_stringsStart - m_header - m_count * 4ull);
                const uint32_t stringsEnd = m_stylesStart ? m_stylesStart : m_poolSize;
                pool += m_data.ToStringView().substr(m_pool + m_stringsStart, stringsEnd - m_stringsStart);
                for (uint32_t i = 0; i < m_extra.size(); ++i) {
                    Write32(pool, m_header + (m_count + i) * 4ull, static_cast<uint32_t>(pool.size() - newStart));
                    const uint32_t length = static_cast<uint32_t>(m_extra[i].size());
                    if (m_utf8) {
                        for (uint32_t j = 0; j < 2; ++j) {
                            if (length >= 128) {
                                pool += static_cast<char>((length >> 8) | 0x80);
                            }
                            pool += static_cast<char>(length);
                        }
                        pool += m_extra[i];
                        pool += '\0';
                    }
                    else {
                        Append16(pool, static_cast<uint16_t>(length));
                        for (const auto value : m_extra[i]) {
                            Append16(pool, static_cast<uint8_t>(value));
                        }
                        Append16(pool, 0);
                    }
                }
                while (pool.size() % 4 != 0) {
                    pool += '\0';
                }
                const uint32_t stylesStart = m_stylesStart ? static_cast<uint32_t>(pool.size()) : 0;
                if (m_stylesStart) {
                    pool += m_data.ToStringView().substr(m_pool + m_stylesStart, m_poolSize - m_stylesStart);
                }
                if (pool.size() > SR_UINT32_MAX) {
                    return false;
                }
                Write32(pool, 4, static_cast<uint32_t>(pool.size()));
                Write32(pool, 8, static_cast<uint32_t>(m_count + m_extra.size()));
                Write32(pool, 16, Read32(m_data, m_pool + 16) & ~1u);
                Write32(pool, 20, static_cast<uint32_t>(newStart));
                Write32(pool, 24, stylesStart);
                return true;
            }

            String m_data;
            Vector<String> m_extra;
            String m_oldPackage;
            SizeType m_pool = 0;
            uint32_t m_poolSize = 0;
            uint32_t m_header = 0;
            uint32_t m_count = 0;
            uint32_t m_styles = 0;
            uint32_t m_stringsStart = 0;
            uint32_t m_stylesStart = 0;
            bool m_utf8 = false;
            bool m_hasPackage = false;
        };
    }

    bool RewriteAndroidManifestPackage(StringView manifest, StringView package, String& output, String& error) {
        error.clear();
        output.clear();
        if (!ValidPackage(package)) {
            error = "Invalid Android package ID. Use at least two dot-separated identifiers starting with letters.";
            return false;
        }
        ManifestEditor editor(manifest);
        if (!editor.Rewrite(package, output)) {
            output.clear();
            error = "Unsupported or malformed binary Android manifest. Package ID was not changed.";
            return false;
        }
        return true;
    }
}
