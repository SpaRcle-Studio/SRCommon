//
// Created by OpenAI Codex on 12.11.2026.
//

#include <Utils/Platform/ProcessImpl.h>
#include <Utils/Platform/Platform.h>
#include <Utils/FileSystem/VFS.h>

namespace SR_UTILS_NS::Platform {
    Process::Process()
        : m_impl(CreateProcessImpl())
    { }

    Process::~Process() {
        Stop();
    }

    bool Process::Start(ProcessInfo info) {
        Stop();
        if (!m_impl || info.input.size() > 4096) {
            return false;
        }

        auto&& vfs = VFS::Instance();
        if (info.executable.ToStringView().contains("/")) {
            if (!vfs.ResolvePath(info.executable, FileMode::Read)) {
                return false;
            }
        }
        if (!info.workingDirectory.empty() && !vfs.ResolvePath(info.workingDirectory, FileMode::Read)) {
            return false;
        }
        if (!vfs.CreateDirectories(info.output.GetPrevious().ToStringView())) {
            return false;
        }

        auto output = vfs.OpenFile(info.output, FileMode::Write);
        if (!output) {
            return false;
        }
        auto errors = output;
        if (!info.errorOutput.empty() && info.errorOutput != info.output) {
            if (!vfs.CreateDirectories(info.errorOutput.GetPrevious().ToStringView())) {
                return false;
            }
            errors = vfs.OpenFile(info.errorOutput, FileMode::Write);
            if (!errors) {
                return false;
            }
        }
        if (!m_impl->Start(info)) {
            return false;
        }

        m_stopOutput = false;
        m_outputError = false;
        m_pumpOutput = [this, output = std::move(output), errors = std::move(errors), closed = uint8_t(0)]() mutable {
            char buffer[16384];
            for (uint32_t stream = 0; stream < 2; ++stream) {
                const uint8_t mask = 1 << stream;
                if (closed & mask) {
                    continue;
                }
                const int64_t count = m_impl->ReadOutput(buffer, sizeof(buffer), stream == 1);
                if (count < 0 || (count == 0 && m_stopOutput)) {
                    closed |= mask;
                }
                else if (count > 0) {
                    auto&& file = stream == 0 ? output : errors;
                    if (file.Write(buffer, count) != static_cast<uint64_t>(count)) {
                        m_outputError = true;
                        closed |= mask;
                    }
                    file.Flush();
                }
            }
            if (closed != 3) {
                return false;
            }
            output = File();
            errors = File();
            return true;
        };
        if (!info.backgroundOutput) {
            return true;
        }
        return SR_HTYPES_NS::Thread::Factory::Instance().Create(m_outputThread, [this]() {
            while (!m_pumpOutput()) {
                SR_HTYPES_NS::Thread::Sleep(20);
            }
            return false;
        });
    }

    bool Process::IsRunning() {
        if (!m_outputThread && m_pumpOutput) {
            m_pumpOutput();
        }
        return m_impl && m_impl->IsRunning();
    }

    int32_t Process::GetExitCode() const {
        return m_impl ? m_impl->GetExitCode() : -1;
    }

    bool Process::HasOutputError() const {
        return m_outputError;
    }

    void Process::FinishOutput() {
        m_stopOutput = true;
        if (m_outputThread) {
            m_outputThread->Join();
            m_outputThread->Free();
            m_outputThread = nullptr;
        }
        else if (m_pumpOutput) {
            while (!m_pumpOutput()) {
                SR_HTYPES_NS::Thread::Sleep(20);
            }
        }
        m_pumpOutput = nullptr;
    }

    void Process::Stop() {
        if (m_impl) {
            m_impl->Stop();
        }
        FinishOutput();
    }

    String GetExecutableName(StringView name) {
        String result(name);
        if (GetType() == PlatformType::Windows) {
            result += ".exe";
        }
        return result;
    }

    String GetDesktopPlatformName() {
        if (GetType() == PlatformType::Windows) {
            return "Windows";
        }
        if (GetType() == PlatformType::Linux) {
            return "Linux";
        }
        return {};
    }

    String GetProcessArchitecture() {
    #if defined(_M_X64) || defined(__x86_64__)
        return "x64";
    #elif defined(_M_ARM64) || defined(__aarch64__)
        return "aarch64";
    #else
        return "unknown";
    #endif
    }

#if !defined(SR_WIN32) && !defined(SR_LINUX)
    RawPointerHolder<ProcessImpl> CreateProcessImpl() {
        return {};
    }

    bool OpenAssociated(StringView target) {
        return false;
    }
#endif
}
