//
// Created by Nikita on 16.11.2020.
//

#include <Utils/Debug.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/Platform/Stacktrace.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Common/Breakpoint.h>

#ifdef SR_COMMON_GIT_METADATA
    #include <git.h>
#endif

#include <Codegen/Debug.generated.hpp>

namespace SR_UTILS_NS {
    void Debug::Print(StringView rawMsg, DebugLogType type) {
        SR_LOCK_GUARD;
        SR_TRACY_ZONE;

        if (!m_isInit) {
            SR_PLATFORM_NS::WriteConsoleError("Debug::Print() : debugger isn't initialized!\n\tMessage: {}{}{}"_format(rawMsg, "\n", SR_UTILS_NS::GetStacktrace()));
            Breakpoint();
            return;
        }

        switch (type) {
            case DebugLogType::Warn:
                ++m_countWarnings;
                break;
            case DebugLogType::Error:
            case DebugLogType::Assert:
            case DebugLogType::VulkanError:
                ++m_countErrors;
                break;
            default:
                break;
        }

        String msg = rawMsg;

    #ifndef SR_EMSCRIPTEN
        if (type == DebugLogType::Assert) {
            msg.append("\nStack trace:\n").append(GetStacktrace());
        }
    #endif

        msg.append("\n");

        std::string threadName = SR_UTILS_NS::GetThisThreadId();

        auto&& prefix = SR_FORMAT("[{}] [{}]", SR_UTILS_NS::EnumReflector::ToStringAtom(type).ToCStr(), threadName);
        auto&& memoryUsage = m_showUseMemory ? SR_FORMAT("<{} MB> ", static_cast<uint32_t>(SR_PLATFORM_NS::GetProcessUsedMemory() / 1024 / 1024)) : std::string();

        {
            if (SR_PLATFORM_NS::GetType() == PlatformType::Android) {
                std::lock_guard lock(SR_PLATFORM_NS::g_platformLogMutex);
                try {
                    if (IsErrorLogType(type)) {
                        SR_PLATFORM_NS::WriteConsoleError(fmt::vformat("{} {}",fmt::make_format_args(prefix, msg)));
                    }
                    else if (IsWarningLogType(type)) {
                        SR_PLATFORM_NS::WriteConsoleWarn(fmt::vformat("{} {}",fmt::make_format_args(prefix, msg)));
                    }
                    else {
                        SR_PLATFORM_NS::WriteConsoleLog(fmt::vformat("{} {}",fmt::make_format_args(prefix, msg)));
                    }
                }
                catch (const std::exception& ex) {
                    std::cout << " Error while printing message: " << ex.what() << "\nMessage: " << msg << std::endl;
                }
            }
            else {
                fmt::print(fmt::fg(fmt::color::dark_gray) | fmt::emphasis::faint, fmt::runtime(memoryUsage));
                fmt::print(GetTextStyleColorByLogType(type), fmt::runtime(prefix));

                std::lock_guard lock(SR_PLATFORM_NS::g_platformLogMutex);
                try {
                    fmt::print(fmt::emphasis::bold, " {}", msg);
                }
                catch (const std::exception& ex) {
                    std::cout << " Error while printing message: " << ex.what() << "\nMessage: " << msg << std::endl;
                }
            }


            std::cout << std::flush;

            if (m_file.is_open()) {
                m_file << (memoryUsage + prefix + " " + msg) << std::flush;
            }
        }

        volatile static bool enableBreakPoints = true;
        if (type == DebugLogType::Assert && Platform::IsRunningUnderDebugger() && enableBreakPoints) {
            Breakpoint();
        }
    }

