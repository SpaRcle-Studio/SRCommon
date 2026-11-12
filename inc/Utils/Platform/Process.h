#ifndef SR_UTILS_PROCESS_H
#define SR_UTILS_PROCESS_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Types/RawPointerHolder.h>
#include <Utils/Types/Thread.h>
#include <Utils/Types/Function.h>

namespace SR_UTILS_NS::Platform {
    struct ProcessInfo {
        Path executable;
        Vector<String> arguments;
        Vector<String> environment;
        Path workingDirectory;
        Path output;
        Path errorOutput;
        String input;
        bool ownProcessTree = false;
        bool backgroundOutput = true;
    };

    class ProcessImpl;

    class SR_COMMON_DLL_API Process : public NonCopyable {
    public:
        Process();
        ~Process() override;

        bool Start(ProcessInfo info);
        bool IsRunning();
        int32_t GetExitCode() const;
        bool HasOutputError() const;
        void Stop();
        void FinishOutput();

    private:
        RawPointerHolder<ProcessImpl> m_impl;
        SR_HTYPES_NS::Thread::Ptr m_outputThread = nullptr;
        SR_HTYPES_NS::Function<bool()> m_pumpOutput;
        std::atomic<bool> m_stopOutput = false;
        std::atomic<bool> m_outputError = false;
    };

    SR_COMMON_DLL_API String GetExecutableName(StringView name);
    SR_COMMON_DLL_API String GetDesktopPlatformName();
    SR_COMMON_DLL_API String GetProcessArchitecture();
    SR_COMMON_DLL_API bool OpenAssociated(StringView target);
}

#endif
