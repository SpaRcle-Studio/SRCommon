//
// Created by Monika on 19.07.2026.
//

#include <Utils/Types/SharedPtrBaseHolder.h>

namespace SR_UTILS_NS {
    SharedPtrBaseHolder::SharedPtrBaseHolder(SR_HTYPES_NS::SharedPtrBase* pData)
        : m_pData(pData)
        , m_pControl(pData ? pData->GetPtrData() : nullptr)
    {
        Increment();
    }

    SharedPtrBaseHolder::SharedPtrBaseHolder() = default;

    SharedPtrBaseHolder::SharedPtrBaseHolder(const SharedPtrBaseHolder& other) {
        m_pData = other.m_pData;
        m_pControl = other.m_pControl;
        Increment();
    }

    SharedPtrBaseHolder::SharedPtrBaseHolder(SharedPtrBaseHolder&& other) noexcept {
        m_pData = other.m_pData;
        m_pControl = other.m_pControl;
        other.m_pData = nullptr;
        other.m_pControl = nullptr;
    }

    SharedPtrBaseHolder::~SharedPtrBaseHolder() {
        Decrement();
    }

    SharedPtrBaseHolder& SharedPtrBaseHolder::operator=(const SharedPtrBaseHolder& other) {
        if (this != &other) {
            Decrement();
            m_pData = other.m_pData;
            m_pControl = other.m_pControl;
            Increment();
        }
        return *this;
    }

    SharedPtrBaseHolder& SharedPtrBaseHolder::operator=(SharedPtrBaseHolder&& other) noexcept {
        if (this != &other) {
            Decrement();
            m_pData = other.m_pData;
            m_pControl = other.m_pControl;
            other.m_pData = nullptr;
            other.m_pControl = nullptr;
        }
        return *this;
    }

    void SharedPtrBaseHolder::Increment() {
        if (m_pControl)  {
            m_pControl->IncrementStrong();
        }
    }

    void SharedPtrBaseHolder::Decrement() {
        if (m_pControl) {
            m_pControl->DecrementStrong();
        }
    }

    SRClass* SharedPtrBaseHolder::GetSRClass() const {
        if (m_pControl && m_pControl->valid) {
            return m_pData->GetSRClass();
        }
        return nullptr;
    }
}