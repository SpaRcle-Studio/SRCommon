//
// Created by Monika on 23.08.2022.
//

#ifndef SR_ENGINE_SHARED_PTR_H
#define SR_ENGINE_SHARED_PTR_H

#include <Utils/Common/StringFormat.h>
#include <Utils/Types/Function.h>
#include <Utils/Common/TypeInfo.h>
#include <Utils/Debug.h>
#include <Utils/Platform/Platform.h>

namespace SR_UTILS_NS {
    enum class SharedPtrPolicy : uint8_t {
        Automatic, Manually
    };
}

/// #define SR_SHARED_PTR_TRACE

namespace SR_HTYPES_NS {
    class SharedPtrDynamicData;

    class SR_DLL_EXPORT SharedPtrDynamicDataCounter : public Singleton<SharedPtrDynamicDataCounter> {
        SR_REGISTER_SINGLETON(SharedPtrDynamicDataCounter);
    public:
        SR_NODISCARD uint64_t GetCount() const { return m_count; }

        void Increment(SharedPtrDynamicData* pData) {
            #ifdef SR_SHARED_PTR_TRACE
                m_data.insert(pData);
            #endif
            ++m_count;
        }

        void Decrement(SharedPtrDynamicData* pData) {
            #ifdef SR_SHARED_PTR_TRACE
                m_data.erase(pData);
            #endif
            --m_count;
        }

        SR_MAYBE_UNUSED static bool CheckMemoryLeaks();

    public:
        SR_NODISCARD bool IsSingletonCanBeDestroyed() const override { return false; }
        SR_NODISCARD const std::unordered_set<SharedPtrDynamicData*>& GetData() const { return m_data; }

    private:
        uint64_t m_count = 0;
        std::unordered_set<SharedPtrDynamicData*> m_data;

    };

    struct SharedPtrDynamicData {
        SharedPtrDynamicData(uint16_t strongCount, uint16_t weakCount, bool valid, SR_UTILS_NS::SharedPtrPolicy policy)
            : strongCount(strongCount)
            , weakCount(weakCount)
            , valid(valid)
            , policy(policy)
        {
            SharedPtrDynamicDataCounter::Instance().Increment(this);
        #ifdef SR_SHARED_PTR_TRACE
            debugTrace = SR_UTILS_NS::GetStacktrace();
        #endif
        }

        ~SharedPtrDynamicData() {
            SharedPtrDynamicDataCounter::Instance().Decrement(this);
        }

        SR_NODISCARD SR_UTILS_NS::StringAtom GetDebugTrace() const {
            #ifdef SR_SHARED_PTR_TRACE
                return debugTrace;
            #else
                return SR_UTILS_NS::StringAtom();
            #endif
        }

        SR_NODISCARD uint16_t GetStrongCount() const { return strongCount; }

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
        bool deallocated = false;
        SR_UTILS_NS::SharedPtrPolicy policy = SR_UTILS_NS::SharedPtrPolicy::Automatic;

    #ifdef SR_SHARED_PTR_TRACE
        SR_UTILS_NS::StringAtom debugTrace;
    #endif

    };

