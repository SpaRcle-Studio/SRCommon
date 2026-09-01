//
// Created by Monika on 07.02.2026.
//

#ifndef SR_ENGINE_WEAK_PTR_H
#define SR_ENGINE_WEAK_PTR_H

#include <Utils/Types/SharedPtr.h>

namespace SR_HTYPES_NS {
    template<class T> class WeakPtr {
    public:
        WeakPtr() = default;

        WeakPtr(const T* pData)
            : m_ptr(const_cast<T*>(pData))
        {
            if (m_ptr) {
                m_data = m_ptr->GetPtrData();
                m_data->IncrementWeak();
            }
        }

        WeakPtr(const SharedPtr<T>& ptr) {
            if ((m_data = ptr.m_data)) {
                m_data->IncrementWeak();
            }
            m_ptr = ptr.m_ptr;
        }

        WeakPtr(const WeakPtr<T>& ptr) {
            if ((m_data = ptr.m_data)) {
                m_data->IncrementWeak();
            }
            m_ptr = ptr.m_ptr;
        }

        WeakPtr(WeakPtr<T>&& ptr) noexcept
            : m_ptr(SR_UTILS_NS::Exchange(ptr.m_ptr, nullptr))
            , m_data(SR_UTILS_NS::Exchange(ptr.m_data, nullptr))
        { }

        WeakPtr<T>& operator=(const SharedPtr<T>& ptr) {
            if (m_data) {
                SR_SAFE_PTR_ASSERT(m_data->weakCount != 0, "WeakPtr is corrupted!");
                m_data->DecrementWeak();
                if (m_data->strongCount == 0 && m_data->weakCount == 0) {
                    delete m_data;
                }
            }

            m_ptr = ptr.m_ptr;

            if ((m_data = ptr.m_data)) {
                m_data->IncrementWeak();
            }

            return *this;
        }

        WeakPtr<T>& operator=(const T* ptr) {
            if (m_data) {
                SR_SAFE_PTR_ASSERT(m_data->weakCount != 0, "WeakPtr is corrupted!");
                m_data->DecrementWeak();
                if (m_data->strongCount == 0 && m_data->weakCount == 0) {
                    delete m_data;
                }
                m_data = nullptr;
            }

            m_ptr = const_cast<T*>(ptr);

            if (m_ptr) {
                m_data = m_ptr->GetPtrData();
                m_data->IncrementWeak();
            }

            return *this;
        }

        WeakPtr<T>& operator=(WeakPtr<T>&& ptr) noexcept {
            if (this == &ptr){
                return *this;
            }

            if (m_data) {
                SR_SAFE_PTR_ASSERT(m_data->weakCount != 0, "WeakPtr is corrupted!");
                m_data->DecrementWeak();
                if (m_data->strongCount == 0 && m_data->weakCount == 0) {
                    delete m_data;
                }
            }

            m_ptr = SR_UTILS_NS::Exchange(ptr.m_ptr, nullptr);
            m_data = SR_UTILS_NS::Exchange(ptr.m_data, nullptr);

            return *this;
        }

        ~WeakPtr() {
            if (m_data) {
                SR_SAFE_PTR_ASSERT(m_data->weakCount != 0, "WeakPtr is corrupted!");
                m_data->DecrementWeak();
                if (m_data->strongCount == 0 && m_data->weakCount == 0) {
                    delete m_data;
                }
            }
        }

        SR_NODISCARD operator bool() const noexcept {
            return m_ptr && m_data && m_data->valid;
        }

        WeakPtr<T>& operator=(const WeakPtr<T>& ptr) {
            if (this == &ptr){
                return *this;
            }

            if (m_data) {
                SR_SAFE_PTR_ASSERT(m_data->weakCount != 0, "WeakPtr is corrupted!");
                m_data->DecrementWeak();
                if (m_data->strongCount == 0 && m_data->weakCount == 0) {
                    delete m_data;
                }
            }

            m_ptr = ptr.m_ptr;

            if ((m_data = ptr.m_data)) {
                m_data->IncrementWeak();
            }

            return *this;
        }

        SR_NODISCARD bool IsExpired() const {
            return !m_data || m_data->strongCount == 0 || !m_data->valid;
        }

        SR_NODISCARD SharedPtr<T> Lock() const {
            if (m_data && m_data->valid) {
                SharedPtr<T> sharedPtr;
                sharedPtr.m_ptr = m_ptr;
                sharedPtr.m_data = m_data;
                sharedPtr.m_data->IncrementStrong();
                return sharedPtr;
            }
            return SharedPtr<T>();
        }

        SR_NODISCARD SharedPtrDynamicData* GetPtrData() const {
            return m_data;
        }

        void SetPtrData(SharedPtrDynamicData* data) {
            m_data = data;
        }

        void SetPtr(T* ptr) {
            m_ptr = ptr;
        }

        SR_NODISCARD T* GetUnchecked() const {
            return m_ptr;
        }

    private:
        T* m_ptr = nullptr;
        SharedPtrDynamicData* m_data = nullptr;

    };
}

#endif //SR_ENGINE_WEAK_PTR_H
