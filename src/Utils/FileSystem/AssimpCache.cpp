//
// Created by Monika on 15.02.2023.
//

#include <Utils/FileSystem/AssimpCache.h>
#include <Utils/FileSystem/MappedFile.h>
#include <Utils/Types/Marshal.h>
#include <Utils/Profile/TracyContext.h>

#ifdef SR_UTILS_ASSIMP
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/include/assimp/Exporter.hpp>
#include <assimp/include/assimp/cexport.h>

namespace SR_UTILS_NS {
    const uint8_t SR_ASSIMP_MAX_NUMBER_OF_COLOR_SETS = AI_MAX_NUMBER_OF_COLOR_SETS;
    const uint8_t SR_ASSIMP_MAX_NUMBER_OF_TEXTURECOORDS = AI_MAX_NUMBER_OF_TEXTURECOORDS;

    template<typename T> void SaveMesh(SR_HTYPES_NS::Marshal& marshal, const T* pMesh) {
        for (uint8_t colorId = 0; colorId < SR_ASSIMP_MAX_NUMBER_OF_COLOR_SETS; ++colorId) {
            if (pMesh->mColors[colorId]) {
                marshal.WriteBlock(pMesh->mColors[colorId], pMesh->mNumVertices * sizeof(aiColor4D));
            }
        }

        if (pMesh->mVertices) {
            marshal.WriteBlock(pMesh->mVertices, pMesh->mNumVertices * sizeof(aiVector3D));
        }

        if (pMesh->mNormals) {
            marshal.WriteBlock(pMesh->mNormals, pMesh->mNumVertices * sizeof(aiVector3D));
        }

        if (pMesh->mTangents) {
            marshal.WriteBlock(pMesh->mTangents, pMesh->mNumVertices * sizeof(aiVector3D));
        }

        if (pMesh->mBitangents) {
            marshal.WriteBlock(pMesh->mBitangents, pMesh->mNumVertices * sizeof(aiVector3D));
        }

        for (uint8_t numberTextureCoords = 0; numberTextureCoords < SR_ASSIMP_MAX_NUMBER_OF_TEXTURECOORDS; ++numberTextureCoords) {
            if (auto&& textureCoords = pMesh->mTextureCoords[numberTextureCoords]) {
                marshal.WriteBlock(textureCoords, pMesh->mNumVertices * sizeof(aiVector3D));
            }
        }
    }

    template<typename T> void LoadMesh(SR_HTYPES_NS::Marshal& marshal, T* pMesh) {
        for (uint8_t colorId = 0; colorId < SR_ASSIMP_MAX_NUMBER_OF_COLOR_SETS; ++colorId) {
            if (pMesh->mColors[colorId]) {
                pMesh->mColors[colorId] = static_cast<aiColor4D*>(marshal.ReadMapBlock());
            }
        }

        if (pMesh->mVertices) {
            pMesh->mVertices = static_cast<aiVector3D*>(marshal.ReadMapBlock());
        }

        if (pMesh->mNormals) {
            pMesh->mNormals = static_cast<aiVector3D*>(marshal.ReadMapBlock());
        }

        if (pMesh->mTangents) {
            pMesh->mTangents = static_cast<aiVector3D*>(marshal.ReadMapBlock());
        }

        if (pMesh->mBitangents) {
            pMesh->mBitangents = static_cast<aiVector3D*>(marshal.ReadMapBlock());
        }

        for (uint8_t numberTextureCoords = 0; numberTextureCoords < SR_ASSIMP_MAX_NUMBER_OF_TEXTURECOORDS; ++numberTextureCoords) {
            if (pMesh->mTextureCoords[numberTextureCoords]) {
                pMesh->mTextureCoords[numberTextureCoords] = static_cast<aiVector3D*>(marshal.ReadMapBlock());
            }
        }
    }

