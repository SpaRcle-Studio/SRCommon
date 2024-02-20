//
// Created by Monika on 23.08.2022.
//

#ifndef SR_ENGINE_SHAREDPTR_H
#define SR_ENGINE_SHAREDPTR_H

#include <Utils/Common/StringFormat.h>
#include <Utils/Types/Function.h>
#include <Utils/Common/TypeInfo.h>
#include <Utils/Debug.h>

namespace SR_UTILS_NS {
    enum class SharedPtrPolicy : uint8_t {
        Automatic, Manually
    };
}

namespace SR_HTYPES_NS {
    class SR_DLL_EXPORT SharedPtrDynamicDataCounter : public Singleton<SharedPtrDynamicDataCounter> {
        SR_REGISTER_SINGLETON(SharedPtrDynamicDataCounter);
    public:
        SR_NODISCARD uint64_t GetCount() const { return m_count; }

        void Increment() { ++m_count; }
        void Decrement() { --m_count; }

    public:
        SR_NODISCARD bool IsSingletonCanBeDestroyed() const override { return false; }

    private:
        uint64_t m_count = 0;

    };

    struct SharedPtrDynamicData {
        SharedPtrDynamicData(uint16_t strongCount, uint16_t weakCount, bool valid, SharedPtrPolicy policy)
            : strongCount(strongCount)
            , weakCount(weakCount)
            , valid(valid)
            , policy(policy)
        {
            SharedPtrDynamicDataCounter::Instance().Increment();
        }

        ~SharedPtrDynamicData() {
            SharedPtrDynamicDataCounter::Instance().Decrement();
        }

        void IncrementStrong() {
            SRAssert2(strongCount != SR_UINT16_MAX, "Strong count overflow!");
            ++strongCount;
        }

        void DecrementStrong() {
            SRAssert2(strongCount != 0, "Strong count underflow!");
            --strongCount;
        }

        uint16_t strongCount = 0;
        uint16_t weakCount = 0;
        bool valid = false;
        SharedPtrPolicy policy = SharedPtrPolicy::Automatic;

    };

    template<class T> class SR_DLL_EXPORT SharedPtr {
    public:
        using Ptr = SharedPtr<T>;

        SharedPtr() = default;
        SharedPtr(const T* constPtr); /** NOLINT */
        SharedPtr(const T* constPtr, SharedPtrPolicy policy); /** NOLINT */
        SharedPtr(SharedPtr const &ptr);
        SharedPtr(SharedPtr&& ptr) noexcept
            : m_data(SR_UTILS_NS::Exchange(ptr.m_data, nullptr))
            , m_ptr(SR_UTILS_NS::Exchange(ptr.m_ptr, nullptr))
        { }
        ~SharedPtr(); /// не должен быть виртуальным

    public:
        template<typename U = T, typename R = U, typename... Args> SR_NODISCARD static SharedPtr<R> MakeShared(Args&&... args) {
            auto&& pData = new U(std::forward<Args>(args)...);
            if constexpr (std::is_same_v<R, T>) {
                return pData->GetThis();
            }
            else {
                return pData->GetThis().template DynamicCast<R>();
            }
        }

        SR_NODISCARD SR_FORCE_INLINE operator bool() const noexcept { return m_data && m_data->valid; } /** NOLINT */
        SharedPtr<T>& operator=(const SharedPtr<T> &ptr);
        SharedPtr<T>& operator=(T *ptr);
        SharedPtr<T>& operator=(SharedPtr<T>&& ptr) noexcept {
            if (m_data) {
                m_data->DecrementStrong();
            }

            m_data = SR_UTILS_NS::Exchange(ptr.m_data, {});

            /// не делаем инкремент, так как переместили!
            /// if (m_data) {
            ///     m_data->IncrementStrong();
            /// }

            m_ptr = SR_UTILS_NS::Exchange(ptr.m_ptr, {});

            return *this;
        }
        SR_NODISCARD SR_FORCE_INLINE T& operator*() const noexcept { return *m_ptr; }
        SR_NODISCARD SR_FORCE_INLINE T* operator->() const noexcept { return m_ptr; }
        SR_NODISCARD SR_INLINE bool operator==(const SharedPtr<T>& right) const noexcept {
            return m_ptr == right.m_ptr;
        }
        SR_NODISCARD SR_INLINE bool operator!=(const SharedPtr<T>& right) const noexcept {
            return m_ptr != right.m_ptr;
        }
        template<typename U> SharedPtr<U> DynamicCast() const {
            if constexpr (std::is_same_v<T, void>) {
                return SharedPtr<U>();
            }
            return SharedPtr<U>(dynamic_cast<U*>(m_ptr));
        }

        template<typename U> U ReinterpretCast() {
            return reinterpret_cast<U>(m_ptr);
        }

        SR_NODISCARD SR_FORCE_INLINE T* Get() const noexcept { return m_ptr; }
        SR_NODISCARD SR_FORCE_INLINE SharedPtrDynamicData* GetPtrData() const noexcept { return m_data; }
        SR_NODISCARD SR_FORCE_INLINE void* GetRawPtr() const noexcept { return (void*)m_ptr; }
        SR_NODISCARD SharedPtr<T> GetThis() const {
            return *this;
        }
        SR_NODISCARD SR_FORCE_INLINE bool Valid() const noexcept { return m_data && m_data->valid; }

        bool AutoFree(const SR_HTYPES_NS::Function<void(T *ptr)>& freeFun);
        bool AutoFree();

        /// Оставляем методы для совместимости с SafePtr
        void Replace(const SharedPtr &ptr);
        void Reset();
        SR_NODISCARD SR_FORCE_INLINE bool RecursiveLockIfValid() const noexcept;
        SR_NODISCARD SR_FORCE_INLINE bool TryRecursiveLockIfValid() const noexcept;
        SR_FORCE_INLINE void Unlock() const noexcept { /** nothing */  }

    private:
        bool FreeImpl(const SR_HTYPES_NS::Function<void(T *ptr)> &freeFun);

    private:
        SharedPtrDynamicData* m_data = nullptr;
        T* m_ptr = nullptr;

    };

