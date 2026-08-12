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

    class Debug : public Singleton<Debug>, public SRClass {
        SR_REGISTER_SINGLETON(Debug);
        SR_CLASS()
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
        /// @method
        void Print(StringView msg, DebugLogType type);
        /// @method
        void Log(StringView msg);
        /// @method
        void Info(StringView msg);
        /// @method
        void Warn(StringView msg);
        /// @method
        void Error(StringView msg);

        void Success(StringView msg);
        void VulkanLog(StringView msg);
        void Graph(StringView msg);
        void Vulkan(StringView msg);
        void Shader(StringView msg);
        void Script(StringView msg);
        void System(StringView msg);
        void VulkanError(StringView msg);
        bool Assert(StringView msg);
        bool AssertOnceCheck(StringView msg);
        void ScriptLog(StringView msg);
        void ScriptError(StringView msg);

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
