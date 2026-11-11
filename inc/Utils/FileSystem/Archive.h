//
// Created by Monika on 14.09.2026.
//

#ifndef SR_ENGINE_COMMON_ARCHIVE_H
#define SR_ENGINE_COMMON_ARCHIVE_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/Types/RawPointerHolder.h>
#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS {
    class File;

    enum class ArchiveType {
        Unknown,
        Zip,
        SevenZip
    };

    class Archive : public NonCopyable {
    public:
        struct Entry {
            StringView name;
            StringView data;
            bool isDirectory = false;
            bool isExecutable = false;
        };
        using EnumerateCallback = SR_HTYPES_NS::Function<void(const Entry&)>;
    public:
        Archive() = default;

    public:
        RawPointerHolder<Archive> static Load(StringView data);
        RawPointerHolder<Archive> static Load(File file);
        static bool SaveZip(const Path& path, const Vector<Entry>& entries);

    public:
        void Enumerate(const EnumerateCallback& callback) const;

    private:
        bool UnpackZip();

    private:
        StringView m_archiveData;
        String m_unpackedData;
        Vector<Entry> m_entries;

    };
}

#endif //SR_ENGINE_COMMON_ARCHIVE_H
