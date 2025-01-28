//
// Created by Monika on 26.05.2024.
//

#ifndef SR_ENGINE_UTILS_CAST_UTILS_H
#define SR_ENGINE_UTILS_CAST_UTILS_H

#include <Utils/Platform/Platform.h>

namespace SR_UTILS_NS {
    template<typename B, typename A> B PolymorphicCast(A pA) {
        if constexpr (std::is_same_v<void*, A>) {
            return static_cast<B>(pA);
        }

    #if defined(SR_DEBUG)
        if (!dynamic_cast<B>(pA)) {
            SR_PLATFORM_NS::Terminate();
        }
    #endif
        return static_cast<B>(pA);
    }
}

#endif //SR_ENGINE_UTILS_CAST_UTILS_H
