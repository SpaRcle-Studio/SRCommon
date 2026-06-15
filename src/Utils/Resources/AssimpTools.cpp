//
// Created by Monika on 16.06.2026.
//

#include <Utils/Resources/AssimpTools.h>

#ifdef SR_UTILS_ASSIMP

#include <assimp/scene.h>
#include <assimp/mesh.h>

namespace SR_UTILS_NS::AssimpTools {
    uint32_t NormalizeWeights(const aiMesh* pMesh) {
        SR_TRACY_ZONE;

        if (pMesh->mNumBones == 0) {
            return 0;
        }

        struct BoneWeight {
            uint32_t mBoneIndex; /// index of a bone in current mesh
            aiVertexWeight* mVertexWeight; /// a pointer to mVertexWeight in meshs[x]->mBones[x]->mWeight for quick visit
        };

        struct VertexBoneWeights {
            float_t mTotalWeight = 0.f;
            std::vector<BoneWeight> mBoneWeights;
        };

        std::map<uint32_t, VertexBoneWeights> map;

        for (uint32_t b = 0; b < pMesh->mNumBones; b++)
        {
            auto bone = pMesh->mBones[b];

            for (unsigned int w = 0; w < bone->mNumWeights; w++)
            {
                auto vertexWeight = &bone->mWeights[w];
                auto key = vertexWeight->mVertexId;

                if (map.find(key) == map.end()) {
                    map.insert(std::map<uint32_t, VertexBoneWeights>::value_type(key, VertexBoneWeights()));
                }

                auto& vertex_BoneWeights = map[key];

                BoneWeight boneWeights = {};
                boneWeights.mBoneIndex = b;
                boneWeights.mVertexWeight = vertexWeight;

                vertex_BoneWeights.mTotalWeight += vertexWeight->mWeight;
                vertex_BoneWeights.mBoneWeights.push_back(boneWeights);
            }
        }


        uint32_t count = 0;
        /// normalize all weights:
        /// every weight for a same vertex divided by totalWeight of this vertex
        for (auto& item : map)
        {
            auto& vertex_BoneWeights = item.second;
            auto f = 1.f / vertex_BoneWeights.mTotalWeight;
            for (auto&& mBoneWeight : vertex_BoneWeights.mBoneWeights)
            {
                mBoneWeight.mVertexWeight->mWeight *= f;
                count++;
            }
        }

        return count;
    }
}

#endif
