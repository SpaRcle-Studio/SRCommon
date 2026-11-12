#include <Utils/Network/Download.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/Platform/Process.h>

#ifdef SR_LINUX
namespace SR_NETWORK_NS {
    bool DownloadInternal(const DownloadRequest& request, const SR_UTILS_NS::Path& temporary) {
        SR_PLATFORM_NS::Process process;
        SR_PLATFORM_NS::ProcessInfo info;
        info.executable = "curl";
        info.arguments = {"--fail", "--location", "--silent", "--show-error", "--proto", "=https", "--proto-redir", "=https",
            "--retry", "2", "--connect-timeout", "30", "--max-time", "1800", request.url};
        info.output = temporary;
        info.errorOutput = temporary.ConcatExt("log");
        if (!process.Start(std::move(info))) {
            return false;
        }
        while (process.IsRunning()) {
            if (request.isCancelled && request.isCancelled()) {
                process.Stop();
                return false;
            }
            if (request.progress || request.transferProgress) {
                auto file = SR_UTILS_NS::VFS::Instance().OpenFile(temporary, SR_UTILS_NS::FileMode::Read);
                if (file) {
                    if (request.progress) {
                        request.progress(file.GetSize());
                    }
                    if (request.transferProgress) {
                        request.transferProgress(file.GetSize(), 0);
                    }
                }
            }
            SR_HTYPES_NS::Thread::Sleep(100);
        }
        process.FinishOutput();
        return process.GetExitCode() == 0 && !process.HasOutputError();
    }
}
#endif