    template<class T> SharedPtr<T>::SharedPtr(const T* constPtr)
        : SharedPtr(constPtr, SharedPtrPolicy::Automatic)
    { }

    template<class T> SharedPtr<T>::SharedPtr(const T* constPtr, SharedPtrPolicy policy) {
        T* ptr = const_cast<T*>(constPtr);
        bool needAlloc = true;

        if constexpr (IsDerivedFrom<SharedPtr, T>::value) {
            if (ptr && (m_data = ptr->GetPtrData())) {
                m_data->IncrementStrong();
                needAlloc = false;
                m_ptr = ptr;
            }
        }

        if (needAlloc && ptr) {
            m_data = new SharedPtrDynamicData(
                0,                   /// strong
                0,                   /// weak
                (bool)(m_ptr = ptr), /// valid
                policy               /// policy
            );
        }
    }

    template<class T> SharedPtr<T>::SharedPtr(const SharedPtr &ptr) {
        m_ptr = ptr.m_ptr;
        if ((m_data = ptr.m_data)) {
            m_data->IncrementStrong();
        }
    }

    template<class T> SharedPtr<T>::~SharedPtr() {
        Reset();
    }

    template<class T> SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<T> &ptr) {
        Reset();

        m_ptr = ptr.m_ptr;

        if ((m_data = ptr.m_data)) {
            m_data->valid = bool(m_ptr);
            m_data->IncrementStrong();
        }

        return *this;
    }

    template<class T> SharedPtr<T>& SharedPtr<T>::operator=(T *ptr) {
        if (m_ptr != ptr) {
            Reset();

            bool needAlloc = true;

            if constexpr (IsDerivedFrom<SharedPtr, T>::value) {
                if (ptr && (m_data = ptr->GetPtrData())) {
                    m_data->IncrementStrong();
                    needAlloc = false;
                    m_ptr = ptr;
                }
                else if (ptr) {
                    SR_SAFE_PTR_ASSERT(false, "Class was inherit, but not initialized!");
                }
            }

            if (needAlloc && ptr) {
                m_data = new SharedPtrDynamicData(
                    0,    /// strong
                    0,    /// weak
                    true, /// valid
                    SharedPtrPolicy::Automatic /// policy
                );
            }
        }

        m_ptr = ptr;

        if (m_data) {
            m_data->valid = bool(m_ptr);
        }

        return *this;
    }

    template<typename T> bool SharedPtr<T>::AutoFree(const SR_HTYPES_NS::Function<void(T *ptr)> &freeFun) {
        SharedPtr<T> ptrCopy = SharedPtr<T>(*this);

        /// чтобы при ручном освобождении не ругаться не не освобожденную память,
        /// так как в последней копии все равно вызовется деструктор при наследовании, делаем вспомогательную копию.
        SharedPtr<T> ptrCopy2 = ptrCopy;

        /// после вызова FreeImpl this может потенциально инвалидироваться!

        return ptrCopy.Valid() ? ptrCopy.FreeImpl(freeFun) : false;
    }

    template<typename T> bool SharedPtr<T>::AutoFree() {
        SharedPtr<T> ptrCopy = SharedPtr<T>(*this);

        /// чтобы при ручном освобождении не ругаться не не освобожденную память,
        /// так как в последней копии все равно вызовется деструктор при наследовании, делаем вспомогательную копию.
        SharedPtr<T> ptrCopy2 = ptrCopy;

        /// после вызова FreeImpl this может потенциально инвалидироваться!

        return ptrCopy.Valid() ? ptrCopy.FreeImpl([](auto&& pData) { delete pData; }) : false;
    }

    template<typename T> bool SharedPtr<T>::FreeImpl(const SR_HTYPES_NS::Function<void(T *ptr)> &freeFun) {
        if (m_data && m_data->valid) {
            freeFun(m_ptr);
            m_data->valid = false;
            m_ptr = nullptr;
            return true;
        }
        else {
            return false;
        }
    }

    template<class T> void SharedPtr<T>::Replace(const SharedPtr &ptr) {
        if (ptr.m_ptr == m_ptr && ptr.m_data == m_data) {
            return;
        }

        SharedPtr copy = *this;
        *this = ptr;
    }

    template<class T> bool SharedPtr<T>::RecursiveLockIfValid() const noexcept {
        return m_data && m_data->valid;
    }

    template<class T> bool SharedPtr<T>::TryRecursiveLockIfValid() const noexcept {
        return m_data && m_data->valid;
    }

    template<class T> void SharedPtr<T>::Reset() {
        /// Делаем копию, так как в процессе удаления можем потярять this,
        /// а так же зануляем m_data, чтобы не войти в рекурсию
        SharedPtrDynamicData* pData = m_data;
        T* pPtr = m_ptr; /// тут может быть потенциально висячий указатель.
        m_data = nullptr;
        m_ptr = nullptr;

        if (!pData) {
            return;
        }

        const auto strongCount = pData->strongCount;
        if (strongCount < 1) {
            if (pData->policy == SharedPtrPolicy::Manually) {
                SR_SAFE_PTR_ASSERT(!pData->valid, "Ptr was not freed!");
                delete pData;
                return;
            }
            else if (pData->policy == SharedPtrPolicy::Automatic && pData->valid) {
                pData->valid = false;
                SR_SAFE_DELETE_PTR(pPtr);
            }

            if (pData->weakCount == 0) {
                delete pData;
            }
        }
        else {
            pData->DecrementStrong();
        }
    }
}

namespace std {
    template<typename T> struct hash<SR_HTYPES_NS::SharedPtr<T>> {
        size_t operator()(SR_HTYPES_NS::SharedPtr<T> const& ptr) const {
            return std::hash<void*>()(ptr.GetRawPtr());
        }
    };

    template <typename T> struct less<SR_HTYPES_NS::SharedPtr<T>> {
        bool operator()(const SR_HTYPES_NS::SharedPtr<T> &lhs, const SR_HTYPES_NS::SharedPtr<T> &rhs) const {
            void* a = lhs.GetRawPtr();
            void* b = rhs.GetRawPtr();
            return a < b;
        }
    };
}

#endif //SR_ENGINE_SHAREDPTR_H
