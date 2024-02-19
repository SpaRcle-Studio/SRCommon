//
// Created by Monika on 19.02.2024.
//

#ifndef SR_UTILS_TYPES_NODE_H
#define SR_UTILS_TYPES_NODE_H

#include <Utils/Common/NonCopyable.h>

namespace SR_HTYPES_NS {
    struct Node : public SR_UTILS_NS::NonCopyable {
    private:
        using Super = SR_UTILS_NS::NonCopyable;

    public:
        Node(void* pData, uint64_t size)
            : Super()
            , size(size)
        {
            this->pData = malloc(size);
            memcpy(this->pData, pData, size);
        }

        ~Node() override {
            if (pData != nullptr) {
                free(pData);
                pData = nullptr;
            }
        }

    public:
        void* pData = nullptr;
        Node* pNext = nullptr;
        Node* pPrev = nullptr;
        uint64_t size = 0;

    };
}

#endif //SR_UTILS_TYPES_NODE_H
