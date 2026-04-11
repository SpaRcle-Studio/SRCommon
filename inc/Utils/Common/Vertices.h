//
// Created by Monika on 25.03.2022.
//

#ifndef SR_ENGINE_COMMON_VERTICES_H
#define SR_ENGINE_COMMON_VERTICES_H

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
        None                   ,  // none
        Position               ,  // vec3
        Normal                 ,  // vec3
        Tangent                ,  // vec4 (w = sign)
        UV0                    ,  // vec2
        UV1                    ,  // vec2
        UV2                    ,  // vec2
        UV3                    ,  // vec2
        UV4                    ,  // vec2
        UV5                    ,  // vec2
        UV6                    ,  // vec2
        UV7                    ,  // vec2
        BlendIndices           ,  // vec4 (x,y,z,w = bone indices)
        BlendWeights           ,  // vec4 (x,y,z,w = bone weights)
        BlendIndices2          ,  // vec4 (x,y,z,w = bone indices) extended
        BlendWeights2          ,  // vec4 (x,y,z,w = bone weights) extended
        Color0                 ,  // vec4
        Color1                 ,  // vec4
        Color2                 ,  // vec4
        Color3                 ,  // vec4
        Color4                 ,  // vec4
        Color5                 ,  // vec4
        Color6                 ,  // vec4
        Color7                 ,  // vec4
        MaterialWeights        ,  // vec4 (x,y,z,w = material weights)
        MaterialIndices        ,  // vec4 (x,y,z,w = material indices)
        MaterialID0            ,  // uint
        MaterialID1            ,  // uint
        MaterialID2            ,  // uint
        MaterialID3            ,  // uint
        MaterialID4            ,  // uint
        MaterialID5            ,  // uint
        MaterialID6            ,  // uint
        MaterialID7            ,  // uint
        BlendFactor            ,  // float
        Custom0                ,  // vec4
        Custom1                ,  // vec4
        Custom2                ,  // vec4
        Custom3                ,  // vec4
        Custom4                ,  // vec4
        Custom5                ,  // vec4
        Custom6                ,  // vec4
        Custom7                   // vec4
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

    static constexpr std::array SKYBOX_INDICES = { 0,1,2,0,2,3,2,1,4,5,2,4,6,0,3,6,3,7,5,4,6,7,5,6,3,2,5,3,5,7,4,1,0,6,4,0 };
    static constexpr std::array SKYBOX_INDEXED_VERTICES = {
        SR_MATH_NS::FVector3(1, -1, 1),
        SR_MATH_NS::FVector3(-1, -1, 1),
        SR_MATH_NS::FVector3(-1, 1, 1),
        SR_MATH_NS::FVector3(1, 1, 1),
        SR_MATH_NS::FVector3(-1, -1, -1),
        SR_MATH_NS::FVector3(-1, 1, -1),
        SR_MATH_NS::FVector3(1, -1, -1),
        SR_MATH_NS::FVector3(1, 1, -1),
    };

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
}

#endif //SR_ENGINE_COMMON_VERTICES_H