    void Debug::Initialize(const Path& logPath, bool ShowUsedMemory, Theme colorTheme) {
        m_theme = colorTheme;

        InitColorTheme();

    #if !defined(SR_ANDROID) && !defined(SR_EMSCRIPTEN)
        auto&& successfulPath = SR_PLATFORM_NS::GetApplicationPath().GetFolder().Concat("/successful");
        if (successfulPath.Exists(Path::Type::File))
            Platform::Delete(successfulPath);
    #endif

        m_isInit = true;
        m_showUseMemory = ShowUsedMemory;

    #ifndef SR_EMSCRIPTEN
        m_logPath = logPath;

        if (!m_logPath.GetFolder().CreateIfNotExists()) {
            SR_PLATFORM_NS::WriteConsoleError("Failed to create log folder!\n\tLog path: " + m_logPath.ToString());
        }

        if (m_logPath.Exists(Path::Type::File)) {
            Platform::Delete(m_logPath);
        }

        m_file.open(m_logPath.c_str());
        if (!m_file.is_open()) {
            SR_PLATFORM_NS::WriteConsoleError("Debug::Init() : failed to open log file!\n\tLog path: " + m_logPath.ToString());
        }

        Print("Debugger has been initialized. \n\tLog path: {}"_format(m_logPath), DebugLogType::Debug);
    #endif

    #ifdef SR_COMMON_GIT_METADATA
        std::string gitMetadata;
        std::string dirty = git_AnyUncommittedChanges() ? "true" : "false";

        std::time_t timestamp = std::stoll(git_CommitDate());
        std::tm* timeUTC = std::gmtime(&timestamp);

        gitMetadata += "Build Info: '" + std::string(git_CommitSHA1()).substr(0, 7) + "' in '" + git_Branch() + "' (dirty: " + dirty + ") "
            "by '" +
            + git_AuthorName() + "' on\n\t" + std::asctime(timeUTC);

        Print(gitMetadata, DebugLogType::Info);
    #endif
    }

    void Debug::Log(StringView msg) { Print(msg, DebugLogType::Log); }
    void Debug::Success(StringView msg) { Print(msg, DebugLogType::Success); }
    void Debug::VulkanLog(StringView msg) { Print(msg, DebugLogType::VulkanLog); }
    void Debug::Info(StringView msg) { Print(msg, DebugLogType::Info); }
    void Debug::Graph(StringView msg) { Print(msg, DebugLogType::Graph); }
    void Debug::Vulkan(StringView msg) { Print(msg, DebugLogType::Vulkan); }
    void Debug::Shader(StringView msg) { Print(msg, DebugLogType::Shader); }
    void Debug::Script(StringView msg) { Print(msg, DebugLogType::Script); }
    void Debug::System(StringView msg) { Print(msg, DebugLogType::System); }
    void Debug::Warn(StringView msg) { Print(msg, DebugLogType::Warn);}
    void Debug::Error(StringView msg) { Print(msg, DebugLogType::Error); }
    void Debug::VulkanError(StringView msg) { Print(msg, DebugLogType::VulkanError); }
    bool Debug::Assert(StringView msg) { Print(msg, DebugLogType::Assert); return false; }
    void Debug::ScriptLog(StringView msg) { Print(msg, DebugLogType::ScriptLog); }
    void Debug::ScriptError(StringView msg) { Print(msg, DebugLogType::ScriptError); }

    void Debug::DeInitialize() {
        SR_LOCK_GUARD;

        if (!m_isInit) {
            SR_PLATFORM_NS::WriteConsoleError("Debug::DeInitialize() : debugger isn't initialized!\n");
            return;
        }

        if (m_countErrors == 0 && m_countWarnings == 0) {
            std::string msg = "Debugger has been stopped with no errors and warnings!";
            Print(msg, DebugLogType::Debug);
        }
        else {
            std::string msg = "Debugger has been stopped with errors!\n"
                              "\tErrors count: "+std::to_string(m_countErrors)+
                              "\n\tWarnings count: "+std::to_string(m_countWarnings);
            Print(msg, DebugLogType::Debug);
        }

        if (m_file.is_open()) {
            m_file.close();
        }

    #ifndef SR_ANDROID
        auto&& path = Platform::GetApplicationPath().GetFolder().Concat("/successful");
        std::ofstream success(path.c_str());
        if (success.is_open()) {
            success.close();
        }
    #endif

        m_isInit = false;
    }

