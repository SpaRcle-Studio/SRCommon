//
// Created by Monika on 19.07.2026.
//

#ifndef SR_ENGINE_COMMON_SHARED_PTR_BASE_HOLDER_H
#define SR_ENGINE_COMMON_SHARED_PTR_BASE_HOLDER_H

#include <Utils/stdInclude.h>

namespace SR_HTYPES_NS {
    class SharedPtrBase;
}

namespace SR_UTILS_NS {
    class SRClass;

    class SharedPtrBaseHolder {
    public:
        explicit SharedPtrBaseHolder(SR_HTYPES_NS::SharedPtrBase* pData);
        SharedPtrBaseHolder();
        SharedPtrBaseHolder(const SharedPtrBaseHolder& other);
        SharedPtrBaseHolder(SharedPtrBaseHolder&& other) noexcept;
        ~SharedPtrBaseHolder();

        SharedPtrBaseHolder& operator=(const SharedPtrBaseHolder& other);
        SharedPtrBaseHolder& operator=(SharedPtrBaseHolder&& other) noexcept;

    public:
        SR_NODISCARD SRClass* GetSRClass() const;

    private:
        void Increment();
        void Decrement();

    private:
        SR_HTYPES_NS::SharedPtrBase* m_pData = nullptr;

    };
}

#endif //SR_ENGINE_COMMON_SHARED_PTR_BASE_HOLDER_H
