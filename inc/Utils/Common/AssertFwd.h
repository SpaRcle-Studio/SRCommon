//
// Created by Monika on 30.11.2025.
//

#if !defined(SR_ENGINE_UTILS_ASSERT_FWD_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_ENGINE_UTILS_ASSERT_FWD_H

#include <Utils/Common/StringFormat.h>

namespace SR_UTILS_NS::LogDetails {
    SR_NORETURN void AssertTerminateFwd(const std::string& msg);
    bool AssertFwd(const std::string& msg);
    bool AssertOnceCheckFwd(const std::string& msg);
    void LogFwd(const std::string& msg);
    void LogTestFwd(const std::string& msg);
    void WarnFwd(const std::string& msg);
    void InfoFwd(const std::string& msg);
    void ErrorFwd(const std::string& msg);
    void GraphLogFwd(const std::string& msg);
    void SuccessFwd(const std::string& msg);
    void ShaderFwd(const std::string& msg);
    void ShaderLogFwd(const std::string& msg);
    void SystemLogFwd(const std::string& msg);
    void VulkanMsgFwd(const std::string& msg);
    void VulkanLogFwd(const std::string& msg);
    void VulkanErrorFwd(const std::string& msg);
    void DebugLogFwd(const std::string& msg);
}

#define SR_LOG_TEST(...)     SR_UTILS_NS::LogDetails::LogTestFwd(SR_FORMAT(__VA_ARGS__))
#define SR_LOG(...)          SR_UTILS_NS::LogDetails::LogFwd(SR_FORMAT(__VA_ARGS__))
#define SR_INFO(...)         SR_UTILS_NS::LogDetails::InfoFwd(SR_FORMAT(__VA_ARGS__))
#define SR_WARN(...)         SR_UTILS_NS::LogDetails::WarnFwd(SR_FORMAT(__VA_ARGS__))
#define SR_ERROR(...)        SR_UTILS_NS::LogDetails::ErrorFwd(SR_FORMAT(__VA_ARGS__))
#define SR_GRAPH(...)        SR_UTILS_NS::LogDetails::GraphLogFwd(SR_FORMAT(__VA_ARGS__))
#define SR_SUCCESS(...)      SR_UTILS_NS::LogDetails::SuccessFwd(SR_FORMAT(__VA_ARGS__))
#define SR_GRAPH_LOG(...)    SR_UTILS_NS::LogDetails::GraphLogFwd(SR_FORMAT(__VA_ARGS__))
#define SR_SHADER(...)       SR_UTILS_NS::LogDetails::ShaderFwd(SR_FORMAT(__VA_ARGS__))
#define SR_SHADER_LOG(...)   SR_UTILS_NS::LogDetails::ShaderLogFwd(SR_FORMAT(__VA_ARGS__))
#define SR_SYSTEM_LOG(...)   SR_UTILS_NS::LogDetails::SystemLogFwd(SR_FORMAT(__VA_ARGS__))
#define SR_VULKAN_MSG(...)   SR_UTILS_NS::LogDetails::VulkanMsgFwd(SR_FORMAT(__VA_ARGS__))
#define SR_VULKAN_LOG(...)   SR_UTILS_NS::LogDetails::VulkanLogFwd(SR_FORMAT(__VA_ARGS__))
#define SR_VULKAN_ERROR(...) SR_UTILS_NS::LogDetails::VulkanErrorFwd(SR_FORMAT(__VA_ARGS__))
#define SR_DEBUG_LOG(...)    SR_UTILS_NS::LogDetails::DebugLogFwd(SR_FORMAT(__VA_ARGS__))

#if defined(SR_DEBUG) || defined(SR_ANDROID)
    #define SR_ENABLE_ASSERTS
#endif

#ifdef SR_ENABLE_ASSERTS
    #define SRAssert2(expr, ...) (!!(expr) || SR_UTILS_NS::LogDetails::AssertFwd(SR_MAKE_ASSERT_MSG(SR_FORMAT(__VA_ARGS__))))

    #define SRAssert1(expr) SRAssert2(expr, #expr)
    #define SRAssert(expr) SRAssert2(expr, "An exception has been occured.")

    #define SRVerifyFalse2(expr, ...) (!(expr) || (SR_UTILS_NS::LogDetails::AssertFwd(SR_MAKE_ASSERT_MSG(SR_FORMAT(__VA_ARGS__))) && false))
    #define SRVerify2(expr, ...) ((expr) || (SR_UTILS_NS::LogDetails::AssertFwd(SR_MAKE_ASSERT_MSG(SR_FORMAT(__VA_ARGS__))) && false))

    #define SR_SAFE_PTR_ASSERT(expr, msg) SRAssert2(expr, SR_FORMAT("[SafePtr] {} \n\tPtr: {}", msg, (void *) m_ptr));

    #define SRAssert2Once(expr, ...) ((!(expr) && SR_UTILS_NS::LogDetails::AssertOnceCheckFwd(SR_MAKE_ASSERT_MSG(SR_FORMAT(__VA_ARGS__)))) || SRAssert2(expr, SR_FORMAT(__VA_ARGS__)))
#else
    #define SRAssert2(expr, ...) (SR_NOOP)
    #define SRAssert(expr) (SR_NOOP)
    #define SRAssert1(expr) SRAssert(expr)
    #define SR_SAFE_PTR_ASSERT(expr, msg) (SR_NOOP)
    #define SRAssert2Once(expr, ...) (SR_NOOP)
    #define SRVerifyFalse2(expr, ...) ((!(expr)))
    #define SRVerify2(expr, ...) ((expr))
#endif

#define SRHalt(...) SR_UTILS_NS::LogDetails::AssertFwd(SR_MAKE_ASSERT_MSG(SR_FORMAT(__VA_ARGS__)))
#define SRHaltTerminate(...) SR_UTILS_NS::LogDetails::AssertTerminateFwd(SR_MAKE_ASSERT_MSG(SR_FORMAT(__VA_ARGS__)))
#define SRHaltOnce(...) SR_UTILS_NS::LogDetails::AssertOnceCheckFwd(SR_MAKE_ASSERT_MSG(SR_FORMAT(__VA_ARGS__))) || SRHalt(SR_FORMAT(__VA_ARGS__))
#define SRHalt0() SRHalt("An exception has been occured!")
#define SRHaltOnce0() SRHaltOnce("An exception has been occured!")

#define SRVerifyFalse(expr) SRVerifyFalse2(expr, "An exception has been occured!")
#define SRVerify(expr) SRVerify2(expr, "An exception has been occured!")

#define SRAssert1Once(expr) SRAssert2Once(expr, #expr)
#define SRAssertOnce(expr) SRAssert2Once(expr, "An exception has been occured!")

#endif //SR_ENGINE_UTILS_ASSERT_FWD_H