    bool AssimpCache::Save(const SR_UTILS_NS::Path& path, const aiScene* pScene) const {
        SR_TRACY_ZONE;

        SR_HTYPES_NS::Marshal marshal;

        marshal.Reserve(64 * 1024 * 1024); // 64 MB

        marshal.Write<uint64_t>(VERSION);
        marshal.WriteBlock((void*)pScene, sizeof(aiScene));

        if (pScene->mRootNode) {
            SaveNode(marshal, pScene->mRootNode);
        }

        SaveMeshes(marshal, pScene);
        SaveSkeletons(marshal, pScene);
        SaveMaterials(marshal, pScene);
        SaveTextures(marshal, pScene);
        SaveAnimations(marshal, pScene);

        return marshal.Save(path);
    }

    aiScene* AssimpCache::Load(const Path& path, SR_HTYPES_NS::RawPointerHolder<SR_UTILS_NS::MappedFile>& cache) const {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(path.ToStringRef());

        SRAssert2(!cache, "Heap is not empty!");

        SR_UTILS_NS::MappedFile mappedFile = SR_UTILS_NS::MappedFile::Open(path, true);
        if (!mappedFile) {
            return nullptr;
        }

        auto&& marshal = SR_HTYPES_NS::Marshal(mappedFile);

        /// version
        if (marshal.Read<uint64_t>() != VERSION) {
            return nullptr;
        }

        auto&& pScene = static_cast<aiScene*>(marshal.ReadMapBlock());

        if (pScene->mRootNode) { /// has root node
            LoadNode(marshal, pScene->mRootNode);
        }

        LoadMeshes(marshal, pScene);
        LoadSkeletons(marshal, pScene);
        LoadMaterials(marshal, pScene);
        LoadTextures(marshal, pScene);
        LoadAnimations(marshal, pScene);

        cache = new SR_UTILS_NS::MappedFile(std::move(mappedFile));

        return pScene;
    }

    void AssimpCache::LoadNode(SR_HTYPES_NS::Marshal& marshal, aiNode*& pNode) const {
        pNode = static_cast<aiNode*>(marshal.ReadMapBlock());
        pNode->mMeshes = static_cast<uint32_t*>(marshal.ReadMapBlock());

        if (pNode->mChildren) {
            pNode->mChildren = static_cast<aiNode**>(marshal.ReadMapBlock());
            for (uint64_t childId = 0; childId < pNode->mNumChildren; ++childId) {
                LoadNode(marshal, pNode->mChildren[childId]);
                pNode->mChildren[childId]->mParent = pNode;
            }
        }
    }

    void AssimpCache::SaveNode(SR_HTYPES_NS::Marshal& marshal, const aiNode* pNode) const {
        marshal.WriteBlock((void*)pNode, sizeof(aiNode));
        marshal.WriteBlock((void*)pNode->mMeshes, pNode->mNumMeshes * sizeof(uint32_t));

        if (pNode->mChildren) {
            marshal.WriteBlock((void*)pNode->mChildren, pNode->mNumChildren * sizeof(aiNode*));
            for (uint64_t childId = 0; childId < pNode->mNumChildren; ++childId) {
                SaveNode(marshal, pNode->mChildren[childId]);
            }
        }
    }

