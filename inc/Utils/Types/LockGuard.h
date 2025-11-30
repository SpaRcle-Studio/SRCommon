//
// Created by Monika on 18.10.2023.
//

#ifndef SR_ENGINE_LOCKGUARD_H
#define SR_ENGINE_LOCKGUARD_H

#include <Utils/Common/NonCopyable.h>

namespace SR_HTYPES_NS {
    template<typename T> class SR_COMMON_DLL_API LockGuard : public NonCopyable {
    public:
        explicit LockGuard(T& mutex)
            : m_mutex(mutex)
        {
            m_mutex.lock();
        }

        ~LockGuard() override {
            m_mutex.unlock();
        }

        LockGuard(LockGuard&& other) noexcept = delete;
        LockGuard& operator=(LockGuard&& rhs) noexcept = delete;

    private:
        T& m_mutex;

    };
}

#define SR_LOCK_GUARD std::lock_guard<std::recursive_mutex> codegen_lock(m_mutex)
#define SR_LOCK_GUARD_INHERIT(baseClass) std::lock_guard<std::recursive_mutex> codegen_lock(baseClass::m_mutex)
#define SR_SCOPED_LOCK std::lock_guard<std::recursive_mutex> codegen_lock(m_mutex)
#define SR_WRITE_LOCK std::lock_guard<std::shared_mutex> SR_MACRO_CONCAT(codegen_write_lock, SR_LINE)(m_mutex)
#define SR_READ_LOCK std::shared_lock<std::shared_mutex> SR_MACRO_CONCAT(codegen_read_lock, SR_LINE)(m_mutex)

#endif //SR_ENGINE_LOCKGUARD_H