    template<class T> class SR_DLL_EXPORT SharedPtr {
    public:
        using Ptr = SharedPtr<T>;

        SharedPtr() = default;
        SharedPtr(const T* constPtr); /** NOLINT(google-explicit-constructor) */
        SharedPtr(const T* constPtr, SR_UTILS_NS::SharedPtrPolicy policy);
        SharedPtr(SharedPtr const& ptr);
        SharedPtr(SharedPtr&& ptr) noexcept
            : m_data(SR_UTILS_NS::Exchange(ptr.m_data, nullptr))
            , m_ptr(SR_UTILS_NS::Exchange(ptr.m_ptr, nullptr))
        { }
        virtual ~SharedPtr();

    public:
        template<typename U = T, typename R = U, typename... Args> SR_NODISCARD static SharedPtr<R> MakeShared(Args&&... args) {
            auto&& pData = new U(std::forward<Args>(args)...);
            if constexpr (std::is_same_v<R, T>) {
                return pData->GetThis();
            }
            else {
                SR_STATIC_ASSERT2((std::is_base_of_v<T, R> || std::is_base_of_v<R, T>), "Invalid type!");
                return pData->GetThis().template StaticCast<R>();
            }
        }

        SR_NODISCARD SR_FORCE_INLINE operator bool() const noexcept { return m_data && m_data->valid; } /** NOLINT */
        SharedPtr<T>& operator=(const SharedPtr<T>& ptr);
        SharedPtr<T>& operator=(T* ptr);
        SharedPtr<T>& operator=(SharedPtr<T>&& ptr) noexcept {
            Reset();

            m_data = SR_UTILS_NS::Exchange(ptr.m_data, {});

            /// не делаем инкремент, так как переместили!
            /// if (m_data) {
            ///     m_data->IncrementStrong();
            /// }

            m_ptr = SR_UTILS_NS::Exchange(ptr.m_ptr, {});

            return *this;
        }
        SR_FORCE_INLINE T& operator*() const { return *m_ptr; }
        SR_FORCE_INLINE T* operator->() const { return m_ptr; }
        SR_INLINE bool operator==(const SharedPtr<T>& right) const {
            return m_ptr == right.m_ptr;
        }
        SR_INLINE bool operator!=(const SharedPtr<T>& right) const {
            return m_ptr != right.m_ptr;
        }

        template<typename U> SharedPtr<U> PolymorphicCast() const {
        #ifdef SR_DEBUG
            if (!DynamicCast<U>()) {
                SR_SAFE_PTR_ASSERT(false, "Invalid cast!");
            }
        #endif
            return StaticCast<U>();
        }

        template<typename U> SharedPtr<U> DynamicCast() const {
            if constexpr (std::is_same_v<T, void>) {
                return SharedPtr<U>();
            }

            if (m_data && m_data->valid) {
                return SharedPtr<U>(dynamic_cast<U*>(m_ptr));
            }

            return SharedPtr<U>();
        }

        template<typename U> SharedPtr<U> StaticCast() const {
            if constexpr (std::is_same_v<T, void>) {
                return SharedPtr<U>();
            }

            if (m_data && m_data->valid) {
                return SharedPtr<U>(static_cast<U*>(m_ptr));
            }

            return SharedPtr<U>();
        }

        template<typename U> U ReinterpretCast() {
            return reinterpret_cast<U>(m_ptr);
        }

        const T* Get() const { return m_ptr; }
        T* Get() { return m_ptr; }

        const SharedPtrDynamicData* GetPtrData() const { return m_data; } /// NOLINT(modernize-use-nodiscard)
        SharedPtrDynamicData* GetPtrData() { return m_data; }

        const void* GetRawPtr() const { return reinterpret_cast<const void*>(m_ptr); } /// NOLINT(modernize-use-nodiscard)
        void* GetRawPtr() { return reinterpret_cast<void*>(m_ptr); }

        SR_NODISCARD SharedPtr<T> GetThis() const {
            return *this;
        }

        bool Valid() const { return m_data && m_data->valid; } /// NOLINT(modernize-use-nodiscard)

        bool AutoFree(const SR_HTYPES_NS::Function<void(T *ptr)>& freeFun);
        bool AutoFree();

        void Reset();

        SR_NODISCARD bool RecursiveLockIfValid() const noexcept;
        SR_NODISCARD bool TryRecursiveLockIfValid() const noexcept;
        void Unlock() const noexcept { /** nothing */  }

    private:
        bool FreeImpl(const SR_HTYPES_NS::Function<void(T *ptr)>& freeFun);

    private:
        SharedPtrDynamicData* m_data = nullptr;
        T* m_ptr = nullptr;
        bool m_basicManually = false;

    };

    template<class T> SharedPtr<T>::SharedPtr(const T* constPtr) {
        T* ptr = const_cast<T*>(constPtr);
        if (!ptr) {
            return;
        }

        if constexpr (SR_UTILS_NS::IsDerivedFrom<SharedPtr, T>::value) {
            if ((m_data = ptr->GetPtrData())) {
                m_data->IncrementStrong();
                m_ptr = ptr;
            }
            else {
                SR_SAFE_PTR_ASSERT(false, "Class was inherit, but not initialized!");
            }
        }
        else {
            m_ptr = ptr;
            m_data = new SharedPtrDynamicData(
                1, /// strong
                0, /// weak
                true, /// valid
                SR_UTILS_NS::SharedPtrPolicy::Automatic /// policy
            );
        }
    }

