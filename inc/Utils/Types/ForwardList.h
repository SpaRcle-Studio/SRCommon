//
// Created by Monika on 23.04.2024.
//

#ifndef SR_ENGINE_UTILS_FORWARD_LIST_H
#define SR_ENGINE_UTILS_FORWARD_LIST_H

#include <Utils/Platform/Platform.h>

namespace SR_HTYPES_NS {
    template<typename T> class ForwardList {
        struct Node {
            explicit Node(T&& dats)
                : data(std::move(dats))
            { }

            T data;
            Node* pData = nullptr;
        };

    public:
        ForwardList() = default;

    private:
        Node* m_head = nullptr;
        Node* m_tail = nullptr;
        uint64_t m_size = 0;

    };
}

#endif //SR_ENGINE_UTILS_FORWARD_LIST_H
