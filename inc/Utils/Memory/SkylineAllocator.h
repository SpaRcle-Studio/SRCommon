//
// Created by Monika on 30.04.2026.
//

#ifndef SR_ENGINE_COMMON_SKYLINE_ALLOCATOR_H
#define SR_ENGINE_COMMON_SKYLINE_ALLOCATOR_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/Math/Rect.h>

namespace SR_UTILS_NS {
    class SkylineAllocator : public NonCopyable {
    public:
        SkylineAllocator() = default;
        SkylineAllocator(uint32_t width, uint32_t height)
            : m_width(width)
            , m_height(height)
        {
            m_nodes.push_back({0, 0, width});
        }

        SR_NODISCARD bool Allocate(uint32_t w, uint32_t h, SR_MATH_NS::USRect& out);
        SR_NODISCARD uint32_t GetNodesCount() const { return m_nodes.size(); }

    private:
        SR_NODISCARD int32_t Fit(uint32_t index, uint32_t w, uint32_t h);

        void AddLevel(uint32_t index, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
        void Merge();

    private:
        struct Node {
            uint32_t x = 0;
            uint32_t y = 0;
            uint32_t width = 0;
        };

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        std::vector<Node> m_nodes;
    };
}

#endif //SR_ENGINE_COMMON_SKYLINE_ALLOCATOR_H
