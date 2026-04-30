//
// Created by Monika on 30.04.2026.
//

#include <Utils/Memory/SkylineAllocator.h>

namespace SR_UTILS_NS {
    int32_t SkylineAllocator::Fit(uint32_t index, uint32_t w, uint32_t h) {
        uint32_t x = m_nodes[index].x;
        uint32_t y = m_nodes[index].y;
        int32_t widthLeft = static_cast<int32_t>(w);

        if (x + w > m_width) {
            return -1;
        }

        uint32_t i = index;

        while (widthLeft > 0) {
            y = std::max(y, m_nodes[i].y);
            if (y + h > m_height) {
                return -1;
            }

            widthLeft -= static_cast<int32_t>(m_nodes[i].width);
            ++i;

            if (i >= static_cast<uint32_t>(m_nodes.size()) && widthLeft > 0) {
                return -1;
            }
        }

        return static_cast<int32_t>(y);
    }

    bool SkylineAllocator::Allocate(uint32_t w, uint32_t h, Math::USRect& out) {
        if (m_nodes.empty()) {
            SRHalt("SkylineAllocator::Allocate() : allocator is not initialized!");
            return false;
        }

        int32_t bestY = std::numeric_limits<int32_t>::max();
        uint32_t bestX = 0;
        int32_t bestIndex = -1;

        for (uint32_t i = 0; i < static_cast<uint32_t>(m_nodes.size()); ++i) {
            int32_t y = Fit(i, w, h);
            if (y >= 0 && y + static_cast<int32_t>(h) < bestY) {
                bestY = y;
                bestIndex = static_cast<int32_t>(i);
                bestX = m_nodes[i].x;
            }
        }

        if (bestIndex == -1) {
            return false;
        }

        AddLevel(bestIndex, bestX, bestY, w, h);

        out = { (uint16_t)bestX, (uint16_t)bestY, (uint16_t)w, (uint16_t)h };
        return true;
    }

    void SkylineAllocator::Merge() {
        for (int64_t i = 0; i < static_cast<int64_t>(m_nodes.size()) - 1; ++i) {
            if (m_nodes[i].y == m_nodes[i + 1].y) {
                m_nodes[i].width += m_nodes[i + 1].width;
                m_nodes.erase(m_nodes.begin() + i + 1);
                --i;
            }
        }
    }

    void SkylineAllocator::AddLevel(uint32_t index, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
        Node newNode = { x, y + h, w };
        m_nodes.insert(m_nodes.begin() + index, newNode);

        for (uint32_t i = index + 1; i < static_cast<uint32_t>(m_nodes.size()); ++i) {
            Node& node = m_nodes[i];
            Node& prev = m_nodes[i - 1];

            const uint32_t prevEnd = prev.x + prev.width;
            if (node.x < prevEnd) {
                const int32_t shrink = static_cast<int32_t>(prev.x) + static_cast<int32_t>(prev.width) - static_cast<int32_t>(node.x);
                const int32_t newWidth = static_cast<int32_t>(node.width) - shrink;

                if (newWidth <= 0) {
                    m_nodes.erase(m_nodes.begin() + i);
                    --i;
                }
                else {
                    node.x = static_cast<uint32_t>(static_cast<int32_t>(node.x) + shrink);
                    node.width = static_cast<uint32_t>(newWidth);
                    break;
                }
            }
            else {
                break;
            }
        }

        Merge();
    }
}