    void AssimpCache::SaveAnimations(SR_HTYPES_NS::Marshal& marshal, const aiScene* pScene) const {
        if (pScene->mAnimations) {
            marshal.WriteBlock((void*)pScene->mAnimations, pScene->mNumAnimations * sizeof(aiAnimation*));
            for (uint64_t animationId = 0; animationId < pScene->mNumAnimations; ++animationId) {
                auto&& pAnimation = pScene->mAnimations[animationId];
                marshal.WriteBlock((void*)pAnimation, sizeof(aiAnimation));

                if (pAnimation->mChannels) {
                    marshal.WriteBlock((void*)pAnimation->mChannels, pAnimation->mNumChannels * sizeof(aiNodeAnim*));
                    for (uint64_t channelId = 0; channelId < pAnimation->mNumChannels; ++channelId) {
                        auto&& pChannel = pAnimation->mChannels[channelId];
                        marshal.WriteBlock((void*)pChannel, sizeof(aiNodeAnim));

                        marshal.WriteBlock((void*)pChannel->mPositionKeys, pChannel->mNumPositionKeys * sizeof(aiVectorKey));
                        marshal.WriteBlock((void*)pChannel->mRotationKeys, pChannel->mNumRotationKeys * sizeof(aiQuatKey));
                        marshal.WriteBlock((void*)pChannel->mScalingKeys, pChannel->mNumScalingKeys * sizeof(aiVectorKey));
                    }
                }

                if (pAnimation->mMeshChannels) {
                    marshal.WriteBlock((void*)pAnimation->mMeshChannels, pAnimation->mNumMeshChannels * sizeof(aiMeshAnim*));
                    for (uint64_t meshChannelId = 0; meshChannelId < pAnimation->mNumMeshChannels; ++meshChannelId) {
                        auto&& pMeshChannel = pAnimation->mMeshChannels[meshChannelId];
                        marshal.WriteBlock((void*)pMeshChannel, sizeof(aiMeshAnim));
                        marshal.WriteBlock((void*)pMeshChannel->mKeys, pMeshChannel->mNumKeys * sizeof(aiMeshKey));
                    }
                }

                if (pAnimation->mMorphMeshChannels) {
                    marshal.WriteBlock((void*)pAnimation->mMorphMeshChannels, pAnimation->mNumMorphMeshChannels * sizeof(aiMeshMorphAnim*));
                    for (uint64_t morphMeshChannelId = 0; morphMeshChannelId < pAnimation->mNumMorphMeshChannels; ++morphMeshChannelId) {
                        auto&& pMorphMeshChannel = pAnimation->mMorphMeshChannels[morphMeshChannelId];
                        marshal.WriteBlock((void*)pMorphMeshChannel, sizeof(aiMeshMorphAnim));

                        if (pMorphMeshChannel->mKeys) {
                            marshal.WriteBlock((void*)pMorphMeshChannel->mKeys, pMorphMeshChannel->mNumKeys * sizeof(aiMeshMorphKey));
                            for (uint64_t keyId = 0; keyId < pMorphMeshChannel->mNumKeys; ++keyId) {
                                auto&& key = pMorphMeshChannel->mKeys[keyId];
                                marshal.WriteBlock((void*)&key, sizeof(aiMeshMorphKey));
                                marshal.WriteBlock((void*)key.mValues, key.mNumValuesAndWeights * sizeof(uint32_t));
                                marshal.WriteBlock((void*)key.mWeights, key.mNumValuesAndWeights * sizeof(double_t));
                            }
                        }
                    }
                }
            }
        }
    }

    void AssimpCache::LoadAnimations(SR_HTYPES_NS::Marshal& marshal, aiScene* pScene) const {
        if (pScene->mAnimations) {
            pScene->mAnimations = static_cast<aiAnimation**>(marshal.ReadMapBlock());
            for (uint64_t animationId = 0; animationId < pScene->mNumAnimations; ++animationId) {
                auto&& pAnimation = pScene->mAnimations[animationId];
                pAnimation = static_cast<aiAnimation*>(marshal.ReadMapBlock());

                if (pAnimation->mChannels) {
                    pAnimation->mChannels = static_cast<aiNodeAnim**>(marshal.ReadMapBlock());
                    for (uint64_t channelId = 0; channelId < pAnimation->mNumChannels; ++channelId) {
                        auto&& pChannel = pAnimation->mChannels[channelId];
                        pChannel = static_cast<aiNodeAnim*>(marshal.ReadMapBlock());

                        pChannel->mPositionKeys = static_cast<aiVectorKey*>(marshal.ReadMapBlock());
                        pChannel->mRotationKeys = static_cast<aiQuatKey*>(marshal.ReadMapBlock());
                        pChannel->mScalingKeys = static_cast<aiVectorKey*>(marshal.ReadMapBlock());
                    }
                }

                if (pAnimation->mMeshChannels) {
                    pAnimation->mMeshChannels = static_cast<aiMeshAnim**>(marshal.ReadMapBlock());
                    for (uint64_t meshChannelId = 0; meshChannelId < pAnimation->mNumMeshChannels; ++meshChannelId) {
                        auto&& pMeshChannel = pAnimation->mMeshChannels[meshChannelId];
                        pMeshChannel = static_cast<aiMeshAnim*>(marshal.ReadMapBlock());
                        pMeshChannel->mKeys = static_cast<aiMeshKey*>(marshal.ReadMapBlock());
                    }
                }

                if (pAnimation->mMorphMeshChannels) {
                    pAnimation->mMorphMeshChannels = static_cast<aiMeshMorphAnim**>(marshal.ReadMapBlock());
                    for (uint64_t morphMeshChannelId = 0; morphMeshChannelId < pAnimation->mNumMorphMeshChannels; ++morphMeshChannelId) {
                        auto&& pMorphMeshChannel = pAnimation->mMorphMeshChannels[morphMeshChannelId];
                        pMorphMeshChannel = static_cast<aiMeshMorphAnim*>(marshal.ReadMapBlock());

                        if (pMorphMeshChannel->mKeys) {
                            pMorphMeshChannel->mKeys = static_cast<aiMeshMorphKey*>(marshal.ReadMapBlock());
                            for (uint64_t keyId = 0; keyId < pMorphMeshChannel->mNumKeys; ++keyId) {
                                auto&& key = pMorphMeshChannel->mKeys[keyId];
                                marshal.ReadBlock((void*)&key);
                                key.mValues = static_cast<uint32_t*>(marshal.ReadMapBlock());
                                key.mWeights = static_cast<double_t*>(marshal.ReadMapBlock());
                            }
                        }
                    }
                }
            }
        }
    }

