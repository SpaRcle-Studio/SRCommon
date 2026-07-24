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

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];

            if (flags.count(arg) > 0) {
                // Check if a value follows the flag
                if (i + 1 < argc && std::string(argv[i + 1]).rfind("--", 0) != 0) {
                    SR_PLATFORM_NS::WriteConsoleWarn(SR_FORMAT("CLIManager::Init() : unexpected value for flag '{}': '{}'"
                        "\n\tFlags do not take values!\n", arg, argv[i + 1])
                    );

                    ++i; // Skip the unexpected value
                } else {
                    m_flags |= flags.find(arg)->second;
                }
            }
            else if (options.count(arg) > 0) {
                if (i + 1 < argc && std::string(argv[i + 1]).rfind("--", 0) != 0) {
                    m_options[options.find(arg)->second] = argv[i + 1];
                    ++i; // Skip the value
                } else {
                    SR_PLATFORM_NS::WriteConsoleWarn(SR_FORMAT("CLIManager::Init() : missing value for option "
                        "'{}': '{}'\n", arg, argv[i + 1])
                    );
                }
            }
            else if (arg.ends_with(".srproject")) {
                m_projectPath = SR_UTILS_NS::Path(arg);
                if (!m_projectPath->IsFile()) {
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
