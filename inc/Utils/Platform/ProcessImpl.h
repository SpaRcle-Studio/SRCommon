//
// Created by OpenAI Codex on 12.11.2026.
//

#ifndef SR_UTILS_PROCESS_IMPL_H
#define SR_UTILS_PROCESS_IMPL_H

#include <Utils/Platform/Process.h>

namespace SR_UTILS_NS::Platform {
    class ProcessImpl : public NonCopyable {
    public:
        virtual ~ProcessImpl() = default;
        virtual bool Start(const ProcessInfo& info) = 0;
        virtual bool IsRunning() = 0;
        virtual int32_t GetExitCode() const = 0;
        virtual int64_t ReadOutput(char* buffer, uint32_t size, bool errorStream) = 0;
        virtual void Stop() = 0;
    };

    RawPointerHolder<ProcessImpl> CreateProcessImpl();
}

#endif
