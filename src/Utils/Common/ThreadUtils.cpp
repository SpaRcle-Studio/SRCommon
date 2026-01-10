//
// Created by Monika on 02.01.2026.
//

#include <Utils/Common/ThreadUtils.h>

namespace SR_UTILS_NS {
    SR_UTILS_NS::StringAtom GetThisThreadId() {
        static SR_THREAD_LOCAL SR_UTILS_NS::StringAtom threadId;
        static SR_THREAD_LOCAL bool initialized = false;
        if (!initialized) {
            std::stringstream stream;
            stream << std::this_thread::get_id();
            threadId = stream.str();
            initialized = true;
        }
        return threadId;
    }
}