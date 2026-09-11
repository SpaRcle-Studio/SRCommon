//
// Created by innerviewer on 2025-04-16.
//

#include <Utils/Common/CLIManager.h>
#include <Utils/Common/StringFormat.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Platform/Platform.h>

#include <Enum/CLIFlags.hpp>
#include <Enum/CLIOptions.hpp>

namespace SR_UTILS_NS {
    void CLIManager::Init(int argc, char** argv) {
        SR_TRACY_ZONE;

        std::vector<std::string> args;
        args.reserve(static_cast<size_t>(argc > 1 ? argc - 1 : 0));

        /// argv[0] - путь до исполняемого файла, аргументом он не является.
        for (int i = 1; i < argc; ++i) {
            args.emplace_back(argv[i]);
        }

    #ifdef SR_EMSCRIPTEN
        /*
         * В браузере нет командной строки, её роль играет query-строка адреса:
         * `index.html?headless&run-scene=Scenes/Test.scene`.
         * Нативные аргументы при этом сохраняются - main() может передать их,
         * например, из --proxy-to-worker или из тестового окружения.
         */
        auto&& webArgs = SR_PLATFORM_NS::GetWebCommandLineArgs();
        args.insert(args.end(), webArgs.begin(), webArgs.end());
    #endif

        Init(args);
    }

    void CLIManager::Init(const std::vector<std::string>& args) {
        SR_TRACY_ZONE;

        auto&& rawOptions = SR_UTILS_NS::EnumReflector::GetNames<CLIOptions>();
        auto&& rawFlags = SR_UTILS_NS::EnumReflector::GetNames<CLIFlags>();

        Map<String, CLIOptions> options;
        Map<String, CLIFlags> flags;

        for (auto&& option : rawOptions) {
            options[std::string("--" + SR_UTILS_NS::StringUtils::ToKebabCase(option))] = SR_UTILS_NS::EnumReflector::FromString<CLIOptions>(option);
        }

        for (auto&& flag : rawFlags) {
            flags[std::string("--" + SR_UTILS_NS::StringUtils::ToKebabCase(flag))] = SR_UTILS_NS::EnumReflector::FromString<CLIFlags>(flag);
        }

        String availableOptions = "CLIManager::Init() : available options:\n";
        for (auto&& [flag, value] : flags) {
            availableOptions += SR_FORMAT("\t{}\n", flag);
        }
        for (auto&& [option, value] : options) {
            availableOptions += SR_FORMAT("\t{} <value>\n", option);
        }
        SR_PLATFORM_NS::WriteConsoleLog(availableOptions);

        for (size_t i = 0; i < args.size(); ++i) {
            const std::string& arg = args[i];

            const bool hasValue = i + 1 < args.size() && args[i + 1].rfind("--", 0) != 0;

            if (flags.count(arg) > 0) {
                // Check if a value follows the flag
                if (hasValue) {
                    SR_PLATFORM_NS::WriteConsoleWarn(SR_FORMAT("CLIManager::Init() : unexpected value for flag '{}': '{}'"
                        "\n\tFlags do not take values!\n", arg, args[i + 1])
                    );

                    ++i; // Skip the unexpected value
                } else {
                    m_flags |= flags.find(arg)->second;
                }
            }
            else if (options.count(arg) > 0) {
                if (hasValue) {
                    m_options[options.find(arg)->second] = args[i + 1];
                    ++i; // Skip the value
                } else {
                    SR_PLATFORM_NS::WriteConsoleWarn(SR_FORMAT("CLIManager::Init() : missing value for option "
                        "'{}'\n", arg)
                    );
                }
            }
            else if (arg.ends_with(".srproject")) {
                m_projectPath = SR_UTILS_NS::Path(arg);
                if (!SR_PLATFORM_NS::IsFileExists(m_projectPath->ToStringView())) {
                    SR_PLATFORM_NS::WriteConsoleWarn(SR_FORMAT("CLIManager::Init() : project path is not a file: '{}'\n", *m_projectPath));
                    m_projectPath = std::nullopt;
                }
                else {
                    SR_PLATFORM_NS::WriteConsoleLog(SR_FORMAT("CLIManager::Init() : loading project from path: '{}'\n", *m_projectPath));
                }
            }
            else if (arg.rfind("-", 0) == 0) {
                SR_PLATFORM_NS::WriteConsoleWarn(SR_FORMAT("CLIManager::Init() : unknown argument: '{}'\n", arg));
            }
            else {
                SR_PLATFORM_NS::WriteConsoleWarn(SR_FORMAT("CLIManager::Init() : unexpected argument: '{}'\n", arg));
            }
        }

        if (IsHeadlessMode()) {
            SR_PLATFORM_NS::WriteConsoleLog("CLIManager::Init() : running in headless mode.\n");
        }
    }

    std::optional<String> CLIManager::GetOptionValue(CLIOptions option) {
        auto&& it = m_options.find(option);
        if (it != m_options.end()) {
            return it->second;
        }

        return std::nullopt;
    }

    bool CLIManager::IsFlagPresent(CLIFlags flag) const {
        return SR_MATH_NS::IsMaskIncludedSubMask(m_flags, flag);
    }

    bool CLIManager::IsHeadlessMode() const {
        return IsFlagPresent(CLIFlags::Headless);
    }
}
