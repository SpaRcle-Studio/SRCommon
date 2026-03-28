//
// Created by innerviewer on 10/26/2023.
//

#include <Utils/Common/Vertices.h>
#include <Utils/Debug.h>

// #include <Hull/Coordinate.h>
// #include <QuickHull/FastQuickHull.h>

#ifdef SR_COMMON_MESHOPTIMIZER
    #include <meshoptimizer.h>
#endif

namespace SR_UTILS_NS {
    namespace Details {
        bool OptimizeVerticesImpl(
            const void* vertices,
            uint64_t vertexSize,
            uint64_t verticesCount,
            const SR_HTYPES_NS::FastMemoryArray<uint32_t>& indices,
            uint64_t targetIndicesCount,
            float_t targetError,
            SR_HTYPES_NS::FastMemoryArray<uint32_t>& outSimplifiedIndices
        ) {
            SR_TRACY_ZONE;

            outSimplifiedIndices.resize(indices.size());

        #ifdef SR_COMMON_MESHOPTIMIZER
            float resultError = 0.0f;
            uint32_t options = 0;

            size_t resultCount = meshopt_simplify(
                outSimplifiedIndices.data(),
                indices.data(),
                indices.size(),
                (float*)vertices,
                verticesCount,
                vertexSize,
                targetIndicesCount,
                targetError,
                options,
                &resultError
            );

            if (resultCount == 0) {
                SR_ERROR("SR_UTILS_NS::Details::OptimizeVerticesImpl() : mesh simplification failed!");
                outSimplifiedIndices.clear();
                return false;
            }

            outSimplifiedIndices.resize(resultCount);
            return true;
        #else
            memcpy(outSimplifiedIndices.data(), indices.data(), indices.size() * sizeof(uint32_t));
            return true;
        #endif
        }
    }

    std::vector<Vertex> ComputeConvexHull(const std::vector<Vertex>& vertices) {
        /*std::vector<hull::Coordinate> coordinates;
        coordinates.reserve(vertices.size());

        for (auto&& vertex : vertices) {
            coordinates.emplace_back(hull::Coordinate{vertex.position.x, vertex.position.y, vertex.position.z});
        }

        auto&& hullCoordinates = qh::convex_hull(coordinates);

        std::vector<Vertex> hullVertices;
        hullVertices.reserve(hullCoordinates.size() * 3);

        for (uint16_t i = 0; i <= hullCoordinates.size(); ++i) {
            auto&& face = hullCoordinates[i];

            hullVertices.emplace_back(vertices[face[0]]);
            hullVertices.emplace_back(vertices[face[1]]);
            hullVertices.emplace_back(vertices[face[2]]);
        }

        return hullVertices;*/
        return std::vector<Vertex>();
    }
}
