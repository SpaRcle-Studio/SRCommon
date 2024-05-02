//
// Created by innerviewer on 2024-05-02.
//

#include <Utils/World/ChunkSystem/Chunk.h>
#include <Utils/World/Region.h>

#include <Utils/ECS/GameObject.h>
#include <Utils/ECS/Transform.h>

namespace SR_WORLD_NS {
    Chunk::Chunk(ChunkCreateInfo createInfo) {
        //m_observer = createInfo.observer;
        m_region = createInfo.region;
        m_position = createInfo.position;
        m_dimensionInfo = createInfo.dimensionInfo;
    }

    bool Chunk::Belongs(ChunkDimensionInfo dimensionInfo, const SR_MATH_NS::IVector3& position, const SR_MATH_NS::FVector3& point) {
        switch (dimensionInfo.type) {
            case ChunkDimensionType::Cube: {
                const float_t xMax = position.x + dimensionInfo.dimension.cube.x;
                const float_t yMax = position.y + dimensionInfo.dimension.cube.y;
                const float_t zMax = position.z + dimensionInfo.dimension.cube.x;

                return point.x >= position.x && point.y >= position.y && point.z >= position.z &&
                       point.x <= xMax && point.y <= yMax && point.z <= zMax;
            }
            default:
                SR_PLATFORM_NS::Terminate();
                return false;
        }
    }

    bool Chunk::Belongs(const SR_MATH_NS::FVector3& point) {
        return Belongs(m_dimensionInfo, m_position, point);
    }

    bool Chunk::PreLoad(SR_HTYPES_NS::Marshal* pMarshal) {
        /// TODO: add version and migration

        if (pMarshal && pMarshal->Valid()) {
            if (m_position != pMarshal->Read<SR_MATH_NS::IVector3>()) {
                SRAssert2(false, "Something went wrong...");
                return false;
            }

            const uint64_t count = pMarshal->Read<uint64_t>();
            for (uint64_t i = 0; i < count; ++i) {
                if (auto&& ptr = GameObject::Load(*pMarshal, nullptr)) {
                    auto&& pTransform = ptr->GetTransform();

                    if (pTransform->GetMeasurement() == SR_UTILS_NS::Measurement::Space3D) {
                        pTransform->GlobalTranslate(GetWorldPosition());
                    }

                    m_preloaded.emplace_back(ptr);
                }
            }
        }

        m_loadState = LoadState::PreLoaded;

        return true;
    }

    SR_MATH_NS::FVector3 Chunk::GetWorldPosition(SR_MATH_NS::AxisFlag center) const {
        switch (m_dimensionInfo.type) {
            case ChunkDimensionType::Cube: {
                auto&& position = SR_UTILS_NS::World::AddOffset(
                        ((m_region->GetWorldPosition()) + (m_position - SR_MATH_NS::FVector3(1, 1, 1))).Cast<SR_MATH_NS::Unit>(),
                        m_observer->m_offset.m_chunk.Cast<SR_MATH_NS::Unit>()
                );

                position = SR_MATH_NS::FVector3(
                        position.x * m_dimensionInfo.dimension.cube.x + (center & SR_MATH_NS::Axis::X ? (SR_MATH_NS::Unit) m_dimensionInfo.dimension.cube.x / 2 : 0),
                        position.y * m_dimensionInfo.dimension.cube.y + (center & SR_MATH_NS::Axis::Y ? (SR_MATH_NS::Unit) m_dimensionInfo.dimension.cube.y / 2 : 0),
                        position.z * m_dimensionInfo.dimension.cube.x + (center & SR_MATH_NS::Axis::Z ? (SR_MATH_NS::Unit) m_dimensionInfo.dimension.cube.x / 2 : 0)
                );

                position = position.DeSingular(SR_MATH_NS::FVector3(m_dimensionInfo.dimension.cube));

                return position;
            }
            default:
                SR_PLATFORM_NS::Terminate();
                return false;
        }
        auto fPos = SR_UTILS_NS::World::AddOffset(
                ((m_region->GetWorldPosition()) + (m_position - SR_MATH_NS::FVector3(1, 1, 1))).Cast<SR_MATH_NS::Unit>(),
                m_observer->m_offset.m_chunk.Cast<SR_MATH_NS::Unit>()
        );

        fPos = SR_MATH_NS::FVector3(
                fPos.x * m_dimensionInfo.dimension.cube.x + (center & SR_MATH_NS::Axis::X ? (SR_MATH_NS::Unit) m_dimensionInfo.dimension.cube.x / 2 : 0),
                fPos.y * m_dimensionInfo.dimension.cube.y + (center & SR_MATH_NS::Axis::Y ? (SR_MATH_NS::Unit) m_dimensionInfo.dimension.cube.y / 2 : 0),
                fPos.z * m_dimensionInfo.dimension.cube.x + (center & SR_MATH_NS::Axis::Z ? (SR_MATH_NS::Unit) m_dimensionInfo.dimension.cube.x / 2 : 0)
        );

        fPos = fPos.DeSingular(SR_MATH_NS::FVector3(m_dimensionInfo.dimension.cube));
    }
}