    void AssimpCache::SaveSkeletons(SR_HTYPES_NS::Marshal& marshal, const aiScene* pScene) const {
        auto&& nodeMap = BuildNodeMap(pScene);
        auto&& meshMap = BuildMeshMap(pScene);

        if (pScene->mSkeletons) {
            marshal.WriteBlock((void*)pScene->mSkeletons, sizeof(aiSkeleton*) * pScene->mNumSkeletons);
            for (uint64_t skeletonId = 0; skeletonId < pScene->mNumSkeletons; ++skeletonId) {
                auto&& pSkeleton = pScene->mSkeletons[skeletonId];
                marshal.WriteBlock((void*)pSkeleton, sizeof(aiSkeleton));

                if (pSkeleton->mBones) {
                    marshal.WriteBlock((void*)pSkeleton->mBones, sizeof(aiSkeletonBone*) * pSkeleton->mNumBones);
                    for (uint64_t boneId = 0; boneId < pSkeleton->mNumBones; ++boneId) {
                        auto&& pBone = pSkeleton->mBones[boneId];
                        marshal.WriteBlock((void*)pBone, sizeof(aiSkeletonBone));

                        if (pBone->mMeshId) {
                            marshal.Write<uint64_t>(meshMap.second.at(pBone->mMeshId));
                        }

                        if (pBone->mArmature) {
                            marshal.Write<uint64_t>(nodeMap.second.at(pBone->mArmature));
                        }

                        if (pBone->mNode) {
                            marshal.Write<uint64_t>(nodeMap.second.at(pBone->mNode));
                        }

                        marshal.WriteBlock((void*)pBone->mWeights, pBone->mNumnWeights * sizeof(aiVertexWeight));
                    }
                }
            }
        }
    }

    void AssimpCache::LoadSkeletons(SR_HTYPES_NS::Marshal& marshal, aiScene* pScene) const {
        SR_TRACY_ZONE;

        auto&& nodeMap = BuildNodeMap(pScene);
        auto&& meshMap = BuildMeshMap(pScene);

        if (pScene->mSkeletons) {
            for (uint64_t skeletonId = 0; skeletonId < pScene->mNumSkeletons; ++skeletonId) {
                auto&& pSkeleton = pScene->mSkeletons[skeletonId];
                pSkeleton = static_cast<aiSkeleton*>(marshal.ReadMapBlock());

                if (pSkeleton->mBones) {
                    for (uint64_t boneId = 0; boneId < pSkeleton->mNumBones; ++boneId) {
                        auto&& pBone = pSkeleton->mBones[boneId];
                        pBone = static_cast<aiSkeletonBone*>(marshal.ReadMapBlock());

                        if (pBone->mMeshId) {
                            pBone->mMeshId = meshMap.first[marshal.Read<uint64_t>()];
                        }

                        if (pBone->mArmature) {
                            pBone->mArmature = nodeMap.first[marshal.Read<uint64_t>()];
                        }

                        if (pBone->mNode) {
                            pBone->mNode = nodeMap.first[marshal.Read<uint64_t>()];
                        }

                        pBone->mWeights = static_cast<aiVertexWeight*>(marshal.ReadMapBlock());
                    }
                }
            }
        }
    }

