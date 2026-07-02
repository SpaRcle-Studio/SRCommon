//
// Created by Monika on 15.02.2023.
//

#ifndef SR_ENGINE_ASSIMP_CACHE_H
#define SR_ENGINE_ASSIMP_CACHE_H

#include <Utils/Types/RawPointerHolder.h>
#include <Utils/Common/Singleton.h>
#include <Utils/FileSystem/Path.h>

#ifdef SR_UTILS_ASSIMP
namespace Assimp {
    class Importer;
}

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiAnimMesh;
struct aiString;

namespace SR_HTYPES_NS {
    class Marshal;
}

namespace SR_UTILS_NS {
    class MappedFile;

    extern const uint8_t SR_ASSIMP_MAX_NUMBER_OF_COLOR_SETS;
    extern const uint8_t SR_ASSIMP_MAX_NUMBER_OF_TEXTURECOORDS;

    class AssimpCache final : public Singleton<AssimpCache> {
        SR_REGISTER_SINGLETON(AssimpCache);
        SR_MAYBE_UNUSED SR_INLINE_STATIC const uint64_t VERSION = 1015;
        using NodeIndex = uint64_t;
        using MeshIndex = uint64_t;
        using NodeMap = std::pair<std::vector<aiNode*>, std::unordered_map<aiNode*, NodeIndex>>;
        using MeshMap = std::pair<std::vector<aiMesh*>, std::unordered_map<aiMesh*, MeshIndex>>;
    public:
        bool Save(const SR_UTILS_NS::Path& path, const aiScene* pScene) const;
        aiScene* Load(const SR_UTILS_NS::Path& path, SR_HTYPES_NS::RawPointerHolder<SR_UTILS_NS::MappedFile>& cache) const;

    private:
        SR_NODISCARD NodeMap BuildNodeMap(const aiScene* pScene) const;
        SR_NODISCARD MeshMap BuildMeshMap(const aiScene* pScene) const;

        void SaveSkeletons(SR_HTYPES_NS::Marshal& marshal, const aiScene* pScene) const;
        void LoadSkeletons(SR_HTYPES_NS::Marshal& marshal, aiScene* pScene) const;

        void SaveAnimations(SR_HTYPES_NS::Marshal& marshal, const aiScene* pScene) const;
        void LoadAnimations(SR_HTYPES_NS::Marshal& marshal, aiScene* pScene) const;

        void SaveNode(SR_HTYPES_NS::Marshal& marshal, const aiNode* pNode) const;
        void LoadNode(SR_HTYPES_NS::Marshal& marshal, aiNode*& pNode) const;

        void SaveMeshes(SR_HTYPES_NS::Marshal& marshal, const aiScene* pScene) const;
        void LoadMeshes(SR_HTYPES_NS::Marshal& marshal, aiScene* pScene) const;

        void SaveMaterials(SR_HTYPES_NS::Marshal& marshal, const aiScene* pScene) const;
        void LoadMaterials(SR_HTYPES_NS::Marshal& marshal, aiScene* pScene) const;

        void SaveTextures(SR_HTYPES_NS::Marshal& marshal, const aiScene* pScene) const;
        void LoadTextures(SR_HTYPES_NS::Marshal& marshal, aiScene* pScene) const;

    };
}
#endif

#endif //SR_ENGINE_ASSIMP_CACHE_H
