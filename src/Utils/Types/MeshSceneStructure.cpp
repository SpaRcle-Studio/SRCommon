//
// Created by Monika on 16.06.2026.
//

#include <Utils/Types/MeshSceneStructure.h>

#ifdef SR_UTILS_ASSIMP
    #include <Utils/Resources/AssimpTools.h>

    #include <assimp/scene.h>
#endif

namespace SR_HTYPES_NS {
    static const StringView PRE_TRANSFORM_NODE_PREFIX = "_$AssimpFbx$_";

    void MeshSceneStructure::Reset() {
        SR_TRACY_ZONE;
        m_scenePool.clear();
        m_meshes.clear();
        m_animations.clear();
    }

#ifdef SR_UTILS_ASSIMP
    void MeshSceneStructure::ImportFromAssimp(const aiScene* pScene, bool loadAnimation) {
        SR_TRACY_ZONE;

        Reset();

        if (!pScene) {
            SRHalt("MeshSceneStructure::ImportFromAssimp() : pScene is nullptr!");
            return;
        }

        if (pScene->mRootNode) {
            m_scenePool.reserve(pScene->mNumMeshes * SR_MIN(16, pScene->mRootNode->mNumChildren));
        }

        m_meshes.resize(pScene->mNumMeshes);
        for (uint32_t i = 0; i < pScene->mNumMeshes; ++i) {
            auto&& pMesh = pScene->mMeshes[i];
            auto&& meshData = m_meshes[i];
            meshData.materialId = pMesh->mMaterialIndex;
            meshData.meshId = i;
        }

        if (loadAnimation) {
            for (uint32_t i = 0; i < pScene->mNumAnimations; ++i) {
                auto&& pAnimation = pScene->mAnimations[i];
                std::string_view animationName(pAnimation->mName.C_Str(), pAnimation->mName.length);
                m_animations[SR_UTILS_NS::StringAtom(animationName)] = pAnimation;
            }
        }

        ImportScene(pScene);
        ImportBones(pScene);

        for (auto&& mesh : m_meshes) {
            mesh.maxBoneId = 0;
            for (const auto& [boneName, boneInfo] : mesh.bones) {
                mesh.maxBoneId = SR_MAX(mesh.maxBoneId, boneInfo.boneId);
            }
        }
    }

    void MeshSceneStructure::ImportBones(const aiScene* pScene) {
        SR_TRACY_ZONE;

        for (uint32_t meshId = 0; meshId < pScene->mNumMeshes; ++meshId) {
            auto&& pMesh = pScene->mMeshes[meshId];
            auto&& bones = m_meshes[meshId].bones;

            for (uint32_t boneId = 0; boneId < pMesh->mNumBones; ++boneId) {
                StringView boneName(pMesh->mBones[boneId]->mName.C_Str(), pMesh->mBones[boneId]->mName.length);
                if (boneName.find(PRE_TRANSFORM_NODE_PREFIX) != StringView::npos) {
                    continue;
                }

                auto&& name = SR_UTILS_NS::StringAtom(boneName);

                if (bones.count(name) == 1) {
                    SR_WARN("RawMesh::CalculateBones() : bone already exists! \n\tName: " + name.ToString());
                    continue;
                }

                auto&& matrix = pMesh->mBones[boneId]->mOffsetMatrix;
                aiQuaternion rotation;
                aiVector3D scaling, translation;
                matrix.Decompose(scaling, rotation, translation);

                MeshSceneStructure::BoneInfo& info = bones[name];
                info.boneId = static_cast<uint32_t>(bones.size());
                //info.offsetMatrix = SR_MATH_NS::Matrix4x4(AssimpTools::ConvertAssimpMatrix(matrix));
                info.offsetMatrix = SR_MATH_NS::Matrix4x4(
                    SR_MATH_NS::FVector3(translation.x, translation.y, translation.z),
                    SR_MATH_NS::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w),
                    SR_MATH_NS::FVector3(scaling.x, scaling.y, scaling.z)
                );
            }
        }
    }

    void MeshSceneStructure::ImportScene(const aiScene* pScene) {
        SR_TRACY_ZONE;
        if (!pScene->mRootNode) {
            SRHalt("MeshSceneStructure::ImportScene() : root node is nullptr!");
            return;
        }
        ImportNode(pScene->mRootNode, std::nullopt);
    }

    void MeshSceneStructure::ImportNode(const aiNode* pNode, std::optional<uint16_t> parentIndex) {
        SR_TRACY_ZONE;

        aiMatrix4x4 preTransform;
        StringView nodeName(pNode->mName.C_Str(), pNode->mName.length);
        if (nodeName.find(PRE_TRANSFORM_NODE_PREFIX) != StringView::npos) {
            static SR_HTYPES_NS::Function<void(const aiNode*&, aiMatrix4x4&)> calculatePreTransform = [](const aiNode*& node, aiMatrix4x4& transform) {
                if (node->mNumChildren != 1) {
                    SRHalt("Pre-transform node should have exactly one child!");
                    return;
                }
                transform = transform * node->mTransformation;
                node = node->mChildren[0];
                StringView childName(node->mName.C_Str(), node->mName.length);
                if (childName.find(PRE_TRANSFORM_NODE_PREFIX) != StringView::npos) {
                    calculatePreTransform(node, transform);
                }
            };
            calculatePreTransform(pNode, preTransform);
            nodeName = StringView(pNode->mName.C_Str(), pNode->mName.length);
        }

        auto&& sceneNode = AddSceneNode(parentIndex);
        sceneNode.name = StringAtom(nodeName);
        sceneNode.parent = parentIndex;

        aiQuaternion rotation;
        aiVector3D scaling, translation;
        preTransform *= pNode->mTransformation;
        preTransform.Decompose(scaling, rotation, translation);

        sceneNode.transform = SR_MATH_NS::Matrix4x4(
            SR_MATH_NS::FVector3(translation.x, translation.y, translation.z),
            SR_MATH_NS::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w),
            SR_MATH_NS::FVector3(scaling.x, scaling.y, scaling.z)
        );

        for (uint32_t i = 0; i < pNode->mNumMeshes; ++i) {
            uint32_t meshId = pNode->mMeshes[i];
            if (meshId < m_meshes.size()) {
                sceneNode.meshes.emplace_back(meshId);
            }
        }

        const uint32_t index = sceneNode.index; /// scene node may be reallocated during AddSceneNode() call, so we need to store the index before the recursive call
        for (uint32_t i = 0; i < pNode->mNumChildren; ++i) {
            ImportNode(pNode->mChildren[i], index);
        }
    }
