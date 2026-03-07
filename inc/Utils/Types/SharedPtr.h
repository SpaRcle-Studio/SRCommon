//
// Created by Monika on 23.08.2022.
//

#if !defined(SR_ENGINE_SHARED_PTR_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_ENGINE_SHARED_PTR_H

#include <Utils/Types/SharedPtrDynamicData.h>

namespace SR_HTYPES_NS {
    class SR_COMMON_DLL_API SharedPtrBase {
    public:
        SharedPtrBase();
        explicit SharedPtrBase(SharedPtrDynamicData* data);
        virtual ~SharedPtrBase();

    public:
        const SharedPtrDynamicData* GetPtrData() const; /// NOLINT(modernize-use-nodiscard)
        SharedPtrDynamicData* GetPtrData();
        SR_NODISCARD virtual SRClass* GetSRClass() const = 0;
        SR_NODISCARD virtual bool Valid() const = 0;
        virtual void Reset() = 0;
        virtual void IncrementPointer() = 0;
        virtual void DecrementPointer() = 0;
        virtual void SetPointerFromBase(SharedPtrBase* pBase) = 0;

        SR_NODISCARD uint64_t GetStrongCount() const;

    protected:
        SharedPtrDynamicData* m_data = nullptr;
        bool m_basicManually = false;

    };

    template<typename T> class WeakPtr;

    template<class T> class SharedPtr : public SharedPtrBase {
        friend class WeakPtr<T>;
    public:
        using Ptr = SharedPtr<T>;
        using SharedPointerType = T;

        SharedPtr() = default;
        SharedPtr(const T* constPtr); /** NOLINT(google-explicit-constructor) */
        SharedPtr(const T* constPtr, SR_UTILS_NS::SharedPtrPolicy policy);
        SharedPtr(SharedPtr const& ptr);
        SharedPtr(SharedPtr&& ptr) noexcept;
        ~SharedPtr() override;

    public:
        template<typename U = T, typename R = U, typename... Args> SR_NODISCARD static SharedPtr<R> MakeShared(Args&&... args);

        SharedPtr<T>& operator=(const SharedPtr<T>& ptr);
        SharedPtr<T>& operator=(T* ptr);
        SharedPtr<T>& operator=(SharedPtr<T>&& ptr) noexcept;

        template<typename Y> SR_DEPRECATED SR_NODISCARD SharedPtr<Y> DynamicCast() const;
        template<typename Y> SR_DEPRECATED SR_NODISCARD SharedPtr<Y> StaticCast() const;

        SR_NODISCARD SR_FORCE_INLINE operator bool() const noexcept { return m_data && m_data->valid; } /** NOLINT */
        SR_NODISCARD SR_FORCE_INLINE T& operator*() const { return *m_ptr; }
        SR_NODISCARD SR_FORCE_INLINE T* operator->() const { return m_ptr; }
        SR_NODISCARD SR_FORCE_INLINE bool operator==(const SharedPtr<T>& right) const { return m_ptr == right.m_ptr; }
        SR_NODISCARD SR_FORCE_INLINE bool operator!=(const SharedPtr<T>& right) const { return m_ptr != right.m_ptr; }

        SR_NODISCARD SR_FORCE_INLINE const T* Get() const { return m_ptr; }
        SR_NODISCARD SR_FORCE_INLINE T* Get() { return m_ptr; }
        SR_NODISCARD SR_FORCE_INLINE const T& SR_FASTCALL GetUncheckedRef() const { return *m_ptr; }
        SR_NODISCARD SR_FORCE_INLINE T& SR_FASTCALL GetUncheckedRef() { return *m_ptr; }

        SR_NODISCARD SR_FORCE_INLINE const void* GetRawPtr() const { return reinterpret_cast<const void*>(m_ptr); } /// NOLINT(modernize-use-nodiscard)
        SR_NODISCARD SR_FORCE_INLINE void* GetRawPtr() { return reinterpret_cast<void*>(m_ptr); }
        SR_NODISCARD SR_FORCE_INLINE bool Valid() const final { return m_data && m_data->valid; }

        void SetPointerFromBase(SharedPtrBase* pBase) override;

        SR_NODISCARD SRClass* GetSRClass() const override;
        SR_NODISCARD SharedPtr<T> GetThis() const { return *this; }
        template<class Y = T> SR_NODISCARD WeakPtr<Y> GetWeakThis() const;

        void IncrementPointer() override;
        void DecrementPointer() override;

        bool AutoFree(const SR_HTYPES_NS::Function<void(T* pPtr, SharedPtrDynamicData* pControl)>& freeFun);
        bool AutoFree();
        void Reset() override;

    private:
        bool FreeImpl(const SR_HTYPES_NS::Function<void(T* pPtr, SharedPtrDynamicData* pControl)>& freeFun);

    private:
        T* m_ptr = nullptr;

    };

