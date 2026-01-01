//
// Created by Monika on 13.05.2023.
//

#ifndef SR_ENGINE_UTILS_TRACY_CONTEXT_H
#define SR_ENGINE_UTILS_TRACY_CONTEXT_H

#ifdef SR_TRACY_ENABLE

#include <Utils/Common/Singleton.h>
#include <Utils/Common/Enumerations.h>

#ifndef SR_ENGINE_CODEGEN_CLANG_PARSE_MODE
    #include <Tracy.hpp>
#endif

namespace SR_UTILS_NS {
    SR_ENUM_NS_STRUCT_T(TracyType, uint8_t,
        Common,
        Vulkan,

        All
    );

    extern SR_COMMON_DLL_API void StartupEngineProfiler();
    extern SR_COMMON_DLL_API void ShutdownEngineProfiler();

    class TracyContextManager : public SR_UTILS_NS::Singleton<TracyContextManager> {
        SR_REGISTER_SINGLETON(TracyContextManager)
        using TracyContextPtr = void*;
        using IdentifierPtr = void*;
        using ContextMap = std::map<IdentifierPtr, TracyContextPtr>;
    public:
        TracyContextPtr& GetContext(TracyType type, IdentifierPtr pIdentifier);

        void Destroy(TracyType type);

    #ifdef SR_USE_VULKAN
        std::function<void(void*)> VulkanDestroy;
    #endif

    private:
        void Destroy(TracyContextPtr pContext, TracyType type);

    private:
        std::array<ContextMap, TracyType::TracyTypeMAX - 1> m_contexts;

    };
}
#endif

#ifdef SR_TRACY_ENABLE
    #define SR_TRACY_IS_PROFILER_CONNECTED (tracy::GetProfiler().IsConnected())
    #define SR_TRACY_TEXT_N(name, text) ZoneText(text.c_str(), text.size())
    #define SR_TRACY_ZONE ZoneScoped /** NOLINT */
    #define SR_TRACY_ZONE_VALUE(value) ZoneValue(value)
    #define SR_TRACY_ZONE_TEXT(value) ZoneText(value.c_str(), value.size())
    #define SR_TRACY_ZONE_TEXT_C(value) ZoneText(value, strlen(value))
    #define SR_TRACY_ZONE_N(name) ZoneScopedN(name)
    #define SR_TRACY_ZONE_S(name) ZoneTransientN(TracyConcat(__tracy_source_location, TracyLine), name, true)
    #define SR_TRACY_FRAME_MARK FrameMark()
    #define SR_TRACY_FRAME_MARK_N(name) FrameMarkNamed(name)
    #define SR_TRACY_FRAME_IMAGE(ptr, width, height, channels, size) FrameImage(ptr, width, height, channels, size)
    #define SR_TRACY_FRAME_START(name) FrameMarkStart(name)
    #define SR_TRACY_FRAME_END(name) FrameMarkEnd(name)
    #define SR_TRACY_PLOT(name, val) TracyPlot(name, val)
    #define SR_TRACY_ZONE_COLOR(color) ZoneColor(color)

    #define SR_TRACY_GET_CONTEXT(tracyType, pIdentifier) \
        SR_UTILS_NS::TracyContextManager::Instance().GetContext(tracyType, pIdentifier)

    #define SR_TRACY_DESTROY(tracyType) SR_UTILS_NS::TracyContextManager::Instance().Destroy(tracyType);
    #define SR_TRACY_THREAD_NAME(name) tracy::SetThreadName(name)
#else
    #define SR_TRACY_IS_PROFILER_CONNECTED (false)
    #define SR_TRACY_TEXT_N(name, text)
    #define SR_TRACY_ZONE SR_NOOP
    #define SR_TRACY_ZONE_VALUE(value) SR_NOOP
    #define SR_TRACY_ZONE_TEXT(value) SR_NOOP
    #define SR_TRACY_ZONE_TEXT_C(value) SR_NOOP
    #define SR_TRACY_ZONE_N(name) SR_NOOP
    #define SR_TRACY_ZONE_S(name) SR_NOOP
    #define SR_TRACY_FRAME_MARK SR_NOOP
    #define SR_TRACY_FRAME_MARK_N(name) SR_NOOP
    #define SR_TRACY_FRAME_IMAGE(ptr, width, height, channels, size) SR_NOOP
    #define SR_TRACY_FRAME_START(name) SR_NOOP
    #define SR_TRACY_FRAME_END(name) SR_NOOP
    #define SR_TRACY_PLOT(name, val) SR_NOOP
    #define SR_TRACY_ZONE_COLOR(color)

    #define SR_TRACY_GET_CONTEXT(tracyType, pIdentifier)
    #define SR_TRACY_DESTROY(tracyType)
    #define SR_TRACY_THREAD_NAME(name)
#endif


#endif //SR_ENGINE_UTILS_TRACY_CONTEXT_H
