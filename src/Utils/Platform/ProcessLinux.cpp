#include <Utils/Platform/ProcessImpl.h>
#include <Utils/Platform/Platform.h>

#ifdef SR_LINUX
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <signal.h>

namespace SR_UTILS_NS::Platform {
    class LinuxProcess : public ProcessImpl {
    public:
        ~LinuxProcess() override {
            Stop();
            ClosePipes();
        }

        bool Start(const ProcessInfo& info) override {
            ClosePipes();
            int32_t input[2] = {-1, -1};
            if (pipe2(input, O_CLOEXEC) != 0) {
                return false;
            }
            if (pipe2(m_output, O_CLOEXEC) != 0 || pipe2(m_errors, O_CLOEXEC) != 0) {
                close(input[0]);
                close(input[1]);
                ClosePipes();
                return false;
            }

            Vector<String> storage = {"env"};
            for (const auto& setting : info.environment) {
                storage.emplace_back(setting);
            }
            storage.emplace_back(info.executable.ToStringView());
            for (const auto& argument : info.arguments) {
                storage.emplace_back(argument);
            }
            Vector<char*> arguments;
            for (auto& argument : storage) {
                arguments.emplace_back(argument.data());
            }
            arguments.emplace_back(nullptr);

            auto argv = arguments.data();
            const char* directory = info.workingDirectory.empty() ? nullptr : info.workingDirectory.c_str();
            if (!info.input.empty()) {
                int64_t count = -1;
                do {
                    count = write(input[1], info.input.data(), info.input.size());
                } while (count < 0 && errno == EINTR);
                if (count != static_cast<int64_t>(info.input.size())) {
                    close(input[0]);
                    close(input[1]);
                    ClosePipes();
                    return false;
                }
            }
            close(input[1]);
            m_pid = fork();
            if (m_pid == 0) {
                setpgid(0, 0);
                if (directory && chdir(directory) != 0) {
                    _exit(126);
                }
                if (dup2(input[0], STDIN_FILENO) < 0 || dup2(m_output[1], STDOUT_FILENO) < 0 ||
                    dup2(m_errors[1], STDERR_FILENO) < 0) {
                    _exit(126);
                }
                execv("/usr/bin/env", argv);
                _exit(127);
            }

            close(input[0]);
            close(m_output[1]);
            close(m_errors[1]);
            m_output[1] = m_errors[1] = -1;
            if (m_pid <= 0) {
                ClosePipes();
                return false;
            }
            setpgid(m_pid, m_pid);
            m_group = m_pid;
            m_ownTree = info.ownProcessTree;
            m_exitCode = -1;
            fcntl(m_output[0], F_SETFL, O_NONBLOCK);
            fcntl(m_errors[0], F_SETFL, O_NONBLOCK);
            return true;
        }

        bool IsRunning() override {
            if (m_pid <= 0) {
                return false;
            }
            int32_t status = 0;
            const auto result = waitpid(m_pid, &status, WNOHANG);
            if (result == 0 || (result < 0 && errno == EINTR)) {
                return true;
            }
            m_exitCode = result > 0 && WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            m_pid = -1;
            return false;
        }

        int32_t GetExitCode() const override {
            return m_exitCode;
        }

        int64_t ReadOutput(char* buffer, uint32_t size, bool errorStream) override {
            const int32_t pipe = errorStream ? m_errors[0] : m_output[0];
            const int64_t count = read(pipe, buffer, size);
            if (count < 0 && (errno == EAGAIN || errno == EINTR)) {
                return 0;
            }
            return count > 0 ? count : -1;
        }

        void Stop() override {
            if (m_ownTree && m_group > 0) {
                kill(-m_group, SIGTERM);
            }
            if (m_pid > 0 && IsRunning()) {
                kill(m_pid, SIGTERM);
                for (uint32_t i = 0; i < 20 && IsRunning(); ++i) {
                    Sleep(50);
                }
                if (m_pid > 0) {
                    kill(m_pid, SIGKILL);
                    while (waitpid(m_pid, nullptr, 0) < 0 && errno == EINTR) {
                    }
                    m_pid = -1;
                }
            }
            if (m_ownTree && m_group > 0) {
                kill(-m_group, SIGKILL);
            }
            m_group = -1;
        }

    private:
        void ClosePipes() {
            for (uint32_t i = 0; i < 2; ++i) {
                if (m_output[i] >= 0) {
                    close(m_output[i]);
                    m_output[i] = -1;
                }
                if (m_errors[i] >= 0) {
                    close(m_errors[i]);
                    m_errors[i] = -1;
                }
            }
        }

        pid_t m_pid = -1;
        pid_t m_group = -1;
        int32_t m_output[2] = {-1, -1};
        int32_t m_errors[2] = {-1, -1};
        int32_t m_exitCode = -1;
        bool m_ownTree = false;
    };

    RawPointerHolder<ProcessImpl> CreateProcessImpl() {
        return new LinuxProcess();
    }

    bool OpenAssociated(StringView target) {
        Process process;
        ProcessInfo info;
        info.executable = "xdg-open";
        info.arguments.emplace_back(target);
        info.output = GetApplicationUserDataDirectory().Concat("open.log");
        if (!process.Start(std::move(info))) {
            return false;
        }
        for (uint32_t i = 0; i < 100 && process.IsRunning(); ++i) {
            Sleep(50);
        }
        return !process.IsRunning() && process.GetExitCode() == 0;
    }
}
#endif
