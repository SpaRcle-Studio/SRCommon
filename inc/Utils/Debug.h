//
// Created by Nikita on 16.11.2020.
//

#ifndef SR_ENGINE_UTILS_DEBUG_H
#define SR_ENGINE_UTILS_DEBUG_H

#include <Utils/FileSystem/Path.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Common/Enumerations.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Common/CastUtils.h>
#include <Utils/Common/AssertFwd.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(DebugLogType, uint8_t,
        Log, Info, Debug, Graph, Shader, Script, System, Warn,
        Error, ScriptError, ScriptLog, Vulkan, VulkanLog, VulkanError, Assert, Success, Test
    );

    static bool IsErrorLogType(DebugLogType type);
    static bool IsWarningLogType(DebugLogType type);

    static fmt::text_style GetTextStyleColorByLogType(DebugLogType type);

    class Debug : public Singleton<Debug> {
        SR_REGISTER_SINGLETON(Debug);
    public:
        enum class Level {
            None = 0, Low = 1, Medium = 2, High = 3, Full = 4
        };

        enum class Theme {
            Dark, Light
        };

    public:
        Debug();
        ~Debug() override;

    private:
        void InitColorTheme();

    public:
        void SetLevel(Level level);

        SR_NODISCARD Level GetLevel();
        SR_NODISCARD bool IsInitialized() const;

        void MakeCrash();
        void TestPrint();

        void Initialize(const Path& logPath, bool ShowUsedMemory, Theme colorTheme = Theme::Light);
        void DeInitialize();

        bool IsSingletonCanBeDestroyed() const override { return false; }

    public:
        void Log(const std::string& msg);
        void Success(const std::string& msg);
        void VulkanLog(const std::string& msg);
        void Info(const std::string& msg);
        void Graph(const std::string& msg);
        void Vulkan(const std::string& msg);
        void Shader(const std::string& msg);
        void Script(const std::string& msg);
        void System(const std::string& msg);
        void Warn(const std::string& msg);
        void Error(const std::string& msg);
        void VulkanError(const std::string& msg);
        bool Assert(const std::string& msg);
        bool AssertOnceCheck(const std::string& msg);

        void ScriptLog(const std::string& msg);
        void ScriptError(const std::string& msg);

        void Print(std::string msg, DebugLogType type);

    private:
        bool m_showUseMemory = false;
        bool m_ColorThemeIsEnabled = false;

        Theme m_theme = Theme::Light;

        std::atomic<bool> m_isInit = false;
        Path m_logPath;
        std::ofstream m_file;
        std::atomic<Level> m_level = Level::Low;
        size_t m_countErrors = 0;
        size_t m_countWarnings = 0;

    };
}

#endif //SR_ENGINE_UTILS_DEBUG_H
