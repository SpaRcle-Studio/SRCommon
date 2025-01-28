//
// Created by Monika on 16.01.2022.
//

#ifndef SR_ENGINE_NONCOPYABLE_H
#define SR_ENGINE_NONCOPYABLE_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class SR_DLL_EXPORT NonCopyable {
    protected:
        constexpr NonCopyable() = default;
        virtual ~NonCopyable() = default;

    public:
        NonCopyable(const NonCopyable&) = delete;
        virtual NonCopyable& operator=(const NonCopyable&) = delete;

    };

    class SR_DLL_EXPORT NonMovable {
    protected:
        constexpr NonMovable() = default;
        virtual ~NonMovable() = default;

    public:
        NonMovable(NonMovable&&) = delete;
        virtual NonMovable& operator=(NonMovable&&) = delete;

    };
}

#endif //SR_ENGINE_NONCOPYABLE_H
