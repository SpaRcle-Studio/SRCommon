//
// Created by Monika on 25.03.2022.
//

#ifndef SR_ENGINE_SKYBOXCONSTANTS_H
#define SR_ENGINE_SKYBOXCONSTANTS_H

#include <Utils/Types/FastMemoryArray.h>

#define SR_MAX_VERTEX_ATTRIBUTES 48

#ifdef SR_UTILS_ASSIMP
    struct aiMesh;
#endif

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(VertexAttributeFormat, uint8_t,
        None,          // no data

        // Float
        Float32,       // 32-bit float
        Float16,       // 16-bit half float

        // Integer
        UInt32,
        Int32,
        UInt16,
        Int16,
        UInt8,
        Int8,

        // Normalized integer
        UNorm8,        // 0..1
        UNorm16,       // 0..1
        SNorm8,        // -1..1
        SNorm16,       // -1..1

        // Packed
        R10G10B10A2_UNorm, // 10bit RGB + 2bit A
        R11G11B10_Float    // HDR RGB 11+11+10
    )

    uint8_t GetVertexAttributeFormatSize(VertexAttributeFormat format);

    SR_ENUM_NS_CLASS_T(VertexAttribute, uint8_t,
        None                   = 0  ,  // none
        Position               = 1  ,  // vec3
        Normal                 = 2  ,  // vec3
        Tangent                = 3  ,  // vec4 (w = sign)
        UV0                    = 4  ,  // vec2
        UV1                    = 5  ,  // vec2
        UV2                    = 6  ,  // vec2
        UV3                    = 7  ,  // vec2
        UV4                    = 8  ,  // vec2
        UV5                    = 9  ,  // vec2
        UV6                    = 10 ,  // vec2
        UV7                    = 11 ,  // vec2
        BlendIndices           = 12 ,  // vec4 (x,y,z,w = bone indices)
        BlendWeights           = 13 ,  // vec4 (x,y,z,w = bone weights)
        BlendIndices2          = 14 ,  // vec4 (x,y,z,w = bone indices) extended
        BlendWeights2          = 15 ,  // vec4 (x,y,z,w = bone weights) extended
        Color0                 = 16 ,  // vec4
        Color1                 = 17 ,  // vec4
        Color2                 = 18 ,  // vec4
        Color3                 = 19 ,  // vec4
        Color4                 = 20 ,  // vec4
        Color5                 = 21 ,  // vec4
        Color6                 = 22 ,  // vec4
        Color7                 = 23 ,  // vec4
        MaterialID0            = 24 ,  // uint
        MaterialID1            = 25 ,  // uint
        MaterialID2            = 26 ,  // uint
        MaterialID3            = 27 ,  // uint
        MaterialID4            = 28 ,  // uint
        MaterialID5            = 29 ,  // uint
        MaterialID6            = 30 ,  // uint
        MaterialID7            = 31 ,  // uint
        BlendFactor            = 32 ,  // float
        Custom0                = 33 ,  // vec4
        Custom1                = 34 ,  // vec4
        Custom2                = 35 ,  // vec4
        Custom3                = 36 ,  // vec4
        Custom4                = 37 ,  // vec4
        Custom5                = 38 ,  // vec4
        Custom6                = 39 ,  // vec4
        Custom7                = 40    // vec4
    );

    VertexAttribute GetVertexAttributeByIndex(VertexAttribute attribute, uint8_t index);

    struct VertexAttributeDescription {
        VertexAttribute attribute = VertexAttribute::None;
        VertexAttributeFormat format = VertexAttributeFormat::None;
        uint8_t count = 0;
        uint16_t offset = 0;

        SR_NODISCARD uint8_t GetAttributeSizeInBytes() const;
    };

    std::string_view VertexAttributeToName(VertexAttribute attribute);

    struct VertexLayoutDescription {
        VertexAttributeDescription attributes[SR_MAX_VERTEX_ATTRIBUTES] = {};
        uint8_t attributesCount = 0;

        mutable uint64_t strideCache = 0;

        SR_NODISCARD bool Compare(const VertexLayoutDescription& other) const;

        SR_NODISCARD uint64_t GetHash() const;
        SR_NODISCARD uint64_t GetStride() const;
        SR_NODISCARD const VertexAttributeDescription* Find(VertexAttribute attribute) const;

        VertexLayoutDescription& AddAttribute(VertexAttribute attribute, VertexAttributeFormat format, uint8_t count);
    };

    constexpr uint64_t VERTEX_LAYOUT_DESCRIPTION_SIZE = sizeof(VertexLayoutDescription);

    struct VertexDataBuffer : public NonCopyable {
        using Ptr = SR_HTYPES_NS::RawPointerHolder<VertexDataBuffer>;

        VertexDataBuffer() = default;
        VertexDataBuffer(VertexDataBuffer&& other) noexcept;
        VertexDataBuffer& operator=(VertexDataBuffer&& other) noexcept;

        SR_HTYPES_NS::FastMemoryArray<uint8_t> data;
        VertexLayoutDescription layout;
        uint64_t vertexCount = 0;

        void Allocate(uint64_t vertices);
        void SetVertex(uint64_t index, VertexAttribute attribute, const void* pSrc);
        void CopyFrom(const VertexDataBuffer& other);
        void SetLayout(const VertexLayoutDescription& newLayout) { layout = newLayout; }

        SR_NODISCARD const VertexLayoutDescription& GetLayout() const { return layout; }
        SR_NODISCARD void* GetVertex(uint64_t index, VertexAttribute attribute);
        SR_NODISCARD uint8_t* GetAttributeData(VertexAttribute attribute);
        SR_NODISCARD VertexDataBuffer TransitionToLayout(const VertexLayoutDescription& newLayout) const;
        SR_NODISCARD const void* GetRawData() const { return data.data(); }
        SR_NODISCARD uint64_t GetDataSize() const { return data.size(); }
        SR_NODISCARD uint64_t GetVertexCount() const { return vertexCount; }

#ifdef SR_UTILS_ASSIMP
        SR_NODISCARD static VertexDataBuffer AllocateFromAssimp(aiMesh* pMesh, const SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, uint32_t>& bones);
#endif

    };

    struct Vec2 {
        float_t x, y;
    };

    struct Vec3 {
        float_t x, y, z;
    };

    struct Vertex {
        Vertex() = default;

        Vertex(int32_t x, int32_t y, int32_t z)
            : position(Vec3 { static_cast<float_t>(x), static_cast<float_t>(y), static_cast<float_t>(z) })
        { }

        Vertex(Vec3 pos, Vec2 uv, Vec3 norm, Vec3 tang, Vec3 btg)
            : position(pos)
            , uv(uv)
            , normal(norm)
            , tangent(tang)
            , bitangent(btg)
        { }

        Vec3 position;
        Vec2 uv;
        Vec3 normal;
        Vec3 tangent;
        Vec3 bitangent;
        uint8_t weightsNum = 0;
        struct {
            uint32_t boneId = 0;
            float weight = 0.f;
        } weights[SR_MAX_BONES_ON_VERTEX];
    };

    const std::vector<uint32_t> SKYBOX_INDICES = { 0,1,2,0,2,3,2,1,4,5,2,4,6,0,3,6,3,7,5,4,6,7,5,6,3,2,5,3,5,7,4,1,0,6,4,0 };
    const std::vector<Vertex> SKYBOX_INDEXED_VERTICES = {
            Vertex(1, -1, 1),
            Vertex(-1, -1, 1),
            Vertex(-1, 1, 1),
            Vertex(1, 1, 1),
            Vertex(-1, -1, -1),
            Vertex(-1, 1, -1),
            Vertex(1, -1, -1),
            Vertex(1, 1, -1),
    };

    std::vector<Vertex> ComputeConvexHull(const std::vector<Vertex>& vertices);

    namespace Details {
        bool OptimizeVerticesImpl(
            const void* vertices,
            uint64_t vertexSize,
            uint64_t verticesCount,
            const SR_HTYPES_NS::FastMemoryArray<uint32_t>& indices,
            uint64_t targetIndicesCount,
            float_t targetError,
            SR_HTYPES_NS::FastMemoryArray<uint32_t>& outSimplifiedIndices
        );
    }

    template<typename T> bool OptimizeVertices(
        const SR_HTYPES_NS::FastMemoryArray<T>& vertices,
        const SR_HTYPES_NS::FastMemoryArray<uint32_t>& indices,
        uint64_t targetIndicesCount,
        float_t targetError,
        SR_HTYPES_NS::FastMemoryArray<uint32_t>& outSimplifiedIndices
    ) {
        SR_TRACY_ZONE;
        return Details::OptimizeVerticesImpl(
            vertices.data(),
            sizeof(T),
            vertices.size(),
            indices,
            targetIndicesCount,
            targetError,
            outSimplifiedIndices
        );
    }

    template<typename T> static std::vector<T> IndexedVerticesToNonIndexed(
            const std::vector<T>& vertices,
            const std::vector<uint32_t>& indices)
    {
        auto rawVertices = std::vector<T>();

        for (const auto& index : indices)
            rawVertices.emplace_back(vertices[index]);

        return rawVertices;
    }
}

#endif //SR_ENGINE_SKYBOXCONSTANTS_H