    void AssimpCache::LoadMeshes(SR_HTYPES_NS::Marshal& marshal, aiScene* pScene) const {
        SR_TRACY_ZONE;

        auto&& nodeMap = BuildNodeMap(pScene);

        if (pScene->mMeshes) {
            pScene->mMeshes = static_cast<aiMesh**>(marshal.ReadMapBlock());
            for (uint64_t meshId = 0; meshId < pScene->mNumMeshes; ++meshId) {
                auto&& pMesh = pScene->mMeshes[meshId];
                pMesh = static_cast<aiMesh*>(marshal.ReadMapBlock());

                LoadMesh(marshal, pMesh);

                /// has texture coords names
                if (pMesh->mTextureCoordsNames) {
                    pMesh->mTextureCoordsNames = static_cast<aiString**>(marshal.ReadMapBlock());
                    for (uint8_t i = 0; i < SR_ASSIMP_MAX_NUMBER_OF_TEXTURECOORDS; ++i) {
                        if (pMesh->mTextureCoordsNames[i]) {
                            pMesh->mTextureCoordsNames[i] = static_cast<aiString *>(marshal.ReadMapBlock());
                        }
                    }
                }

                /// --------------------------------------------------------------------------------------------------------

                if (pMesh->mFaces) {
                    pMesh->mFaces = static_cast<aiFace*>(marshal.ReadMapBlock());
                    for (uint64_t faceId = 0; faceId < pMesh->mNumFaces; ++faceId) {
                        pMesh->mFaces[faceId].mIndices = static_cast<unsigned int*>(marshal.ReadMapBlock());
                    }
                }

                /// --------------------------------------------------------------------------------------------------------

                if (pMesh->mBones) {
                    pMesh->mBones = static_cast<aiBone**>(marshal.ReadMapBlock());
                    for (uint64_t boneId = 0; boneId < pMesh->mNumBones; ++boneId) {
                        auto&& pBone = pMesh->mBones[boneId];
                        pBone = static_cast<aiBone*>(marshal.ReadMapBlock());
                        pBone->mWeights = static_cast<aiVertexWeight*>(marshal.ReadMapBlock());

                        if (pBone->mArmature) {
                            pBone->mArmature = nodeMap.first[marshal.Read<uint64_t>()];
                        }

                        if (pBone->mNode) {
                            pBone->mNode = nodeMap.first[marshal.Read<uint64_t>()];
                        }
                    }
                }

                /// --------------------------------------------------------------------------------------------------------

                if (pMesh->mAnimMeshes) {
                    pMesh->mAnimMeshes = static_cast<aiAnimMesh**>(marshal.ReadMapBlock());
                    for (uint64_t animatedMeshId = 0; animatedMeshId < pMesh->mNumAnimMeshes; ++animatedMeshId) {
                        auto&& pAnimatedMesh = pMesh->mAnimMeshes[animatedMeshId];
                        pAnimatedMesh = static_cast<aiAnimMesh*>(marshal.ReadMapBlock());
                        LoadMesh(marshal, pAnimatedMesh);
                    }
                }
            }
        }
    }

