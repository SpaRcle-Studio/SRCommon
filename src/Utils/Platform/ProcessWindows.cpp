//
// Created by OpenAI Codex on 12.11.2026.
//

#include <Utils/Platform/ProcessImpl.h>

#ifdef SR_WIN32
    #include <Windows.h>
    #include <shellapi.h>

namespace SR_UTILS_NS::Platform {
    namespace {
        Vector<wchar_t> ToWide(StringView text) {
            const int32_t count = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int32_t>(text.size()), nullptr, 0);
            Vector<wchar_t> result(count + 1);
            MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int32_t>(text.size()), result.data(), count);
            result[count] = 0;
            return result;
        }

        String QuoteArgument(StringView value) {
            String result = "\"";
            uint32_t slashes = 0;
            for (char character : value) {
                if (character == '\\') {
                    ++slashes;
                    continue;
                }
                const uint32_t count = slashes * (character == '"' ? 2 : 1);
                for (uint32_t i = 0; i < count; ++i) {
                    result += '\\';
                }
                slashes = 0;
                if (character == '"') {
                    result += '\\';
                }
                result += character;
            }
            for (uint32_t i = 0; i < slashes * 2; ++i) {
                result += '\\';
            }
            result += '"';
            return result;
        }

        void CloseProcessHandle(HANDLE& handle) {
            if (handle) {
                CloseHandle(handle);
                handle = nullptr;
            }
        }
    }

    class WindowsProcess : public ProcessImpl {
    public:
        ~WindowsProcess() override {
            Stop();
            ClosePipes();
        }

        bool Start(const ProcessInfo& info) override {
            ClosePipes();
            SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
            HANDLE inputRead = nullptr;
            HANDLE inputWrite = nullptr;
            HANDLE outputWrite = nullptr;
            HANDLE errorWrite = nullptr;
            const bool pipesCreated = CreatePipe(&inputRead, &inputWrite, &security, 0) &&
                CreatePipe(&m_output, &outputWrite, &security, 0) &&
                CreatePipe(&m_errors, &errorWrite, &security, 0);
            if (!pipesCreated) {
                CloseProcessHandle(inputRead);
                CloseProcessHandle(inputWrite);
                CloseProcessHandle(outputWrite);
                CloseProcessHandle(errorWrite);
                ClosePipes();
                return false;
            }
            SetHandleInformation(m_output, HANDLE_FLAG_INHERIT, 0);
            SetHandleInformation(m_errors, HANDLE_FLAG_INHERIT, 0);
            SetHandleInformation(inputWrite, HANDLE_FLAG_INHERIT, 0);

            String command = QuoteArgument(info.executable.ToStringView());
            for (const auto& argument : info.arguments) {
                command += " " + QuoteArgument(argument);
            }
            auto commandBuffer = ToWide(command);
            Vector<Vector<wchar_t>> settings;
            auto inherited = GetEnvironmentStringsW();
            if (inherited) {
                for (const wchar_t* item = inherited; *item; item += wcslen(item) + 1) {
                    bool replaced = false;
                    for (const auto& setting : info.environment) {
                        const auto key = ToWide(setting.substr(0, setting.find('=')) + "=");
                        if (_wcsnicmp(item, key.data(), key.size() - 1) == 0) {
                            replaced = true;
                            break;
                        }
                    }
                    if (!replaced) {
                        Vector<wchar_t> value;
                        for (SizeType i = 0; i <= wcslen(item); ++i) {
                            value.emplace_back(item[i]);
                        }
                        settings.emplace_back(std::move(value));
                    }
                }
                FreeEnvironmentStringsW(inherited);
            }
            for (const auto& setting : info.environment) {
                settings.emplace_back(ToWide(setting));
            }
            std::sort(settings.begin(), settings.end(), [](const auto& left, const auto& right) {
                return _wcsicmp(left.data(), right.data()) < 0;
            });
            Vector<wchar_t> environment;
            for (const auto& setting : settings) {
                for (wchar_t character : setting) {
                    environment.emplace_back(character);
                }
            }
            environment.emplace_back(0);
            if (environment.size() == 1) {
                environment.emplace_back(0);
            }

            STARTUPINFOEXW startup{};
            startup.StartupInfo.cb = sizeof(startup);
            startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
            startup.StartupInfo.hStdInput = inputRead;
            startup.StartupInfo.hStdOutput = outputWrite;
            startup.StartupInfo.hStdError = errorWrite;
            SIZE_T attributesSize = 0;
            InitializeProcThreadAttributeList(nullptr, 1, 0, &attributesSize);
            Vector<uint8_t> attributes(attributesSize);
            startup.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributes.data());
            const bool initialized = InitializeProcThreadAttributeList(startup.lpAttributeList, 1, 0, &attributesSize) != FALSE;
            HANDLE inheritedHandles[] = {inputRead, outputWrite, errorWrite};
            const bool configured = initialized && UpdateProcThreadAttribute(startup.lpAttributeList, 0,
                PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inheritedHandles, sizeof(inheritedHandles), nullptr, nullptr);

            bool jobReady = true;
            if (info.ownProcessTree) {
                m_job = CreateJobObjectW(nullptr, nullptr);
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
                limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                jobReady = m_job && SetInformationJobObject(m_job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
            }
            const auto directory = ToWide(info.workingDirectory.ToStringView());
            PROCESS_INFORMATION process{};
            const bool started = configured && jobReady && CreateProcessW(nullptr, commandBuffer.data(), nullptr, nullptr, TRUE,
                CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT | CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT,
                environment.data(), info.workingDirectory.empty() ? nullptr : directory.data(), &startup.StartupInfo, &process);
            if (initialized) {
                DeleteProcThreadAttributeList(startup.lpAttributeList);
            }
            CloseProcessHandle(inputRead);
            CloseProcessHandle(outputWrite);
            CloseProcessHandle(errorWrite);
            if (!started) {
                CloseProcessHandle(inputWrite);
                CloseProcessHandle(m_job);
                ClosePipes();
                return false;
            }
            if (m_job && !AssignProcessToJobObject(m_job, process.hProcess)) {
                TerminateProcess(process.hProcess, 1);
                CloseHandle(process.hProcess);
                CloseHandle(process.hThread);
                CloseProcessHandle(inputWrite);
                CloseProcessHandle(m_job);
                ClosePipes();
                return false;
            }
            ResumeThread(process.hThread);
            CloseHandle(process.hThread);
            m_process = process.hProcess;
            m_exitCode = -1;
            if (!info.input.empty()) {
                DWORD written = 0;
                WriteFile(inputWrite, info.input.data(), static_cast<DWORD>(info.input.size()), &written, nullptr);
            }
            CloseProcessHandle(inputWrite);
            return true;
        }

        bool IsRunning() override {
            if (!m_process) {
                return false;
            }
            if (WaitForSingleObject(m_process, 0) == WAIT_TIMEOUT) {
                return true;
            }
            DWORD code = 1;
            GetExitCodeProcess(m_process, &code);
            m_exitCode = static_cast<int32_t>(code);
            CloseProcessHandle(m_process);
            return false;
        }

        int32_t GetExitCode() const override {
            return m_exitCode;
        }

        int64_t ReadOutput(char* buffer, uint32_t size, bool errorStream) override {
            HANDLE pipe = errorStream ? m_errors : m_output;
            DWORD available = 0;
            if (!pipe || !PeekNamedPipe(pipe, nullptr, 0, nullptr, &available, nullptr)) {
                return -1;
            }
            if (!available) {
                return 0;
            }
            DWORD received = 0;
            if (!ReadFile(pipe, buffer, SR_MIN(size, available), &received, nullptr)) {
                return -1;
            }
            return received;
        }

        void Stop() override {
            if (m_job) {
                TerminateJobObject(m_job, 1);
            }
            if (m_process && IsRunning()) {
                TerminateProcess(m_process, 1);
                WaitForSingleObject(m_process, 3000);
                IsRunning();
            }
            CloseProcessHandle(m_job);
        }

    private:
        void ClosePipes() {
            CloseProcessHandle(m_output);
            CloseProcessHandle(m_errors);
        }

        HANDLE m_process = nullptr;
        HANDLE m_job = nullptr;
        HANDLE m_output = nullptr;
        HANDLE m_errors = nullptr;
        int32_t m_exitCode = -1;
    };

    RawPointerHolder<ProcessImpl> CreateProcessImpl() {
        return new WindowsProcess();
    }

    bool OpenAssociated(StringView target) {
        const auto wide = ToWide(target);
        return reinterpret_cast<intptr_t>(ShellExecuteW(nullptr, L"open", wide.data(), nullptr, nullptr, SW_SHOWNORMAL)) > 32;
    }
}
#endif
