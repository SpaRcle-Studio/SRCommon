//
// Created by Monika on 18.03.2024.
//

#include <Utils/Types/SharedPtr.h>
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

    SharedPtrBase::SharedPtrBase() = default;

    SharedPtrBase::SharedPtrBase(SharedPtrDynamicData* data)
        : m_data(data)
    { }

    SharedPtrBase::~SharedPtrBase() = default;

    SharedPtrDynamicData *SharedPtrBase::GetPtrData() {
        return m_data;
    }

    const SharedPtrDynamicData *SharedPtrBase::GetPtrData() const {
        return m_data;
    }
}