    void AssimpCache::SaveMeshes(SR_HTYPES_NS::Marshal& marshal, const aiScene* pScene) const {
        auto&& nodeMap = BuildNodeMap(pScene);

        if (pScene->mMeshes) {
            marshal.WriteBlock((void*)pScene->mMeshes, pScene->mNumMeshes * sizeof(aiMesh*));
            for (uint64_t meshId = 0; meshId < pScene->mNumMeshes; ++meshId) {
                auto&& pMesh = pScene->mMeshes[meshId];
                marshal.WriteBlock((void*)pMesh, sizeof(aiMesh));

                /// --------------------------------------------------------------------------------------------------------

                SaveMesh(marshal, pMesh);

                /// --------------------------------------------------------------------------------------------------------

                if (pMesh->mTextureCoordsNames) {
                    marshal.WriteBlock((void*)pMesh->mTextureCoordsNames, SR_ASSIMP_MAX_NUMBER_OF_TEXTURECOORDS * sizeof(aiString*));
                    for (uint8_t i = 0; pMesh->mTextureCoordsNames && i < SR_ASSIMP_MAX_NUMBER_OF_TEXTURECOORDS; ++i) {
                        if (auto&& pTextureCoordName = pMesh->mTextureCoordsNames[i]) {
                            marshal.WriteBlock((void *) pTextureCoordName, sizeof(aiString));
                        }
                    }
                }

                /// --------------------------------------------------------------------------------------------------------

                if (pMesh->mFaces) {
                    marshal.WriteBlock((void*)pMesh->mFaces, pMesh->mNumFaces * sizeof(aiFace));
                    for (uint64_t faceId = 0; faceId < pMesh->mNumFaces; ++faceId) {
                        marshal.WriteBlock(pMesh->mFaces[faceId].mIndices, pMesh->mFaces[faceId].mNumIndices * sizeof(unsigned int));
                    }
                }

                /// --------------------------------------------------------------------------------------------------------

                if (pMesh->mBones) {
                    marshal.WriteBlock((void*)pMesh->mBones, pMesh->mNumBones * sizeof(aiBone*));
                    for (uint64_t boneId = 0; boneId < pMesh->mNumBones; ++boneId) {
                        auto&& pBone = pMesh->mBones[boneId];
                        marshal.WriteBlock((void*)pBone, sizeof(aiBone));
                        marshal.WriteBlock((void*)pBone->mWeights, pBone->mNumWeights * sizeof(aiVertexWeight));

                        if (pBone->mArmature) {
                            marshal.Write<uint64_t>(nodeMap.second.at(pBone->mArmature));
                        }
                        if (pBone->mNode) {
                            marshal.Write<uint64_t>(nodeMap.second.at(pBone->mNode));
                        }
                    }
                }

                /// --------------------------------------------------------------------------------------------------------

                if (pMesh->mAnimMeshes) {
                    marshal.WriteBlock((void*)pMesh->mAnimMeshes, pMesh->mNumAnimMeshes * sizeof(aiAnimMesh*));
                    for (uint64_t animatedMeshId = 0; animatedMeshId < pMesh->mNumAnimMeshes; ++animatedMeshId) {
                        auto&& pAnimatedMesh = pMesh->mAnimMeshes[animatedMeshId];
                        marshal.WriteBlock((void*)pAnimatedMesh, sizeof(aiAnimMesh));
                        SaveMesh(marshal, pAnimatedMesh);
                    }
                }
            }
        }
    }

    AssimpCache::NodeMap AssimpCache::BuildNodeMap(const aiScene* pScene) const {
        SR_TRACY_ZONE;

        AssimpCache::NodeMap nodeMap;

        if (!pScene->mRootNode) {
            return std::move(nodeMap);
        }

        std::stack<uint64_t> stack;
        aiNode* pCurrentNode = pScene->mRootNode;

        uint64_t index = 0;
        uint64_t childId = 0;

        nodeMap.second[pCurrentNode] = index;
        nodeMap.first.emplace_back(pCurrentNode);

    retry:
        if (pCurrentNode && childId < pCurrentNode->mNumChildren) {
            pCurrentNode = pCurrentNode->mChildren[childId];
            nodeMap.second[pCurrentNode] = ++index;
            nodeMap.first.emplace_back(pCurrentNode);
            stack.push(childId);
            goto retry;
        }

        if (!stack.empty()) {
            pCurrentNode = pCurrentNode->mParent;
            childId = stack.top() + 1;
            stack.pop();
            goto retry;
        }

        return std::move(nodeMap);
    }

