#ifndef SR_UTILS_NETWORK_DOWNLOAD_H
#define SR_UTILS_NETWORK_DOWNLOAD_H

#include <Utils/FileSystem/Path.h>
#include <Utils/Types/Function.h>

namespace SR_NETWORK_NS {
    struct DownloadRequest {
        SR_UTILS_NS::String url;
        SR_UTILS_NS::Path destination;
        SR_HTYPES_NS::Function<bool()> isCancelled;
        SR_HTYPES_NS::Function<void(uint64_t)> progress;
        SR_HTYPES_NS::Function<void(uint64_t, uint64_t)> transferProgress;
    };

    SR_COMMON_DLL_API bool Download(const DownloadRequest& request);
    bool DownloadInternal(const DownloadRequest& request, const SR_UTILS_NS::Path& temporary);
}

#endif
