//
// Created by innerviewer on 2024-04-29.
//
#ifndef SRENGINE_CHUNKINFO_H
#define SRENGINE_CHUNKINFO_H

#include <Utils/Math/Vector3.h>

namespace SR_WORLD_NS {
    class Observer;
    class Region;

    SR_ENUM_NS_CLASS_T(ChunkDimensionType, uint8_t,
            Square,
            Cube,
            Hexagon
    )

    struct ChunkDimensionInfo {
        ChunkDimensionType type {};

        union Dimension {
            SR_MATH_NS::IVector2 square;
            SR_MATH_NS::IVector3 cube;
            float_t hexagon {};
        } dimension;
    };

    struct ChunkCreateInfo {
        SR_WORLD_NS::Observer* observer = nullptr;
        SR_WORLD_NS::Region* region = nullptr;
        SR_MATH_NS::IVector3 position;
        ChunkDimensionInfo dimensionInfo;
    };
}

#endif //SRENGINE_CHUNKINFO_H