    /// ================================================= IMPLEMENTATION ===============================================

    #pragma region IMPLEMENTATION

    template<class T> SharedPtr<T>::SharedPtr(SharedPtr&& ptr) noexcept
        : SharedPtrBase(SR_UTILS_NS::Exchange(ptr.m_data, nullptr))
        , m_ptr(SR_UTILS_NS::Exchange(ptr.m_ptr, nullptr))
    { }

    template<class T> SharedPtr<T>::SharedPtr(const T* constPtr) {
        T* ptr = const_cast<T*>(constPtr);
        if (!ptr) {
            return;
        }

        m_ptr = ptr;

        if constexpr (!SR_UTILS_NS::IsCompleteTypeV<T>) {
            static_assert(AlwaysFalseV<T>, "SharedPtr<T>::SharedPtr(const T* constPtr) : T must be a complete type!");
        }
        else if constexpr (SR_UTILS_NS::IsDerivedFrom<SharedPtr, T>::value) {
            if ((m_data = ptr->GetPtrData())) {
                m_data->IncrementStrong();
            }
            else {
                SR_SAFE_PTR_ASSERT(false, "Class was inherit, but not initialized! Or called wrong constructor with policy!");
            }
        }
        else if constexpr (std::is_polymorphic_v<T>) {
            if (auto&& pBase = dynamic_cast<SharedPtrBase*>(ptr)) {
                m_data = pBase->GetPtrData();
                m_data->IncrementStrong();
            }
            else {
                m_data = new SharedPtrDynamicData(
                    1, /// strong
                    0, /// weak
                    true, /// valid
                    SR_UTILS_NS::SharedPtrPolicy::Automatic /// policy
                );
                m_data->InitBasic<T>();
            }
        }
        else {
            m_data = new SharedPtrDynamicData(
                1, /// strong
                0, /// weak
                true, /// valid
                SR_UTILS_NS::SharedPtrPolicy::Automatic /// policy
            );
            m_data->InitBasic<T>();
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
        m_data->InitBasic<T>();
    }

    template<class T> SharedPtr<T>::SharedPtr(const SharedPtr& ptr) {
        m_ptr = ptr.m_ptr;
        if ((m_data = ptr.m_data)) {
            m_data->IncrementStrong();
        }
    }

    template<class T> SharedPtr<T>::~SharedPtr() {
        if (!m_data || m_basicManually) {
            if (m_data && m_data->strongCount == 0 && m_data->weakCount == 0) {
                delete m_data;
                m_data = nullptr;
            }
            return;
        }

        Reset();
    }

    template<class T> SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<T>&& ptr) noexcept {
        Reset();
        m_data = SR_UTILS_NS::Exchange(ptr.m_data, {});
        m_ptr = SR_UTILS_NS::Exchange(ptr.m_ptr, {});
        return *this;
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

        m_ptr = ptr;

        if constexpr (SR_UTILS_NS::IsDerivedFrom<SharedPtr, T>::value) {
            if ((m_data = ptr->GetPtrData())) {
                m_data->IncrementStrong();
                m_ptr = ptr;
            }
            else {
                SR_SAFE_PTR_ASSERT(false, "Class was inherit, but not initialized!");
            }
        }
        else if constexpr (std::is_polymorphic_v<T>) {
            if (auto&& pBase = dynamic_cast<SharedPtrBase*>(ptr)) {
                m_data = pBase->GetPtrData();
                m_data->IncrementStrong();
            }
            else {
                m_data = new SharedPtrDynamicData(
                    1, /// strong
                    0, /// weak
                    true, /// valid
                    SR_UTILS_NS::SharedPtrPolicy::Automatic /// policy
                );
                m_data->InitBasic<T>();
            }
        }
        else {
            m_data = new SharedPtrDynamicData(
                1, /// strong
                0, /// weak
                true, /// valid
                SR_UTILS_NS::SharedPtrPolicy::Automatic /// policy
            );
            m_data->InitBasic<T>();
        }

        return *this;
    }

    template<class T> SRClass* SharedPtr<T>::GetSRClass() const {
        return (m_data && m_data->classGetter) ? m_data->classGetter(m_ptr) : nullptr;
    }

    template<class T> void SharedPtr<T>::DecrementPointer() {
        SharedPtrDynamicData* pData = m_data;

        if (!pData) {
            return;
        }

        SR_SAFE_PTR_ASSERT(pData->strongCount != 0, "SharedPtr is corrupted!");

        if (pData->strongCount == 1) {
            if (pData->policy == SR_UTILS_NS::SharedPtrPolicy::Manually) {
                SR_SAFE_PTR_ASSERT(pData->deallocated, "Ptr was not freed!");
            }
            else if (pData->policy == SR_UTILS_NS::SharedPtrPolicy::Automatic && pData->valid) {
                pData->valid = false;
                T* pPtr = m_ptr;
                m_ptr = nullptr;
                m_data = nullptr;
                pData->deleter(pPtr);
            }

            if (pData->weakCount == 0) {
                delete pData;
            }
        }
        else {
            pData->DecrementStrong();
        }
    }

