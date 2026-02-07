//
// Created by Monika on 18.03.2024.
//

#include <Utils/Types/SharedPtr.h>
#include <Utils/Platform/Platform.h>

namespace SR_HTYPES_NS {
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

    uint64_t SharedPtrBase::GetStrongCount() const {
        return m_data ? m_data->strongCount.load() : 0;
    }
}