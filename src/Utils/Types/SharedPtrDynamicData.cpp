//
// Created by Monika on 07.02.2026.
//

#include <Utils/Types/SharedPtrDynamicData.h>
#include <Utils/Platform/Platform.h>

namespace SR_HTYPES_NS {
    bool SharedPtrDynamicDataCounter::CheckMemoryLeaks() {
        auto&& pointersCount = SR_HTYPES_NS::SharedPtrDynamicDataCounter::Instance().GetCount();
        if (pointersCount != 0) {
            SR_PLATFORM_NS::WriteConsoleError(SR_FORMAT("Memory leaks detected! Count: {}\n", pointersCount));
            for (auto&& pData : SR_HTYPES_NS::SharedPtrDynamicDataCounter::Instance().GetData()) {
                SR_PLATFORM_NS::WriteConsoleError(SR_FORMAT("Pointer with strong {} trace: {}\n", pData->GetStrongCount(), pData->GetDebugTrace().ToCStr()));
            }
            return false;
        }
        return true;
    }

    uint64_t SharedPtrDynamicDataCounter::GetCount() const {
        return m_count;
    }

    void SharedPtrDynamicDataCounter::Increment(SharedPtrDynamicData* pData) {
    #ifdef SR_SHARED_PTR_TRACE
        m_data.insert(pData);
    #endif
        ++m_count;
    }

    void SharedPtrDynamicDataCounter::Decrement(SharedPtrDynamicData* pData) {
    #ifdef SR_SHARED_PTR_TRACE
        m_data.erase(pData);
    #endif
        if (m_count == 0) {
            SRHalt("SharedPtrDynamicDataCounter::Decrement() : count underflow!");
            return;
        }
        --m_count;
    }

    bool SharedPtrDynamicDataCounter::IsSingletonCanBeDestroyed() const {
        return false;
    }

    const std::unordered_set<SharedPtrDynamicData *> &SharedPtrDynamicDataCounter::GetData() const {
        return m_data;
    }

    SharedPtrDynamicData::SharedPtrDynamicData(uint64_t strongCount, uint64_t weakCount, bool valid, SR_UTILS_NS::SharedPtrPolicy policy)
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

    SharedPtrDynamicData::~SharedPtrDynamicData() {
        SharedPtrDynamicDataCounter::Instance().Decrement(this);
    }

    SR_UTILS_NS::StringAtom SharedPtrDynamicData::GetDebugTrace() const {
    #ifdef SR_SHARED_PTR_TRACE
        return debugTrace;
    #else
        return SR_UTILS_NS::StringAtom();
    #endif
    }

    void SharedPtrDynamicData::IncrementStrong() {
        SRAssert2(strongCount != SR_UINT64_MAX, "Strong count overflow!");
        ++strongCount;
    }

    void SharedPtrDynamicData::DecrementStrong() {
        SRAssert2(strongCount != 0, "Strong count underflow!");
        --strongCount;
    }

    void SharedPtrDynamicData::IncrementWeak() {
        SRAssert2(weakCount != SR_UINT64_MAX, "Weak count overflow!");
        ++weakCount;
    }

    void SharedPtrDynamicData::DecrementWeak() {
        SRAssert2(weakCount != 0, "Weak count underflow!");
        --weakCount;
    }

    uint64_t SharedPtrDynamicData::GetStrongCount() const {
        return strongCount;
    }

    /// ----------------------------------------------------------------------------------------------------------------

    SharedPtrBase::SharedPtrBase() = default;

    SharedPtrBase::SharedPtrBase(SharedPtrDynamicData* data)
        : m_data(data)
    { }

    SharedPtrBase::~SharedPtrBase() = default;

    SharedPtrDynamicData* SharedPtrBase::GetPtrData() {
        return m_data;
    }

    const SharedPtrDynamicData* SharedPtrBase::GetPtrData() const {
        return m_data;
    }

    uint64_t SharedPtrBase::GetStrongCount() const {
        return m_data ? m_data->strongCount.load() : 0;
    }
}