    template<class T> void SharedPtr<T>::IncrementPointer() {
        if (m_data) {
            m_data->IncrementStrong();
        }
    }

    template<class T> template<class Y> WeakPtr<Y> SharedPtr<T>::GetWeakThis() const {
        WeakPtr<Y> weakPtr;
        if (m_data) {
            weakPtr.SetPtrData(m_data);
            weakPtr.GetPtrData()->IncrementWeak();
        }
        weakPtr.SetPtr(static_cast<Y*>(const_cast<T*>(m_ptr)));
        return weakPtr;
    }

    template<class T> void SharedPtr<T>::SetPointerFromBase(SharedPtrBase *pBase) {
        if constexpr (SR_UTILS_NS::IsCompleteTypeV<T>) {
            if constexpr (std::is_base_of_v<SRClass, T>) {
                Reset();
                if (pBase) {
                    m_data = pBase->GetPtrData();
                    m_data->IncrementStrong();
                    m_ptr = dynamic_cast<T*>(pBase);
                    SR_SAFE_PTR_ASSERT(m_ptr, "Invalid cast!");
                }
                return;
            }
        }
        SR_SAFE_PTR_ASSERT(false, "Incomplete or invalid type!");
    }

    template<class T> template<typename U, typename R, typename... Args> SharedPtr<R> SharedPtr<T>::MakeShared(Args&&... args) {
        auto&& pData = new U(std::forward<Args>(args)...);
        if constexpr (std::is_same_v<R, T>) {
            return pData->GetThis();
        }
        else {
            SR_STATIC_ASSERT2((std::is_base_of_v<T, R> || std::is_base_of_v<R, T>), "Invalid type!");
            auto&& pThis = pData->GetThis();
            return SharedPtr<R>(static_cast<R*>(pThis.Get()));
        }
    }

    template<typename T> bool SharedPtr<T>::AutoFree(const SR_HTYPES_NS::Function<void(T* pPtr, SharedPtrDynamicData* pControl)> &freeFun) {
        return Valid() && FreeImpl(freeFun);
    }

    template<typename T> bool SharedPtr<T>::AutoFree() {
        return Valid() && FreeImpl([](auto&& pPtr, SharedPtrDynamicData* pControl) { pControl->deleter(static_cast<void*>(pPtr)); });
    }

    template<typename T> bool SharedPtr<T>::FreeImpl(const SR_HTYPES_NS::Function<void(T* pPtr, SharedPtrDynamicData* pControl)> &freeFun) {
        if (m_data) {
            const bool valid = m_data->valid;
            const auto pPtr = m_ptr;
            SharedPtrDynamicData* pData = m_data;

            SR_MAYBE_UNUSED SharedPtr<T> pCopy = GetThis();
            Reset();

            if (valid) {
                pData->deallocated = true;
                freeFun(pPtr, pData);
                pData->valid = false;
                return true;
            }

            return false;
        }

        return false;
    }

    template<class T> void SharedPtr<T>::Reset() {
        if (m_basicManually) {
            SR_SAFE_PTR_ASSERT(false, "SharedPtr cannot reset itself!");
            return;
        }

        /// Делаем копию, так как в процессе удаления можем потярять this,
        /// а так же зануляем m_data, чтобы не войти в рекурсию
        SharedPtrDynamicData* pData = m_data;
        T* pPtr = m_ptr; /// тут может быть потенциально висячий указатель.
        m_data = nullptr;
        m_ptr = nullptr;

        if (!pData) {
            return;
        }

        const auto strongCount = pData->strongCount.load();

        SR_SAFE_PTR_ASSERT(strongCount != 0, "SharedPtr is corrupted!");

        if (strongCount == 1) {
            if (pData->policy == SR_UTILS_NS::SharedPtrPolicy::Manually) {
                SR_SAFE_PTR_ASSERT(pData->deallocated, "Ptr was not freed!");
            }
            else if (pData->policy == SR_UTILS_NS::SharedPtrPolicy::Automatic && pData->valid) {
                pData->valid = false;
                pData->deleter(static_cast<void*>(pPtr));
            }

            if (pData->weakCount == 0) {
                delete pData;
            }
        }
        else {
            pData->DecrementStrong();
        }
    }

    #pragma endregion
}

