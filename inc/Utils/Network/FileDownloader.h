//
// Created by Monika on 31.03.2025.
//

#ifndef SR_ENGINE_UTILS_NETWORK_FILE_DOWNLOADER_H
#define SR_ENGINE_UTILS_NETWORK_FILE_DOWNLOADER_H

#include <Utils/Common/Singleton.h>

namespace SR_UTILS_NS {
    class Path;
}

namespace SR_NETWORK_NS {
    class FileDownloader : public SR_UTILS_NS::Singleton<FileDownloader> {
        SR_REGISTER_SINGLETON(FileDownloader)
        using Super = SR_UTILS_NS::Singleton<FileDownloader>;
    public:
        SR_NODISCARD bool DownloadFile(const std::string& url, const SR_UTILS_NS::Path& filePath);

    };
}

#endif //SR_ENGINE_UTILS_NETWORK_FILE_DOWNLOADER_H
