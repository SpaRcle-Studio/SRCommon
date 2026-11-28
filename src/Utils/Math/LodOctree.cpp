//
// Created by Monika on 08.10.2026.
//

#include <Utils/Math/LodOctree.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_MATH_NS {
    FVector3 OctreeNodeId::GetCenter() const noexcept {
        return GetMin() + FVector3(static_cast<float_t>(GetSize()) * 0.5f);
    }

    float_t OctreeNodeId::DistanceTo(const FVector3& point) const noexcept {
        const FVector3 min = GetMin();
        const FVector3 max = GetMax();
        const float_t dx = std::max({ min.x - point.x, point.x - max.x, 0.f });
        const float_t dy = std::max({ min.y - point.y, point.y - max.y, 0.f });
        const float_t dz = std::max({ min.z - point.z, point.z - max.z, 0.f });
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    bool OctreeNodeId::Intersects(const OctreeNodeId& other) const noexcept {
        const IVector3 maxA = position + IVector3(GetSize());
        const IVector3 maxB = other.position + IVector3(other.GetSize());
        return position.x < maxB.x && other.position.x < maxA.x &&
               position.y < maxB.y && other.position.y < maxA.y &&
               position.z < maxB.z && other.position.z < maxA.z;
    }

    OctreeNodeId OctreeNodeId::GetAncestor(uint8_t ancestorLevel) const noexcept {
        if (ancestorLevel <= level) {
            return *this;
        }

        /// Размер - степень двойки, поэтому сброс младших бит округляет вниз и для отрицательных координат
        const int32_t mask = ~((1 << ancestorLevel) - 1);

        OctreeNodeId ancestor;
        ancestor.level = ancestorLevel;
        ancestor.position = IVector3(position.x & mask, position.y & mask, position.z & mask);
        return ancestor;
    }

    void OctreeNodeSet::Clear() {
        m_nodes.clear();
        m_cover.clear();
    }

    void OctreeNodeSet::Add(const OctreeNodeId& node, uint8_t maxLevel) {
        m_nodes.emplace(node);

        for (uint8_t ancestorLevel = node.level; ancestorLevel <= maxLevel; ++ancestorLevel) {
            /// Предок уже есть - значит, вся цепочка выше тоже добавлена
            if (!m_cover.emplace(node.GetAncestor(ancestorLevel)).second) {
                break;
            }
        }
    }

    bool OctreeNodeSet::Intersects(const OctreeNodeId& node, uint8_t maxLevel) const {
        if (m_nodes.empty()) {
            return false;
        }

        /// Узел - предок добавленного или он сам
        if (m_cover.contains(node)) {
            return true;
        }

        return ContainsSelfOrAncestor(node, maxLevel);
    }

    bool OctreeNodeSet::ContainsSelfOrAncestor(const OctreeNodeId& node, uint8_t maxLevel) const {
        if (m_nodes.empty()) {
            return false;
        }

        for (uint8_t ancestorLevel = node.level; ancestorLevel <= maxLevel; ++ancestorLevel) {
            if (m_nodes.contains(node.GetAncestor(ancestorLevel))) {
                return true;
            }
        }

        return false;
    }

    void LodOctree::Build(const FVector3& observer, const Settings& settings, const FilterFn& filter) {
        SR_TRACY_ZONE;

        m_leaves.clear();

        const int32_t rootSize = 1 << settings.maxLevel;
        const IVector3 rootCoord(
            static_cast<int32_t>(std::floor(observer.x / static_cast<float_t>(rootSize))),
            static_cast<int32_t>(std::floor(observer.y / static_cast<float_t>(rootSize))),
            static_cast<int32_t>(std::floor(observer.z / static_cast<float_t>(rootSize)))
        );

        for (int32_t x = -settings.rootRadius.x; x <= settings.rootRadius.x; ++x) {
            for (int32_t y = -settings.rootRadius.y; y <= settings.rootRadius.y; ++y) {
                for (int32_t z = -settings.rootRadius.z; z <= settings.rootRadius.z; ++z) {
                    OctreeNodeId root;
                    root.level = settings.maxLevel;
                    root.position = (rootCoord + IVector3(x, y, z)) * rootSize;
                    Subdivide(root, observer, settings, filter);
                }
            }
        }
    }

    void LodOctree::Subdivide(const OctreeNodeId& node, const FVector3& observer, const Settings& settings, const FilterFn& filter) {
        if (filter && !filter(node)) {
            return;
        }

        const float_t size = static_cast<float_t>(node.GetSize());
        if (node.level == 0 || node.DistanceTo(observer) >= size * settings.splitDistance) {
            m_leaves.emplace_back(node);
            return;
        }

        const int32_t childSize = node.GetSize() / 2;
        for (int32_t i = 0; i < 8; ++i) {
            OctreeNodeId child;
            child.level = node.level - 1;
            child.position = node.position + IVector3((i & 1) ? childSize : 0, (i & 2) ? childSize : 0, (i & 4) ? childSize : 0);
            Subdivide(child, observer, settings, filter);
        }
    }
}
