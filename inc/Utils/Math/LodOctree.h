//
// Created by Monika on 08.10.2026.
//

#ifndef SR_ENGINE_UTILS_MATH_LOD_OCTREE_H
#define SR_ENGINE_UTILS_MATH_LOD_OCTREE_H

#include <Utils/Math/Vector3.h>
#include <Utils/Types/Function.h>
#include <Utils/Types/Vector.h>

namespace SR_MATH_NS {
    /// Узел октодерева. Координаты и размеры - в единицах узлов нулевого уровня.
    struct SR_COMMON_DLL_API OctreeNodeId {
        IVector3 position; /// минимальный угол узла
        uint8_t level = 0; /// размер узла = 2^level

        SR_NODISCARD int32_t GetSize() const noexcept { return 1 << level; }
        SR_NODISCARD FVector3 GetMin() const noexcept { return FVector3(position); }
        SR_NODISCARD FVector3 GetMax() const noexcept { return FVector3(position + IVector3(GetSize())); }
        SR_NODISCARD FVector3 GetCenter() const noexcept;
        /// Расстояние от точки до границ узла, 0 если точка внутри
        SR_NODISCARD float_t DistanceTo(const FVector3& point) const noexcept;
        SR_NODISCARD bool Intersects(const OctreeNodeId& other) const noexcept;

        SR_NODISCARD bool operator==(const OctreeNodeId& other) const noexcept { return level == other.level && position == other.position; }
        SR_NODISCARD bool operator!=(const OctreeNodeId& other) const noexcept { return !(*this == other); }
    };

    /// Октодерево уровней детализации. Корневые узлы уровня maxLevel раскладываются сеткой вокруг наблюдателя,
    /// узел делится на 8 детей, пока наблюдатель ближе чем size * splitDistance. Листья - итоговый набор чанков.
    /// Фильтр позволяет отбросить узлы без поверхности (например, вне сферической оболочки планеты).
    class SR_COMMON_DLL_API LodOctree {
    public:
        using FilterFn = SR_HTYPES_NS::Function<bool(const OctreeNodeId&)>;

        struct Settings {
            uint8_t maxLevel = 3;
            float_t splitDistance = 1.5f;
            IVector3 rootRadius = IVector3(1, 1, 1); /// сколько корневых узлов в каждую сторону от наблюдателя
        };

    public:
        /// observer - позиция наблюдателя в единицах узлов нулевого уровня
        void Build(const FVector3& observer, const Settings& settings, const FilterFn& filter);

        SR_NODISCARD const SR_UTILS_NS::Vector<OctreeNodeId>& GetLeaves() const noexcept { return m_leaves; }

    private:
        void Subdivide(const OctreeNodeId& node, const FVector3& observer, const Settings& settings, const FilterFn& filter);

    private:
        SR_UTILS_NS::Vector<OctreeNodeId> m_leaves;

    };
}

namespace std {
    template<> struct hash<SR_MATH_NS::OctreeNodeId> {
        size_t operator()(const SR_MATH_NS::OctreeNodeId& node) const {
            size_t res = std::hash<SR_MATH_NS::IVector3>()(node.position);
            hash_vector3_combine<uint8_t>(res, node.level);
            return res;
        }
    };
}

#endif //SR_ENGINE_UTILS_MATH_LOD_OCTREE_H