    template<class T> SharedPtr<T>::SharedPtr(const T* constPtr, SR_UTILS_NS::SharedPtrPolicy policy) {
        T* ptr = const_cast<T*>(constPtr);
        SR_SAFE_PTR_ASSERT(ptr, "Ptr is nullptr!");

        if constexpr (SR_UTILS_NS::IsDerivedFrom<SharedPtr, T>::value) {
            SR_SAFE_PTR_ASSERT(!ptr->GetPtrData(), "Class was inherit, but already initialized!");
        }

        m_basicManually = true;
        m_ptr = ptr;
        m_data = new SharedPtrDynamicData(
            0, /// strong
            0, /// weak
            true, /// valid
            policy /// policy
        );
    }

    template<class T> SharedPtr<T>::SharedPtr(const SharedPtr &ptr) {
        m_ptr = ptr.m_ptr;
        if ((m_data = ptr.m_data)) {
            m_data->IncrementStrong();
        }
    }

    template<class T> SharedPtr<T>::~SharedPtr() {
        if (!m_data || m_basicManually) {
            return;
        }

        Reset();
    }

    template<class T> SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<T>& ptr) {
        if (this == &ptr){
            return *this;
        }

        Reset();

        m_ptr = ptr.m_ptr;

        if ((m_data = ptr.m_data)) {
            m_data->valid = bool(m_ptr);
            m_data->IncrementStrong();
        }

        return *this;
    }

    template<class T> SharedPtr<T>& SharedPtr<T>::operator=(T* ptr) {
        if (!ptr) {
            Reset();
            return *this;
        }

        if (m_ptr == ptr) {
            return *this;
        }

        Reset();

        if constexpr (SR_UTILS_NS::IsDerivedFrom<SharedPtr, T>::value) {
            if ((m_data = ptr->GetPtrData())) {
                m_data->IncrementStrong();
                m_ptr = ptr;
            }
            else {
                SR_SAFE_PTR_ASSERT(false, "Class was inherit, but not initialized!");
            }
        }
        else {
            m_ptr = ptr;

            m_data = new SharedPtrDynamicData(
                1, /// strong
                0, /// weak
                true, /// valid
                SR_UTILS_NS::SharedPtrPolicy::Automatic /// policy
            );
        }

        return *this;
    }

    template<typename T> bool SharedPtr<T>::AutoFree(const SR_HTYPES_NS::Function<void(T *ptr)> &freeFun) {
        return Valid() && FreeImpl(freeFun);
    }

    template<typename T> bool SharedPtr<T>::AutoFree() {
        return Valid() && FreeImpl([](auto&& pData) { delete pData; });
    }

    template<typename T> bool SharedPtr<T>::FreeImpl(const SR_HTYPES_NS::Function<void(T* ptr)> &freeFun) {
        if (m_data) {
            const bool valid = m_data->valid;
            const auto pPtr = m_ptr;
            SharedPtrDynamicData* pData = m_data;

            if (valid) {
                pData->deallocated = true;
                freeFun(pPtr);
                pData->valid = false;
                return true;
            }

            pData->valid = false;

            return false;
        }

        return false;
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

        SR_SAFE_PTR_ASSERT(strongCount != 0, "SharedPtr is corrupted!");

        if (strongCount == 1) {
            if (pData->policy == SR_UTILS_NS::SharedPtrPolicy::Manually) {
                SR_SAFE_PTR_ASSERT(pData->deallocated, "Ptr was not freed!");
            }
            else if (pData->policy == SR_UTILS_NS::SharedPtrPolicy::Automatic && pData->valid) {
                pData->valid = false;
                delete pPtr;
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
            const void* a = lhs.GetRawPtr();
            const void* b = rhs.GetRawPtr();
            return a < b;
        }
    };
}

#endif //SR_ENGINE_SHARED_PTR_H
