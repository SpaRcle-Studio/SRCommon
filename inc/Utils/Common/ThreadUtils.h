//
// Created by Monika on 10.05.2022.
//

#ifndef SR_ENGINE_THREADUTILS_H
#define SR_ENGINE_THREADUTILS_H

#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_T(ThreadPriority, int8_t,
          SR_THREAD_PRIORITY_ABOVE_NORMAL,
          SR_THREAD_PRIORITY_BELOW_NORMAL,
          SR_THREAD_PRIORITY_HIGHEST,
          SR_THREAD_PRIORITY_IDLE,
          SR_THREAD_PRIORITY_LOWEST,
          SR_THREAD_PRIORITY_NORMAL,
          SR_THREAD_PRIORITY_TIME_CRITICAL
    );

    SR_INLINE String GetThreadId(const std::thread& thread) {
        std::stringstream stream;
        stream << thread.get_id();

        if (stream.str() == "thread::id of a non-executing thread") {
            return String();
        }

        return stream.str();
    }

    SR_COMMON_DLL_API extern SR_UTILS_NS::StringAtom GetThisThreadId();
}

#endif //SR_ENGINE_THREADUTILS_H
