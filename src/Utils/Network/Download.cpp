#include <Utils/Network/Download.h>
#include <Utils/FileSystem/VFS.h>

namespace SR_NETWORK_NS {
    bool Download(const DownloadRequest& request) {
        if (!request.url.starts_with("https://")) {
            return false;
        }
        auto&& vfs = SR_UTILS_NS::VFS::Instance();
        if (!vfs.CreateDirectories(request.destination.GetPrevious().ToStringView())) {
            return false;
        }
        const auto temporary = request.destination.ConcatExt("part");
        if (!DownloadInternal(request, temporary)) {
            return false;
        }
        if (request.isCancelled && request.isCancelled()) {
            return false;
        }
        if (!vfs.Copy(temporary.ToStringView(), request.destination.ToStringView())) {
            return false;
        }
        vfs.Delete(temporary.ToStringView());
        return true;
    }

#if !defined(SR_WIN32) && !defined(SR_LINUX)
    bool DownloadInternal(const DownloadRequest& request, const SR_UTILS_NS::Path& temporary) {
        return false;
    }
#endif
}