    void Debug::InitColorTheme() {
    #ifdef SR_WIN32
        if (!m_ColorThemeIsEnabled) {
            if (m_theme == Theme::Light)
                system("color 70");
        }
    #endif
        m_ColorThemeIsEnabled = true;
    }

    bool Debug::AssertOnceCheck(StringView msg) {
        SR_TRACY_ZONE;
        SR_SCOPED_LOCK;

        String message(msg);
        static SR_HTYPES_NS::FlatHashSet<String> asserts;

        if (asserts.count(message) == 0) {
            asserts.insert(message);
            return false;
        }

        return true;
    }

    void Debug::MakeCrash() {
        SR_SCOPED_LOCK;

        Assert("[Stacktrace]");
        System("Function \"MakeCrash\" has been called... >_<");
        for (long long int i = 0; ++i; (&i)[i] = i);
        // https://codengineering.ru/q/what-is-the-easiest-way-to-make-a-c-program-crash-24928
    }

    void Debug::TestPrint() {
        for (auto&& logTypeName : SR_UTILS_NS::EnumReflector::GetNames<DebugLogType>()) {
            Print("The quick brown fox jumps over the lazy dog.", SR_UTILS_NS::EnumReflector::FromString<DebugLogType>(logTypeName));
        }
    }

    void Debug::SetLevel(Debug::Level level) {
        m_level = level;
    }

    Debug::Level Debug::GetLevel() {
        return m_level;
    }

    bool Debug::IsInitialized() const {
        return m_isInit;
    }

    Debug::Debug() = default;
    Debug::~Debug() = default;

    bool IsErrorLogType(DebugLogType type) {
        return type == DebugLogType::Error || type == DebugLogType::ScriptError || type == DebugLogType::Assert
            || type == DebugLogType::VulkanError;
    }

    bool IsWarningLogType(DebugLogType type) {
        return type == DebugLogType::Warn;
    }

    fmt::text_style GetTextStyleColorByLogType(DebugLogType type) {
        static auto errorWarnStyle = fmt::emphasis::blink | fmt::emphasis::bold;
        switch (type) {
            case DebugLogType::Log: return fmt::fg(fmt::color::dark_cyan);
            case DebugLogType::Info: return fmt::fg(fmt::color::dark_magenta);
            case DebugLogType::Debug: return fmt::fg(fmt::color::blue_violet);
            case DebugLogType::Graph: return fmt::fg(fmt::color::green);
            case DebugLogType::Shader: return fmt::fg(fmt::color::lawn_green);
            case DebugLogType::Script: return fmt::fg(fmt::color::brown);
            case DebugLogType::System: return fmt::fg(fmt::color::sky_blue);
            case DebugLogType::Warn: return fmt::fg(fmt::color::yellow) | errorWarnStyle;
            case DebugLogType::Error: return fmt::fg(fmt::color::red) | errorWarnStyle;
            case DebugLogType::ScriptError: return fmt::fg(fmt::color::red) | errorWarnStyle;
            case DebugLogType::ScriptLog: return fmt::fg(fmt::color::peach_puff);
            case DebugLogType::Vulkan: return fmt::fg(fmt::color::deep_sky_blue);
            case DebugLogType::VulkanLog: return fmt::fg(fmt::color::deep_sky_blue);
            case DebugLogType::VulkanError: return fmt::fg(fmt::color::red) | errorWarnStyle;
            case DebugLogType::Assert: return fmt::fg(fmt::color::orange_red) | errorWarnStyle;
            case DebugLogType::Success: return fmt::fg(fmt::color::lime_green);
            case DebugLogType::Test: return fmt::fg(fmt::color::lime_green);
            default:
                return fmt::text_style(); /// NOLINT
        }
    }
}
