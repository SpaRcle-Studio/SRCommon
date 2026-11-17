//
// Created by Monika on 30.11.2025.
//

#include <Utils/Common/AssertFwd.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Debug.h>

namespace SR_UTILS_NS::LogDetails {
    bool AssertFwd(StringView msg) {
        return SR_UTILS_NS::Debug::Instance().Assert(msg);
    }

    void AssertTerminateFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Assert(msg);
        SR_PLATFORM_NS::Terminate();
    }

    bool AssertOnceCheckFwd(StringView msg) {
        return SR_UTILS_NS::Debug::Instance().AssertOnceCheck(msg);
    }

    void LogFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Log);
    }

    void LogTestFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Test);
    }

    void WarnFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Warn);
    }

    void InfoFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Info);
    }

    void ErrorFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Error);
    }

    void GraphLogFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Graph);
    }

    void SuccessFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Success);
    }

    void ShaderFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Shader);
    }

    void ShaderLogFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Shader);
    }

    void SystemLogFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::System);
    }

    void VulkanMsgFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Vulkan);
    }

    void VulkanLogFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::VulkanLog);
    }

    void VulkanErrorFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::VulkanError);
    }

    void DebugLogFwd(StringView msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Debug);
    }
}