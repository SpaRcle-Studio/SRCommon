//
// Created by mantsurov-n on 04.12.2021.
//

#include <Utils/World/Observer.h>

#define SR_WORLD_OFFSET(result, offset)                                           \
    if (offset != 0) {                                                            \
        if ((offset >= result && result > 0) || (offset <= result && result < 0)) \
            result += static_cast<SR_MATH_NS::Unit>(result) / abs(result);        \
        else if (result == 0)                                                     \
            result += static_cast<SR_MATH_NS::Unit>(offset) / abs(offset);        \
    }                                                                             \

#define SR_WORLD_REGION_NEIGHBOUR(chunk, region, width, offset, value) \
    if (offset.value >= 0)                                             \
        region.value = (chunk.value - 1) + offset.value;               \
    else                                                               \
        region.value = (chunk.value - width) + offset.value;           \

namespace SR_WORLD_NS {
    Observer::Observer(const Observer::ScenePtr& scene)
        : m_regionWidth(0)
        , m_shiftDistance(0)
        , m_scope(0)
        , m_offset(Offset())
        , m_scene(scene)
    { }

    SR_MATH_NS::FVector3 AddOffset(const SR_MATH_NS::FVector3 &chunk, const SR_MATH_NS::FVector3 &offset) {
        auto result = chunk + offset;

        SR_WORLD_OFFSET(result.x, offset.x);
        SR_WORLD_OFFSET(result.y, offset.y);
        SR_WORLD_OFFSET(result.z, offset.z);

        return result;
    }

    SR_MATH_NS::IVector3 AddOffset(const SR_MATH_NS::IVector3 &region, const SR_MATH_NS::IVector3 &offset) {
        return AddOffset(region.Cast<Math::Unit>(), offset).Cast<int32_t>();
    }

    SR_MATH_NS::FVector3 AddOffset(const SR_MATH_NS::FVector3 &region, const SR_MATH_NS::IVector3 &offset) {
        SRAssertOnce(!region.HasZero());

        auto result = region + offset;

        SR_WORLD_OFFSET(result.x, offset.x);
        SR_WORLD_OFFSET(result.y, offset.y);
        SR_WORLD_OFFSET(result.z, offset.z);

        return result;
    }

    SR_MATH_NS::Unit AddOffset(const SR_MATH_NS::Unit &value, const SR_MATH_NS::Unit &offset) {
        if (value == 0 && offset == 0)
            return 0;

        auto result = value + offset;

        SR_WORLD_OFFSET(result, offset);

        return result;
    }

    SR_MATH_NS::IVector3 MakeChunk(const SR_MATH_NS::IVector3 &rawChunkPos, int32_t width) {
        Math::IVector3 chunk = rawChunkPos;

        #define SR_MAKE_CHUNK(value, width, chunk) {                                              \
            SR_MATH_NS::IVector3 temp = chunk;                                                    \
            if (abs(chunk.value) > width || chunk.value < -width)                                 \
                chunk.value %= width;                                                             \
            if (chunk.value == 0)                                                                 \
                chunk.value = temp.value > 0 ? width : 1;                                         \
            else                                                                                  \
                chunk.value = temp.value > 0 ? chunk.value : width - (abs(chunk.value) - 1);      \
        }                                                                                         \

        SR_MAKE_CHUNK(x, width, chunk);
        SR_MAKE_CHUNK(y, width, chunk);
        SR_MAKE_CHUNK(z, width, chunk);

        SRAssert(abs(chunk.x) <= width && abs(chunk.y) <= width && abs(chunk.z) <= width);

        return chunk;
    }

    void Observer::SetChunk(SR_MATH_NS::IVector3 chunk) {
        m_chunk = MakeChunk(chunk, m_regionWidth);
        SRAssert(!m_chunk.HasZero());
    }

    void Observer::MoveRegion(const SR_MATH_NS::IVector3 &value) {
        m_region += value;

        if (m_region.x == 0)
            m_region.x = -(value.x / abs(value.x));

        if (m_region.y == 0)
            m_region.y = -(value.y / abs(value.y));

        if (m_region.z == 0)
            m_region.z = -(value.z / abs(value.z));
    }

    void Observer::SetWorldMetrics(const SR_MATH_NS::IVector2 &chunkSize, int32_t regionWidth) {
        m_chunkSize = chunkSize;
        m_regionWidth = regionWidth;
    }

    Offset Observer::MathNeighbour(const SR_MATH_NS::IVector3 &offset) {
        SRAssert1Once(!m_chunk.HasZero());

        Math::IVector3 region;

        SR_WORLD_REGION_NEIGHBOUR(m_chunk, region, m_regionWidth, offset, x);
        SR_WORLD_REGION_NEIGHBOUR(m_chunk, region, m_regionWidth, offset, y);
        SR_WORLD_REGION_NEIGHBOUR(m_chunk, region, m_regionWidth, offset, z);

        region /= m_regionWidth;

        const auto chunk = AddOffset(m_chunk, offset);

        return Offset(
                AddOffset(m_region, region),
                MakeChunk(chunk, m_regionWidth)
        );
    }

    Math::IVector3 Observer::WorldPosToChunkPos(const Math::FVector3& position) {
        const auto chunkSize = Math::IVector3(m_chunkSize.x, m_chunkSize.y, m_chunkSize.x);

        return AddOffset(
                position.Singular(Math::FVector3(m_chunkSize.x, m_chunkSize.y, m_chunkSize.x)).Cast<int>() / chunkSize,
                m_offset.m_chunk
        );
    }

    void Observer::SetTarget(const Observer::GameObjectPtr &target) {
        m_target = target;

        /// сохраняем последние значения

        ///m_chunk = SR_MATH_NS::IVector3();
        ///m_region = SR_MATH_NS::IVector3();

        /// m_lastChunk = SR_MATH_NS::IVector3();
        /// m_lastRegion = SR_MATH_NS::IVector3();

        m_targetPosition = SR_MATH_NS::FVector3();

        m_offset = Offset();
    }
}
