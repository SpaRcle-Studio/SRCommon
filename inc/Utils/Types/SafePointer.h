//
// Created by Monika on 04.08.2021.
//

#ifndef SMARTPOINTER_SAFEPOINTER_H
#define SMARTPOINTER_SAFEPOINTER_H

#include <Utils/Common/StringFormat.h>
#include <Utils/Types/Function.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Types/SharedPtr.h>

#define SR_SAFE_PTR_FORWARD_DECLARATION(className) \
    namespace SR_HTYPES_NS { \
        template<class T = className> class SafePtr; \
    } \


namespace SR_HTYPES_NS {
    #define SR_NEW_SAFE_PTR() {                              \
        }                                                    \

    #define SR_DEL_SAFE_PTR() {                              \
        }                                                    \

    struct SafePtrDynamicData {
        mutable std::atomic<bool>            m_lock;
        mutable std::atomic<uint32_t>        m_lockCount;
        mutable std::atomic<uint32_t>        m_useCount;
        bool                                 m_valid = false;
        mutable std::atomic<std::thread::id> m_owner;
    };

    template<class T> class SR_DLL_EXPORT SafePtr {
    public:
        explicit SafePtr(const T *ptr);
        SafePtr(SafePtr const &ptr);
        SafePtr();
        ~SafePtr(); /// не должен быть виртуальным
    public:
        operator bool() const noexcept { return m_data && m_data->m_valid; }
        SafePtr<T> &operator=(const SafePtr<T> &ptr);
        SafePtr<T> &operator=(T *ptr);
        T &operator*() const { return *m_ptr; }
        T *operator->() const { return m_ptr; }
        SR_NODISCARD SR_INLINE bool operator==(const SafePtr<T>& right) const noexcept {
            return m_ptr == right.m_ptr;
        }
        SR_NODISCARD SR_INLINE bool operator!=(const SafePtr<T>& right) const noexcept {
            return m_ptr != right.m_ptr;
        }
    public:
        bool TryLock() const;
        bool TryRecursiveLock() const;
        void Lock() const;
        void Unlock() const;
        bool TryUnlock() const;
        void RecursiveLock() const;

        void RemoveAllLocks();

        void Replace(const SafePtr &ptr);
        void ReplaceAndLock(const SafePtr& ptr);
        void ReplaceAndCopyLock(const SafePtr& ptr);

        template<typename U> SafePtr<U> DynamicCast() const {
            if constexpr (std::is_same_v<T, void>) {
                return SafePtr<U>();
            }
            return SafePtr<U>(dynamic_cast<U*>(m_ptr));
        }

        bool Do(const std::function<void(T* ptr)>& func);
        template<typename U> U Do(const std::function<U(T* ptr)>& func, U _default);
        template<typename U> U TryDo(const std::function<U(T* ptr)>& func, U _default);

        SR_NODISCARD bool TryLockIfValid() const;
        SR_NODISCARD bool TryRecursiveLockIfValid() const;
        SR_NODISCARD bool LockIfValid() const;
        SR_NODISCARD bool RecursiveLockIfValid() const;
        SR_NODISCARD SR_FORCE_INLINE SafePtrDynamicData* GetPtrData() const noexcept { return m_data; }

        SR_NODISCARD T* Get() const { return m_ptr; }
        SR_NODISCARD void* GetRawPtr() const { return (void*)m_ptr; }
        SR_NODISCARD SafePtr<T> GetThis() {
            return SafePtr<T>(*this);
        }
        SR_NODISCARD bool Valid() const { return m_data && m_data->m_valid; }
        SR_NODISCARD bool IsLocked() const { return Valid() && m_data->m_lock; }
        SR_NODISCARD uint32_t GetUseCount() const;

        bool AutoFree(const std::function<void(T *ptr)> &freeFun);
        bool AutoFree();
    private:
        bool FreeImpl(const std::function<void(T *ptr)> &freeFun);

    private:
        SafePtrDynamicData* m_data = nullptr;
        T* m_ptr = nullptr;
    };

    template<typename T>SafePtr<T>::SafePtr(const T *constPtr) {
        SR_NEW_SAFE_PTR();

        T* ptr = const_cast<T*>(constPtr);
        bool needAlloc = true;

        if constexpr (IsDerivedFrom<SafePtr, T>::value) {
            if (ptr && (m_data = ptr->GetPtrData())) {
                ++(m_data->m_useCount);
                needAlloc = false;
                m_ptr = ptr;
            }
        }

        if (needAlloc && ptr) {
            m_data = new SafePtrDynamicData {
                false,                         /// m_lock
                0,                             /// m_lockCount
                1,                             /// m_useCount
                (bool)(m_ptr = ptr),           /// m_valid
                std::atomic<std::thread::id>() /// m_owner
            };
        }
    }
    template<typename T>SafePtr<T>::SafePtr() {
        m_ptr = nullptr;

        SR_NEW_SAFE_PTR();

        m_data = new SafePtrDynamicData {
            false,                         // m_lock
            0,                             // m_lockCount
            1,                             // m_useCount
            false,                         // m_valid
            std::atomic<std::thread::id>() // m_owner
        };
    }
    template<typename T> SafePtr<T>::SafePtr(const SafePtr &ptr) {
        m_ptr = ptr.m_ptr;
        m_data = ptr.m_data;

        if (m_data) {
            ++m_data->m_useCount;
        }
    }
    template<typename T> SafePtr<T>::~SafePtr() {
        if (m_data && m_data->m_useCount <= 1) {
            SR_SAFE_PTR_ASSERT(!m_data->m_valid, "Ptr was not freed!");
            SR_SAFE_PTR_ASSERT(m_data->m_lockCount == 0 && !m_data->m_lock, "Ptr was not unlocked!");

            SR_DEL_SAFE_PTR();

            delete m_data;
        }
        else if (m_data) {
            --(m_data->m_useCount);
        }
    }

    template<typename T> SafePtr<T> &SafePtr<T>::operator=(const SafePtr<T> &ptr) {
        if (m_data && m_data->m_useCount <= 1) {
            SR_SAFE_PTR_ASSERT(!m_data->m_valid, "Ptr was not freed!");
            SR_SAFE_PTR_ASSERT(m_data->m_lockCount == 0 && !m_data->m_lock, "Ptr was not unlocked!");

            SR_DEL_SAFE_PTR();

            delete m_data;
        }
        else if (m_data) {
            --(m_data->m_useCount);
        }

        m_data = ptr.m_data;

        if (m_data) {
            m_data->m_valid = bool(m_ptr = ptr.m_ptr);
            ++(m_data->m_useCount);
        }

        return *this;
    }

    template<typename T> SafePtr<T> &SafePtr<T>::operator=(T *ptr) {
        if (m_ptr != ptr) {
            if (m_data && m_data->m_useCount <= 1) {
                SR_SAFE_PTR_ASSERT(!m_data->m_valid, "Ptr was not freed!");
                SR_SAFE_PTR_ASSERT(m_data->m_lockCount == 0 && !m_data->m_lock, "Ptr was not unlocked!");

                SR_DEL_SAFE_PTR();

                delete m_data;
            }
            else if (m_data) {
                --(m_data->m_useCount);
            }

            SR_NEW_SAFE_PTR();

            if (auto&& inherit = dynamic_cast<SafePtr<T>*>(ptr)) {
                m_data = inherit->m_data;
                ++(m_data->m_useCount);
            }
            else {
                m_data = new SafePtrDynamicData {
                        false,                         // m_lock
                        0,                             // m_lockCount
                        1,                             // m_useCount
                        false,                         // m_valid
                        std::atomic<std::thread::id>() // m_owner
                };
            }
        }

        if (m_data) {
            m_data->m_valid = bool(m_ptr = ptr);
        }

        return *this;
    }

    template<typename T> bool SafePtr<T>::AutoFree(const std::function<void(T *)> &freeFun) {
        SafePtr<T> ptrCopy = SafePtr<T>(*this);
        /// после вызова FreeImpl this может потенциально инвалидироваться!

        bool result = false;

        if (ptrCopy.RecursiveLockIfValid()) {
            result = ptrCopy.FreeImpl(freeFun);
            ptrCopy.Unlock();
        }

        return result;
    }

    template<typename T> bool SafePtr<T>::AutoFree() {
        SafePtr<T> ptrCopy = SafePtr<T>(*this);
        /// после вызова FreeImpl this может потенциально инвалидироваться!

        bool result = false;

        if (ptrCopy.RecursiveLockIfValid()) {
            result = ptrCopy.FreeImpl([](auto&& pData) { delete pData; });
            ptrCopy.Unlock();
        }

        return result;
    }

    template<typename T> bool SafePtr<T>::FreeImpl(const std::function<void(T *ptr)> &freeFun) {
        if (m_data && m_data->m_valid) {
            freeFun(m_ptr);
            m_data->m_valid = false;
            m_ptr = nullptr;
            return true;
        } else
            return false;
    }

    template<typename T> void SafePtr<T>::Lock() const {
        SR_TRACY_ZONE;

        if (!m_data) {
            return;
        }

        const std::thread::id this_id = std::this_thread::get_id();

        if(m_data->m_owner.load() == this_id) {
            SR_SAFE_PTR_ASSERT(false, "Double locking detected!");
        }
        else {
            bool expected = false;
            while (!m_data->m_lock.compare_exchange_weak(expected, true, std::memory_order_acquire))
                expected = false;

            m_data->m_owner.store(this_id);
            m_data->m_lockCount.store(1);
        }
    }

    template<typename T> void SafePtr<T>::Unlock() const {
        if (!m_data) {
            return;
        }

        if(m_data->m_lockCount > 1) {
            /// recursive unlocking
            --(m_data->m_lockCount);
        }
        else if (m_data->m_lockCount) {
            /// normal unlocking

            m_data->m_owner.store(std::thread::id());
            m_data->m_lockCount.store(0);

            m_data->m_lock.store(false, std::memory_order_release);
        }
        else
            SR_SAFE_PTR_ASSERT(false, "lock count = 0!");
    }

    template<typename T> SR_NODISCARD bool SafePtr<T>::LockIfValid() const {
        SR_TRACY_ZONE;

        Lock();

        if (m_data && m_data->m_valid)
            return true;

        Unlock();

        return false;
    }

    template<typename T> void SafePtr<T>::RecursiveLock() const {
        SR_TRACY_ZONE;

        if (!m_data) {
            return;
        }

        const std::thread::id this_id = std::this_thread::get_id();

        if(m_data->m_owner.load() == this_id) {
            /// recursive locking
            ++(m_data->m_lockCount);
            SR_SAFE_PTR_ASSERT("Lock count > 10000!", m_data->m_lockCount < 10000);
        }
        else {
            bool expected = false;
            while (!m_data->m_lock.compare_exchange_weak(expected, true, std::memory_order_acquire))
                expected = false;

            m_data->m_owner.store(this_id);
            m_data->m_lockCount.store(1);
        }
    }

    template<typename T> SR_NODISCARD bool SafePtr<T>::RecursiveLockIfValid() const {
        SR_TRACY_ZONE;

        if (!m_data) {
            return false;
        }

        RecursiveLock();

        if (m_data->m_valid)
            return true;

        Unlock();

        return false;
    }

    template<typename T> void SafePtr<T>::Replace(const SafePtr &ptr) {
        if (ptr.m_ptr == m_ptr)
            return;

        SafePtr copy = *this;
        copy.RecursiveLock();
        *this = ptr;
        copy.Unlock();
    }

    template<typename T> bool SafePtr<T>::TryLockIfValid() const {
        if (!TryLock())
            return false;

        if (m_data->m_valid)
            return true;

        Unlock();

        return false;
    }

    template<typename T> bool SafePtr<T>::TryLock() const {
        const std::thread::id this_id = std::this_thread::get_id();

        if (!m_data) {
            return false;
        }

        if(m_data->m_owner.load() == this_id) {
            SR_SAFE_PTR_ASSERT(false, "Double locking detected!");
            return false;
        }
        else {
            bool expected = false;
            while (!m_data->m_lock.compare_exchange_weak(expected, true, std::memory_order_acquire))
                return false;

            m_data->m_owner.store(this_id);
            m_data->m_lockCount.store(1);

            return true;
        }
    }

    template<typename T> uint32_t SafePtr<T>::GetUseCount() const {
        return Valid() ? m_data->m_useCount.load() : 0;
    }

    template<typename T> bool SafePtr<T>::TryRecursiveLockIfValid() const {
        if (!TryRecursiveLock())
            return false;

        if (m_data->m_valid)
            return true;

        Unlock();

        return false;
    }

    template<typename T> bool SafePtr<T>::TryRecursiveLock() const {
        const std::thread::id this_id = std::this_thread::get_id();

        if (!m_data) {
            return false;
        }

        if(m_data->m_owner.load() == this_id) {
            /// recursive locking
            ++(m_data->m_lockCount);
            SR_SAFE_PTR_ASSERT("Lock count > 10000!", m_data->m_lockCount < 10000);

            return true;
        }
        else {
            bool expected = false;
            while (!m_data->m_lock.compare_exchange_weak(expected, true, std::memory_order_acquire))
                return false;

            m_data->m_owner.store(this_id);
            m_data->m_lockCount.store(1);

            return true;
        }
    }

    template<typename T> void SafePtr<T>::ReplaceAndLock(const SafePtr& ptr) {
        if (ptr.m_ptr == m_ptr)
            return;

        ptr.RecursiveLock();

        SafePtr copy = *this;
        copy.RecursiveLock();
        *this = ptr;
        copy.Unlock();
    }

    template<typename T> void SafePtr<T>::ReplaceAndCopyLock(const SafePtr& ptr) {
        if (ptr.m_ptr == m_ptr)
            return;

        SafePtr copy = *this;
        copy.RecursiveLock();

        for (uint32_t i = 0; i < m_data->m_lockCount; ++i) {
            ptr.RecursiveLock();
        }

        *this = ptr;
        copy.Unlock();
    }

    template<typename T> bool SafePtr<T>::TryUnlock() const {
        if(m_data && m_data->m_lockCount > 1) {
            /// recursive unlocking
            --(m_data->m_lockCount);

            return true;
        }

        if (m_data && m_data->m_lockCount) {
            /// normal unlocking

            m_data->m_owner.store(std::thread::id());
            m_data->m_lockCount.store(0);

            m_data->m_lock.store(false, std::memory_order_release);

            return true;
        }

        return false;
    }

    template<typename T> void SafePtr<T>::RemoveAllLocks() {
        if (!m_data) {
            return;
        }

        m_data->m_owner.store(std::thread::id());
        m_data->m_lockCount.store(0);
        m_data->m_lock.store(false, std::memory_order_release);
    }

    template<class T> bool SafePtr<T>::Do(const std::function<void(T *)> &func)  {
        if (RecursiveLockIfValid()) {
            func(m_ptr);
            Unlock();
            return true;
        }

        return false;
    }

    template<class T> template<typename U> U SafePtr<T>::Do(const std::function<U(T *)> &func, U _default) {
        if (RecursiveLockIfValid()) {
            const auto&& result = func(m_ptr);
            Unlock();
            return result;
        }

        return _default;
    }

    template<class T> template<typename U> U SafePtr<T>::TryDo(const std::function<U(T *)> &func, U _default) {
        if (TryLockIfValid()) {
            const auto&& result = func(m_ptr);
            Unlock();
            return result;
        }

        return _default;
    }
}

namespace std {
    template<typename T> struct hash<SR_HTYPES_NS::SafePtr<T>> {
        size_t operator()(SR_HTYPES_NS::SafePtr<T> const& ptr) const {
            return std::hash<void*>()(ptr.GetRawPtr());
        }
    };

    template <typename T> struct less<SR_HTYPES_NS::SafePtr<T>> {
        bool operator()(const SR_HTYPES_NS::SafePtr<T> &lhs, const SR_HTYPES_NS::SafePtr<T> &rhs) const {
            void* a = lhs.GetRawPtr();
            void* b = rhs.GetRawPtr();
            return a < b;
        }
    };
}

#endif //SMARTPOINTER_SAFEPOINTER_H