#endif

    MeshSceneStructure::SceneNode& MeshSceneStructure::AddSceneNode(std::optional<uint16_t> parentIndex) {
        SceneNode node;
        node.index = static_cast<uint16_t>(m_scenePool.size());
        if (parentIndex.has_value() && SRVerify(parentIndex.value() < m_scenePool.size())) {
            m_scenePool[parentIndex.value()].children.emplace_back(node.index);
        }
        m_scenePool.emplace_back(std::move(node));
        return m_scenePool.back();
    }

    const MeshSceneStructure::SceneNode& MeshSceneStructure::GetRootNode() const {
        if (m_scenePool.empty()) {
            SRHalt("MeshSceneStructure::GetRootNode() : scene pool is empty!");
            static SceneNode emptyNode;
            return emptyNode;
        }
        return m_scenePool.front();
    }

    void MeshSceneStructure::ForEachNode(bool hierarchical, const SR_HTYPES_NS::Function<void(const SceneNode&)>& callback) const {
        if (hierarchical) {
            callback(GetRootNode());
            ForEachChildNode(GetRootNode().index, callback);
        }
        else {
            for (const auto& node : m_scenePool) {
                callback(node);
            }
        }
    }

    void MeshSceneStructure::ForEachChildNode(uint16_t parentIndex, const Function<void(const SceneNode&)>& callback) const {
        if (parentIndex >= m_scenePool.size()) {
            SRHalt("MeshSceneStructure::ForEachChildNode() : parent index is out of range!");
            return;
        }

        const auto& parentNode = m_scenePool[parentIndex];
        for (const auto& childIndex : parentNode.children) {
            const auto& childNode = m_scenePool[childIndex];
            callback(childNode);
            ForEachChildNode(childIndex, callback);
        }
    }

    void MeshSceneStructure::ForEachMeshOnNode(uint16_t nodeIndex, const Function<void(const MeshData&)>& callback) const {
        if (nodeIndex >= m_scenePool.size()) {
            SRHalt("MeshSceneStructure::ForEachMeshOnNode() : node index is out of range!");
            return;
        }

        const auto& node = m_scenePool[nodeIndex];
        for (const auto& meshIndex : node.meshes) {
            if (meshIndex < m_meshes.size()) {
                callback(m_meshes[meshIndex]);
            }
        }
    }
}
