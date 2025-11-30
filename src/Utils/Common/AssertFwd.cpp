//
// Created by Monika on 30.11.2025.
//

#include <Utils/Common/AssertFwd.h>
#include <Utils/Debug.h>

namespace SR_UTILS_NS::LogDetails {
    bool AssertFwd(const std::string& msg) {
        return SR_UTILS_NS::Debug::Instance().Assert(msg);
    }

    bool AssertOnceCheckFwd(const std::string& msg) {
        return SR_UTILS_NS::Debug::Instance().AssertOnceCheck(msg);
    }

    void LogFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Log);
    }

    void LogTestFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Test);
    }

    void WarnFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Warn);
    }

    void InfoFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Info);
    }

    void ErrorFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Error);
    }

    void GraphLogFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Graph);
    }

    void SuccessFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Success);
    }

    void ShaderFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Shader);
    }

    void ShaderLogFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Shader);
    }

    void SystemLogFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::System);
    }

    void VulkanMsgFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Vulkan);
    }

    void VulkanLogFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::VulkanLog);
    }

    void VulkanErrorFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::VulkanError);
    }

    void DebugLogFwd(const std::string& msg) {
        SR_UTILS_NS::Debug::Instance().Print(msg, DebugLogType::Debug);
    }
}