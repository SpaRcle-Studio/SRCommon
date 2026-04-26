//
// Created by Monika on 24.03.2022.
//

#ifndef SR_ENGINE_RAWMESH_H
#define SR_ENGINE_RAWMESH_H

#include <Utils/Resources/IResource.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Types/Map.h>
#include <Utils/Common/Vertices.h>
#include <Utils/Math/Matrix4x4.h>
#include <Utils/Types/FastMemoryArray.h>
#include <Utils/Types/FlatHashMap.h>

#ifdef SR_UTILS_ASSIMP
namespace Assimp {
    class Importer;
}

class aiScene;
class aiAnimation;
class aiMesh;
#endif

namespace SR_WORLD_NS {
    class Scene;
}

namespace SR_UTILS_NS {
    class MappedFile;
}

namespace SR_HTYPES_NS {
    struct RawMeshParams : public SR_UTILS_NS::IResourceVariant {
        bool animation = false;
        bool convexHull = false;

        bool operator==(const RawMeshParams& rhs) const;

        SR_NODISCARD SRHashType GetHash() const override;

    };

    /// @extension(fbx, obj, gltf, gltf2, dae, 3ds, blend, mmd)
    class SR_COMMON_DLL_API RawMesh : public IResource {
        SR_CLASS()
        using Super = IResource;
        using ScenePtr = SR_HTYPES_NS::SharedPtr<SR_WORLD_NS::Scene>;
        using Hash = uint64_t;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<RawMesh>;

    public:
        RawMesh();
        ~RawMesh() override;

    public:
        void ComputeConvexHull();

        void SetVariant(const SR_UTILS_NS::IResourceVariant& variant) override;

        SR_NODISCARD uint32_t GetMeshesCount() const;
        SR_NODISCARD std::string_view GetGeometryName(uint32_t id) const;

        SR_NODISCARD const SR_UTILS_NS::VertexDataBuffer& GetVertexBuffer(uint32_t id, const SR_UTILS_NS::VertexLayoutDescription& layout) const;
        //SR_NODISCARD std::vector<SR_UTILS_NS::Vertex> GetVertices(uint32_t id) const;
        SR_NODISCARD const SR_HTYPES_NS::FastMemoryArray<uint32_t>& GetIndices(uint32_t id) const;
        SR_NODISCARD const SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, uint32_t>& GetBones(uint32_t id) const;
        SR_NODISCARD const SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, uint16_t>& GetOptimizedBones() const;
        SR_NODISCARD const SR_MATH_NS::Matrix4x4& GetBoneOffset(SR_UTILS_NS::StringAtom name) const;
        SR_NODISCARD const SR_MATH_NS::Matrix4x4& GetBoneTransform(uint32_t index) const;
        SR_NODISCARD const SR_MATH_NS::Matrix4x4& GetBoneTransform(SR_UTILS_NS::StringAtom name) const;
        SR_NODISCARD uint32_t GetBoneIndex(SR_UTILS_NS::StringAtom name) const;
        SR_NODISCARD const std::vector<SR_MATH_NS::Matrix4x4>& GetBoneOffsets() const;
        SR_NODISCARD std::string_view GetRootBoneName() const;
        SR_NODISCARD bool HasBones(uint32_t id) const;

        SR_NODISCARD uint32_t GetVerticesCount(uint32_t id) const;
        SR_NODISCARD uint32_t GetIndicesCount(uint32_t id) const;
        SR_NODISCARD uint32_t GetAnimationsCount() const;
        SR_NODISCARD std::vector<SR_UTILS_NS::StringAtom> GetAnimationNames() const;
        SR_NODISCARD int32_t GetMeshId(SR_UTILS_NS::StringAtom name) const;

        SR_NODISCARD float_t GetScaleFactor() const;
        SR_NODISCARD SR_UTILS_NS::Path GetAssociatedPath() const override;

        SR_NODISCARD bool IsAllowedToRevive() const override;

    #ifdef SR_UTILS_ASSIMP
        SR_NODISCARD const void* GetAssimpScene() const noexcept;
    #endif

    protected:
        bool Unload() override;
        bool Load() override;

    private:
        void NormalizeWeights();
        void CalculateBones();
        void OptimizeSkeleton();
        void CalculateOffsets();
        void CalculateTransforms();
        void CalculateAnimations();

    #ifdef SR_UTILS_ASSIMP
        uint32_t NormalizeWeights(const aiMesh* pMesh);
    #endif

    private:
        mutable std::vector<std::vector<SR_UTILS_NS::VertexDataBuffer>> m_vertexBuffersCache;

        std::vector<SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, uint32_t>> m_bones;
        SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, uint16_t> m_optimizedBones;

        SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, SR_MATH_NS::Matrix4x4> m_boneOffsetsMap;
        SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, SR_MATH_NS::Matrix4x4> m_boneTransformsMap;

        std::vector<SR_MATH_NS::Matrix4x4> m_boneOffsets;
        std::vector<SR_MATH_NS::Matrix4x4> m_boneTransforms;

        mutable std::vector<SR_HTYPES_NS::FastMemoryArray<uint32_t>> m_indices;

        RawMeshParams m_params;

        bool m_fromCache = false;
        SR_HTYPES_NS::RawPointerHolder<SR_UTILS_NS::MappedFile> m_pMappedFileCache;

    #ifdef SR_UTILS_ASSIMP
        SR_HTYPES_NS::FlatHashMap<Hash, aiAnimation*> m_animations;
        const aiScene* m_scene = nullptr;
        Assimp::Importer* m_importer = nullptr;
    #endif

    };
}

template<> struct SR_UTILS_NS::SRHash<SR_HTYPES_NS::RawMeshParams> {
    size_t operator()(SR_HTYPES_NS::RawMeshParams const& params) const {
        std::size_t res = 0;

        std::hash<bool> hBool;

        res ^= hBool(params.animation) + 0x9e3779b9 + (res << 6u) + (res >> 2u);
        res ^= hBool(params.convexHull) + 0x9e3779b9 + (res << 6u) + (res >> 2u);

        return res;
    }
};

#endif //SR_ENGINE_RAWMESH_H
