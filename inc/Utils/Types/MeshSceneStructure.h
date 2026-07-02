//
// Created by Monika on 16.06.2026.
//

#ifndef SR_ENGINE_COMMON_MESH_SCENE_STRUCTURE_H
#define SR_ENGINE_COMMON_MESH_SCENE_STRUCTURE_H

#include <Utils/Types/Vector.h>
#include <Utils/Types/SmallVector.h>
#include <Utils/Math/Matrix4x4.h>
#include <Utils/Types/FastMemoryArray.h>
#include <Utils/Types/FlatHashMap.h>
#include <Utils/Common/Vertices.h>

#ifdef SR_UTILS_ASSIMP
namespace Assimp {
    class Importer;
}

struct aiScene;
struct aiNode;
struct aiAnimation;
struct aiMesh;
#endif

namespace SR_HTYPES_NS {
    struct MeshSceneStructure {
    #ifdef SR_UTILS_ASSIMP
        using AnimationMap = SR_HTYPES_NS::FlatHashMap<StringAtom, aiAnimation*>;
    #else
        using AnimationMap = SR_HTYPES_NS::FlatHashMap<StringAtom, void*>;
    #endif
        struct BoneInfo {
            std::optional<uint32_t> boneId;
            std::optional<uint16_t> nodeIndex;
            SR_MATH_NS::Matrix4x4 offsetMatrix;
        };
        using BonesMap = SR_HTYPES_NS::FlatHashMap<StringAtom, BoneInfo>;
        struct MeshData {
            uint16_t meshId = SR_UINT16_MAX;
            Vector<VertexDataBuffer> vertexBuffers;
            SR_HTYPES_NS::FastMemoryArray<uint32_t> indices;
            BonesMap bones;

            std::optional<uint32_t> maxBoneId;
            std::optional<uint32_t> materialId;
            mutable Vector<SR_MATH_NS::Matrix4x4> boneOffsetMatricesCache;

            SR_NODISCARD bool HasBones() const { return !bones.empty(); }
        };
        struct SceneNode {
            StringAtom name;
            uint16_t index = SR_UINT16_MAX;
            std::optional<uint16_t> parent = SR_UINT16_MAX;
            SR_MATH_NS::DecomposedMatrix localTransform;
            SR_MATH_NS::DecomposedMatrix globalTransform;
            SmallVector<uint16_t, 4> children;
            SmallVector<uint16_t, 1> meshes;
        };

        void Reset();

    #ifdef SR_UTILS_ASSIMP
        void ImportFromAssimp(const aiScene* pScene, bool loadAnimation);
    #endif

        SR_NODISCARD Vector<MeshData>& GetMeshes() { return m_meshes; }
        SR_NODISCARD const Vector<MeshData>& GetMeshes() const { return m_meshes; }
        SR_NODISCARD const SceneNode& GetRootNode() const;
        SR_NODISCARD const AnimationMap& GetAnimations() const { return m_animations; }
        SR_NODISCARD uint16_t GetNodesCount() const { return static_cast<uint16_t>(m_scenePool.size()); }
        SR_NODISCARD const SceneNode* FindNodeByName(StringAtom name) const;
        SR_NODISCARD const SceneNode& GetNodeByIndex(uint16_t index) const;
        SR_NODISCARD const Vector<SceneNode>& GetNodes() const { return m_scenePool; }

        void ForEachNode(bool hierarchical, const SR_HTYPES_NS::Function<void(const SceneNode&)>& callback) const;
        void ForEachMeshOnNode(uint16_t nodeIndex, const SR_HTYPES_NS::Function<void(const MeshData&)>& callback) const;
        void ForEachChildNode(uint16_t parentIndex, const SR_HTYPES_NS::Function<void(const SceneNode&)>& callback) const;

    private:
    #ifdef SR_UTILS_ASSIMP
        void ImportBones(const aiScene* pScene);
        void ImportScene(const aiScene* pScene);
        void ImportNode(const aiNode* pNode, std::optional<uint16_t> parentIndex);
    #endif

        SceneNode& AddSceneNode(std::optional<uint16_t> parentIndex);

        void CalculateGlobalTransforms();

    private:
        Vector<SceneNode> m_scenePool;
        Vector<MeshData> m_meshes;
        AnimationMap m_animations;

    };
}

#endif //SR_ENGINE_COMMON_MESH_SCENE_STRUCTURE_H