    AssimpCache::MeshMap AssimpCache::BuildMeshMap(const aiScene* pScene) const {
        AssimpCache::MeshMap meshMap;

        for (uint64_t i = 0; i < pScene->mNumMeshes; ++i) {
            meshMap.second[pScene->mMeshes[i]] = i;
            meshMap.first.emplace_back(pScene->mMeshes[i]);
        }

        return std::move(meshMap);
    }

    void AssimpCache::SaveMaterials(SR_HTYPES_NS::Marshal& marshal, const aiScene* pScene) const {
        if (!pScene->mMaterials) {
            return;
        }
        marshal.WriteBlock((void*)pScene->mMaterials, pScene->mNumMaterials * sizeof(aiMaterial*));
        for (uint64_t materialId = 0; materialId < pScene->mNumMaterials; ++materialId) {
            auto&& pMaterial = pScene->mMaterials[materialId];
            marshal.WriteBlock((void*)pMaterial, sizeof(aiMaterial));

            if (pMaterial->mProperties) {
                marshal.WriteBlock((void*)pMaterial->mProperties, pMaterial->mNumProperties * sizeof(aiMaterialProperty*));
                for (uint64_t propertyId = 0; propertyId < pMaterial->mNumProperties; ++propertyId) {
                    auto&& pProperty = pMaterial->mProperties[propertyId];
                    marshal.WriteBlock((void*)pProperty, sizeof(aiMaterialProperty));
                    if (pProperty->mData) {
                        marshal.WriteBlock((void *) pProperty->mData, pProperty->mDataLength * sizeof(char));
                    }
                }
            }
        }
    }

    void AssimpCache::LoadMaterials(SR_HTYPES_NS::Marshal& marshal, aiScene* pScene) const {
        if (!pScene->mMaterials) {
            return;
        }
        pScene->mMaterials = static_cast<aiMaterial**>(marshal.ReadMapBlock());
        for (uint64_t materialId = 0; materialId < pScene->mNumMaterials; ++materialId) {
            auto&& pMaterial = pScene->mMaterials[materialId];
            pMaterial = static_cast<aiMaterial*>(marshal.ReadMapBlock());

            if (pMaterial->mProperties) {
                pMaterial->mProperties = static_cast<aiMaterialProperty**>(marshal.ReadMapBlock());
                for (uint64_t propertyId = 0; propertyId < pMaterial->mNumProperties; ++propertyId) {
                    auto&& pProperty = pMaterial->mProperties[propertyId];
                    pProperty = static_cast<aiMaterialProperty*>(marshal.ReadMapBlock());
                    if (pProperty->mData) {
                        pProperty->mData = static_cast<char*>(marshal.ReadMapBlock());
                    }
                }
            }
        }
    }

    void AssimpCache::SaveTextures(SR_HTYPES_NS::Marshal& marshal, const aiScene* pScene) const {
        if (!pScene->mTextures) {
            return;
        }
        marshal.WriteBlock((void*)pScene->mTextures, pScene->mNumTextures * sizeof(aiTexture*));
        for (uint64_t textureId = 0; textureId < pScene->mNumTextures; ++textureId) {
            auto&& pTexture = pScene->mTextures[textureId];
            marshal.WriteBlock((void*)pTexture, sizeof(aiTexture));
            if (pTexture->pcData) {
                marshal.WriteBlock((void*)pTexture->pcData, pTexture->mWidth * pTexture->mHeight * sizeof(aiTexel));
            }
        }
    }

    void AssimpCache::LoadTextures(SR_HTYPES_NS::Marshal& marshal, aiScene* pScene) const {
        if (!pScene->mTextures) {
            return;
        }
        pScene->mTextures = static_cast<aiTexture**>(marshal.ReadMapBlock());
        for (uint64_t textureId = 0; textureId < pScene->mNumTextures; ++textureId) {
            auto&& pTexture = pScene->mTextures[textureId];
            pTexture = static_cast<aiTexture*>(marshal.ReadMapBlock());
            if (pTexture->pcData) {
                pTexture->pcData = static_cast<aiTexel*>(marshal.ReadMapBlock());
            }
        }
    }
}

#endif