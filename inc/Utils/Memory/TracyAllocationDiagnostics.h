#ifndef SR_ENGINE_TRACY_ALLOCATION_DIAGNOSTICS_H
#define SR_ENGINE_TRACY_ALLOCATION_DIAGNOSTICS_H

#ifdef SR_TRACY_ALLOCATION_DIAGNOSTICS
#include <Utils/Memory/Allocator.h>

namespace SR_UTILS_NS::TracyAllocationDiagnostics {
    SR_COMMON_DLL_API void OnAllocation(void* address, SR_UTILS_NS::SizeType size);
    SR_COMMON_DLL_API void OnFree(void* address);
    SR_COMMON_DLL_API void OnCrtFree(void* address);

#if defined(SR_WIN32) && defined(SR_COMMON_USE_MINHOOK)
    SR_COMMON_DLL_API void InitializeCrtHooks(void* crt);
#endif
}
#endif

#endif // SR_ENGINE_TRACY_ALLOCATION_DIAGNOSTICS_H
