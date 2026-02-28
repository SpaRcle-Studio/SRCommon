//
// Created by Monika on 13.05.2023.
//

#include <Utils/Profile/TracyContext.h>
#include <Utils/Debug.h>
#include <Utils/Platform/Platform.h>

#ifdef SR_TRACY_ENABLE

namespace SR_UTILS_NS {
    TracyContextManager::TracyContextManager()
        : Super()
    { }

    void TracyContextManager::Destroy(TracyType type) {
        if (type == TracyType::All) {
            for (auto&& contextMap : m_contexts) {
                for (auto&& [pIdentifier, pContext] : contextMap) {
                   Destroy(pContext, type);
                }
                contextMap.clear();
            }
            return;
        }

        ContextMap& contextMap = m_contexts[type];
        for (auto&& [pIdentifier, pContext] : contextMap) {
            Destroy(pContext, type);
        }
        m_contexts[type].clear();
    }

    void TracyContextManager::Destroy(TracyContextPtr pContext, TracyType type) {
        if (!pContext) {
            return;
        }

        switch (type) {
            #ifdef SR_USE_VULKAN
            case TracyType::Vulkan:
                if (VulkanDestroy) {
                    VulkanDestroy(pContext);
                }
                return;
            #endif
            default:
                break;
        }


        SRHalt("Failed to free tracy context!");
    }

    TracyContextManager::TracyContextPtr& TracyContextManager::GetContext(TracyType type, TracyContextManager::IdentifierPtr pIdentifier) {
        if (type >= TracyType::TracyTypeMAX - 1) {
            SR_PLATFORM_NS::WriteConsoleError("TracyContextManager::GetContext() : invalid TracyType!");
            SR_UTILS_NS::Breakpoint();
            static TracyContextPtr pNull = nullptr;
            return pNull;
        }

        return m_contexts[type][pIdentifier];
    }

    void TracyContextManager::InitSingleton() {
        Super::InitSingleton();
    }

    void StartupEngineProfiler() {
        tracy::StartupProfiler();

        g_TracyAllocatorInitialized = true;
    }

    void ShutdownEngineProfiler() {
        g_TracyAllocatorInitialized = false;

        tracy::ShutdownProfiler();
    }
}

#else

namespace SR_UTILS_NS {
    void StartupEngineProfiler() { }
    void ShutdownEngineProfiler() { }
}

#endif