namespace SR_UTILS_NS {
    template<typename Y, typename T> SR_HTYPES_NS::SharedPtr<Y> SR_FORCE_INLINE DynamicPointerCast(const SR_HTYPES_NS::SharedPtr<T>& pPointer) {
        if constexpr (std::is_same_v<T, void>) {
            return SR_HTYPES_NS::SharedPtr<Y>();
        }

        if constexpr (!SR_UTILS_NS::IsCompleteTypeV<T> || !SR_UTILS_NS::IsCompleteTypeV<Y>) {
            SRHalt("DynamicPointerCast with incomplete type! Check includes!");
            return SR_HTYPES_NS::SharedPtr<Y>();
        }

        auto&& pData = pPointer.GetPtrData();
        if (pData && pData->valid) {
            return SR_HTYPES_NS::SharedPtr<Y>(dynamic_cast<Y*>(const_cast<T*>(pPointer.Get())));
        }

        return SR_HTYPES_NS::SharedPtr<Y>();
    }

    template<typename Y, typename T> SR_HTYPES_NS::SharedPtr<Y> SR_FORCE_INLINE StaticPointerCast(const SR_HTYPES_NS::SharedPtr<T>& pPointer) {
        if constexpr (std::is_same_v<T, void>) {
            return SR_HTYPES_NS::SharedPtr<Y>();
        }

        auto&& pData = pPointer.GetPtrData();
        if (pData && pData->valid) {
            return SR_HTYPES_NS::SharedPtr<Y>(static_cast<Y*>(const_cast<T*>(pPointer.Get())));
        }

        return SR_HTYPES_NS::SharedPtr<Y>();
    }

    template<typename Y, typename T> SR_HTYPES_NS::SharedPtr<Y> SR_FORCE_INLINE ConstPointerCast(const SR_HTYPES_NS::SharedPtr<T>& pPointer) {
        if constexpr (std::is_same_v<T, void>) {
            return SR_HTYPES_NS::SharedPtr<Y>();
        }

        auto&& pData = pPointer.GetPtrData();
        if (pData && pData->valid) {
            return SR_HTYPES_NS::SharedPtr<Y>(const_cast<Y*>(pPointer.Get()));
        }

        return SR_HTYPES_NS::SharedPtr<Y>();
    }

    template<typename Y, typename T> SR_HTYPES_NS::SharedPtr<Y> SR_FORCE_INLINE ReinterpretPointerCast(const SR_HTYPES_NS::SharedPtr<T>& pPointer) {
        if constexpr (std::is_same_v<T, void>) {
            return SR_HTYPES_NS::SharedPtr<Y>();
        }

        auto&& pData = pPointer.GetPtrData();
        if (pData && pData->valid) {
            return SR_HTYPES_NS::SharedPtr<Y>(reinterpret_cast<Y*>(const_cast<T*>(pPointer.Get())));
        }

        return SR_HTYPES_NS::SharedPtr<Y>();
    }

    template<typename Y, typename T> SR_HTYPES_NS::SharedPtr<Y> SR_FORCE_INLINE PolymorphicPointerCast(const SR_HTYPES_NS::SharedPtr<T>& pPointer) {
        if constexpr (std::is_same_v<T, void>) {
            return SR_HTYPES_NS::SharedPtr<Y>();
        }
        else {
        #ifdef SR_DEBUG
            if (!DynamicPointerCast<T, Y>(pPointer)) {
                SRHalt("Invalid cast! DynamicPointerCast returned nullptr!");
                return SR_HTYPES_NS::SharedPtr<Y>();
            }
        #endif
            return StaticPointerCast<T, Y>(pPointer);
        }
    }

    namespace SharedPointerTraits {
        template<typename T> struct IsSharedPointer : std::false_type { };
        template<typename T> struct IsSharedPointer<SR_HTYPES_NS::SharedPtr<T>> : std::true_type { };
    }

    template<class T>
    constexpr bool IsSharedPointerV = SharedPointerTraits::IsSharedPointer<RemoveQualifiersT<T>>::value;

    template<class T>
    constexpr bool IsBaseOfSharedPointer = std::is_base_of_v<SR_HTYPES_NS::SharedPtrBase, RemoveQualifiersT<T>>;

    template<typename T> struct InnerType<SR_HTYPES_NS::SharedPtr<T>> {
        using type = typename InnerType<T>::type;
    };
}

namespace SR_HTYPES_NS {
    template<class T> template<typename Y> SharedPtr<Y> SharedPtr<T>::DynamicCast() const {
        return SR_UTILS_NS::DynamicPointerCast<Y, T>(GetThis());
    }

    template<class T> template<typename Y> SharedPtr<Y> SharedPtr<T>::StaticCast() const {
        return SR_UTILS_NS::StaticPointerCast<Y, T>(GetThis());
    }
}

namespace std {
    template<typename T> struct hash<SR_HTYPES_NS::SharedPtr<T>> {
        size_t operator()(SR_HTYPES_NS::SharedPtr<T> const& ptr) const {
            return std::hash<const void*>()(ptr.GetRawPtr());
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
