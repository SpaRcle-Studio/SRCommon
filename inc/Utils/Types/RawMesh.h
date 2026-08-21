//
// Created by Monika on 24.03.2022.
//

#ifndef SR_ENGINE_COMMON_RAW_MESH_H
#define SR_ENGINE_COMMON_RAW_MESH_H

#include <Utils/Resources/IResource.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Types/MeshSceneStructure.h>
#include <Utils/Types/Map.h>

namespace SR_WORLD_NS {
    class Scene;
}

namespace SR_UTILS_NS {
    class MappedFile;
}

namespace SR_HTYPES_NS {
    struct SR_COMMON_DLL_API RawMeshParams : public IResourceVariant {
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
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<RawMesh>;

    public:
        RawMesh();
        ~RawMesh() override;

    public:
        void ComputeConvexHull();

        void SetVariant(const IResourceVariant& variant) override;

        SR_NODISCARD uint32_t GetMeshesCount() const;
        SR_NODISCARD StringView GetGeometryName(uint32_t id) const;

        SR_NODISCARD const VertexDataBuffer& GetVertexBuffer(uint32_t id, const VertexLayoutDescription& layout) const;
        SR_NODISCARD const SR_HTYPES_NS::FastMemoryArray<uint32_t>& GetIndices(uint32_t id) const;
        SR_NODISCARD const Vector<SR_MATH_NS::Matrix4x4>& GetBoneOffsetMatrices(uint32_t id) const;
        SR_NODISCARD const MeshSceneStructure::MeshData& GetMeshData(uint32_t id) const;
        SR_NODISCARD const MeshSceneStructure::BoneInfo& GetBoneInfo(uint32_t id, StringAtom name) const;
        SR_NODISCARD const MeshSceneStructure& GetSceneStructure() const;
        SR_NODISCARD bool HasBones(uint32_t id) const;

        SR_NODISCARD uint32_t GetVerticesCount(uint32_t id) const;
        SR_NODISCARD uint32_t GetIndicesCount(uint32_t id) const;
        SR_NODISCARD uint32_t GetAnimationsCount() const;
        SR_NODISCARD Vector<StringAtom> GetAnimationNames() const;
        SR_NODISCARD int32_t GetMeshId(StringAtom name) const;

        SR_NODISCARD float_t GetScaleFactor() const;
        SR_NODISCARD Path GetAssociatedPath() const override;

        SR_NODISCARD bool IsAllowedToRevive() const override;

    #ifdef SR_UTILS_ASSIMP
        SR_NODISCARD const void* GetAssimpScene() const noexcept;
    #endif

    protected:
        bool Unload() override;
        bool Load() override;

    private:
        RawPointerHolder<MappedFile> m_pMappedFileCache;
        mutable MeshSceneStructure m_sceneStructure;
        RawMeshParams m_params;
        bool m_fromCache = false;

    #ifdef SR_UTILS_ASSIMP
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

#endif //SR_ENGINE_COMMON_RAW_MESH_H
