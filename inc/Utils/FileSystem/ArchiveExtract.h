#ifndef SR_UTILS_ARCHIVE_EXTRACT_H
#define SR_UTILS_ARCHIVE_EXTRACT_H

#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS {
    struct ArchiveExtractOptions {
        String stripPrefix;
        bool stripRoot = false;
    };

    SR_COMMON_DLL_API bool ExtractZip(const Path& source, const Path& destination, const ArchiveExtractOptions& options = {});
    SR_COMMON_DLL_API bool ExtractTarGzip(const Path& source, const Path& destination, const ArchiveExtractOptions& options = {});
}

#endif
