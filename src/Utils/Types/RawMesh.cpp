//
// Created by Monika on 24.03.2022.
//

#include <Utils/Types/RawMesh.h>
#include <Utils/Common/Features.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/FileSystem/AssimpCache.h>

#ifdef SR_UTILS_ASSIMP
    #include <assimp/scene.h>
    #include <assimp/postprocess.h>
    #include <assimp/Importer.hpp>
    #include <assimp/include/assimp/Exporter.hpp>
    #include <assimp/include/assimp/cexport.h>
#endif

namespace SR_HTYPES_NS {
#ifdef SR_UTILS_ASSIMP
    SR_INLINE_STATIC int SR_RAW_MESH_ASSIMP_FLAGS = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_SortByPType | aiProcess_GlobalScale;
    SR_INLINE_STATIC int SR_RAW_MESH_ASSIMP_CACHED_FLAGS = aiProcess_FlipUVs;
    SR_INLINE_STATIC int SR_RAW_MESH_ASSIMP_ANIMATION_FLAGS = aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder;
#endif

    RawMesh::RawMesh()
        : IResource(SR_COMPILE_TIME_CRC32_TYPE_NAME(RawMesh))
    {
    #ifdef SR_UTILS_ASSIMP
        m_importer = new Assimp::Importer();
    #endif
    }

    RawMesh::~RawMesh() {
    #ifdef SR_UTILS_ASSIMP
        delete m_importer;

        if (m_fromCache) {
            delete m_scene;
            m_scene = nullptr;
        }
    #endif
    }

    SR_UTILS_NS::Path RawMesh::InitializeResourcePath() const {
        auto&& resourceId = GetResourceId();
        return SR_UTILS_NS::Path(SR_UTILS_NS::StringUtils::SubstringView(resourceId, '|', 1));
    }

    RawMesh::Ptr RawMesh::Load(const SR_UTILS_NS::Path &rawPath) {
        return Load(rawPath, RawMeshParams());
    }

    RawMesh::Ptr RawMesh::Load(const SR_UTILS_NS::Path &rawPath, RawMeshParams params) {
        RawMesh::Ptr pRawMesh = nullptr;

        ResourceManager::Instance().Execute([&]() {
            Path&& id = Path(rawPath).RemoveSubPath(ResourceManager::Instance().GetResPath());

            auto&& paramsHash = SR_HASH(params);
            id = id.EmplaceFront(SR_FORMAT("{}|", paramsHash));

            if (auto&& pResource = ResourceManager::Instance().Find<RawMesh>(id)) {
                pRawMesh = pResource;
                SRAssert(pRawMesh->m_params == params);
                return;
            }

            pRawMesh = new RawMesh();
            pRawMesh->m_params = params;
            pRawMesh->SetId(id.ToStringRef(), false /** auto register */);

            if (!pRawMesh->Reload()) {
                SR_ERROR("RawMesh::Load() : failed to load raw mesh! \n\tPath: " + rawPath.ToString());
                pRawMesh->DeleteResource();
                pRawMesh = nullptr;
                return;
            }

            /// отложенная ручная регистрация
            ResourceManager::Instance().RegisterResource(pRawMesh);
        });

        return pRawMesh;
    }

    bool RawMesh::Unload() {
        bool hasErrors = !IResource::Unload();

    #ifdef SR_UTILS_ASSIMP
        if (m_importer) {
            m_importer->FreeScene();
        }

        if (m_fromCache) {
            delete m_scene;
            m_scene = nullptr;
        }

        m_animations.clear();
    #endif

        m_fromCache = false;

        m_indices.clear();
        m_bones.clear();
        m_optimizedBones.clear();

        m_boneOffsetsMap.clear();
        m_boneTransformsMap.clear();

        m_boneOffsets.clear();
        m_boneTransforms.clear();

        return !hasErrors;
    }

    bool RawMesh::Load() {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(GetResourcePath().ToStringRef());

        bool hasErrors = !IResource::Load();

        auto&& resPath = GetResourcePath();

        Path&& path = ResourceManager::Instance().GetResPath().Concat(resPath);
        Path&& cache = ResourceManager::Instance().GetCachePath().Concat("Models").Concat(resPath);

        if (m_params.animation) {
            cache = cache.ConcatExt("animation");
        }

        SR_MAYBE_UNUSED Path&& binary = cache.ConcatExt("cache");
        SR_MAYBE_UNUSED Path&& hashFile = cache.ConcatExt("hash");

        SR_MAYBE_UNUSED const uint64_t resourceHash = path.GetFileHash();

        SR_MAYBE_UNUSED const bool supportFastLoad = SR_UTILS_NS::Features::Instance().Enabled("FastModelsLoad", false);
        SR_MAYBE_UNUSED bool needFastLoad = supportFastLoad;

    #ifdef SR_UTILS_ASSIMP
    retry:
        if (needFastLoad && resourceHash == SR_UTILS_NS::FileSystem::ReadHashFromFile(hashFile)) {
            if ((m_scene = SR_UTILS_NS::AssimpCache::Instance().Load(binary))) {
                m_fromCache = true;
            }
            else {
                needFastLoad = false;
                goto retry;
            }
        }
        else {
            m_scene = m_importer->ReadFile(path.ToStringRef(), m_params.animation ? SR_RAW_MESH_ASSIMP_ANIMATION_FLAGS : SR_RAW_MESH_ASSIMP_FLAGS);

            if (!m_scene) {
                SR_ERROR("RawMesh::Load() : failed to load file!\n\tPath: " + path.ToStringRef() + "\n\tReason: " + std::string(m_importer->GetErrorString()));
                return false;
            }

            if (m_params.convexHull) {
                ComputeConvexHull();
            }

            NormalizeWeights();

            if (needFastLoad) {
                SR_LOG("RawMesh::Load() : export model to cache... \n\tPath: " + binary.ToString());

                Assimp::Exporter exporter;
                const aiExportFormatDesc* format = exporter.GetExportFormatDescription(14);

                exporter.Export(m_scene, format->id, binary.ToString(), m_params.animation ? SR_RAW_MESH_ASSIMP_ANIMATION_FLAGS : SR_RAW_MESH_ASSIMP_FLAGS);

                SR_UTILS_NS::FileSystem::WriteHashToFile(hashFile, resourceHash);
            }
        }

        if (m_scene && !m_fromCache && supportFastLoad) {
            SR_UTILS_NS::AssimpCache::Instance().Save(binary, m_scene);
        }

        if (m_scene) {
            CalculateBones();
            OptimizeSkeleton();
            CalculateOffsets();
            CalculateTransforms();
            CalculateAnimations();
        }
        else {
            SR_ERROR("RawMesh::Load() : failed to read file! \n\tPath: " + path.ToString() + "\n\tReason: " + m_importer->GetErrorString());
            hasErrors |= true;
        }
    #endif

        return !hasErrors;
    }

    uint32_t RawMesh::GetMeshesCount() const {
    #ifdef SR_UTILS_ASSIMP
        if (!m_scene) {
            SRHalt("RawMesh::GetMeshesCount() : assimp scene is invalid!");
            return 0;
        }

        return m_scene->mNumMeshes;
    #else
        return 0;
    #endif
    }

    std::string RawMesh::GetGeometryName(uint32_t id) const {
    #ifdef SR_UTILS_ASSIMP
        if (!m_scene || id >= m_scene->mNumMeshes) {
            SRAssert2(false, "Out of range or invalid scene!");
            return {};
        }

        return m_scene->mMeshes[id]->mName.C_Str();
    #else
        return std::string();
    #endif
    }

    std::vector<SR_UTILS_NS::Vertex> RawMesh::GetVertices(uint32_t id) const {
        std::vector<SR_UTILS_NS::Vertex> vertices;

    #ifdef SR_UTILS_ASSIMP
        if (!m_scene || id >= m_scene->mNumMeshes) {
            SRAssert2(false, "Out of range or invalid scene!");
            return {};
        }

        auto&& mesh = m_scene->mMeshes[id];

        vertices.reserve(mesh->mNumVertices);

        const bool hasUV = mesh->mTextureCoords[0];
        const bool hasNormals = mesh->mNormals;
        const bool hasTangents = mesh->mTangents;
        const bool hasBones = mesh->mBones;

        for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
            SR_UTILS_NS::Vertex vertex = SR_UTILS_NS::Vertex();
            vertex.position = *reinterpret_cast<Vec3*>(&mesh->mVertices[i]);
            vertex.uv = hasUV ? (*reinterpret_cast<Vec2*>(&mesh->mTextureCoords[0][i])) : Vec2 { 0.f, 0.f };
            //vertex.uv.x = -vertex.uv.x;
            vertex.normal = hasNormals ? (*reinterpret_cast<Vec3*>(&mesh->mNormals[i])) : Vec3 { 0.f, 0.f, 0.f };
            vertex.tangent = hasTangents ? (*reinterpret_cast<Vec3*>(&mesh->mTangents[i])) : Vec3 { 0.f, 0.f, 0.f };
            vertex.bitangent = hasTangents ? (*reinterpret_cast<Vec3*>(&mesh->mBitangents[i])) : Vec3 { 0.f, 0.f, 0.f };

            /*vertex.position.x = -vertex.position.x;
            vertex.normal.x = -vertex.normal.x;
            vertex.tangent.x = -vertex.tangent.x;
            vertex.bitangent.x = -vertex.bitangent.x;*/

            vertices.emplace_back(vertex);
        }

        //std::reverse(vertices.begin(), vertices.end());

        //for (uint32_t i = 0; i < vertices.size() / 3; i += 3) {
        //    std::swap(vertices[i + 1], vertices[i + 2]);
        //}

        if (hasBones) {
            auto&& bones = GetBones(id);

            bool hasWarn = false;

            for (uint32_t i = 0; i < mesh->mNumBones; i++) {
                for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
                    auto&& vertex = vertices[mesh->mBones[i]->mWeights[j].mVertexId];

                    vertex.weightsNum++;

                    if (vertex.weightsNum > SR_MAX_BONES_ON_VERTEX) {
                        if (!hasWarn) {
                            SR_WARN("RawMesh::GetVertices() : number of weights on vertex is already {}. Some weights will be omitted! VertexID = {}. weightsNum = {}",
                                SR_MAX_BONES_ON_VERTEX, mesh->mBones[i]->mWeights[j].mVertexId, vertex.weightsNum);
                            hasWarn = true;
                        }
                        continue;
                    }

                    auto&& boneIndex = bones.at(SR_UTILS_NS::StringAtom(mesh->mBones[i]->mName.C_Str()));

                    vertex.weights[vertex.weightsNum - 1].boneId = boneIndex;
                    vertex.weights[vertex.weightsNum - 1].weight = mesh->mBones[i]->mWeights[j].mWeight;
                }
            }

        #ifdef SR_DEBUG
            static bool hasError = false;

            if (!hasError) {
                for (auto&& vertex : vertices) {
                    float_t sum = 0.f;

                    for (auto&&[boneId, weight] : vertex.weights) {
                        sum += weight;
                    }

                    if (!SR_EQUALS(sum, 1.f)) {
                        SR_WARN("RawMesh::GetVertices() : incorrect mesh weight!\n\tPath: " + GetResourcePath().ToStringRef() +
                            "\n\tIndex: " + std::to_string(id) + "\n\tSum: " + std::to_string(sum)
                        );
                        hasError = true;
                        break;
                    }
                }
            }
        #endif
        }
    #endif

        return vertices;
    }

    const std::vector<uint32_t>& RawMesh::GetIndices(uint32_t id) const {
        SR_TRACY_ZONE;

        static std::vector<uint32_t> empty;

        if (id >= m_indices.size()) {
            SRHalt("Out of range!");
            return empty;
        }

        if (!m_indices[id].empty()) {
            return m_indices[id];
        }

        std::vector<uint32_t> indices;

    #ifdef SR_UTILS_ASSIMP
        if (!m_scene || id >= m_scene->mNumMeshes) {
            SRHalt("Out of range or invalid scene!");
            static std::vector<uint32_t> empty;
        }

        auto&& mesh = m_scene->mMeshes[id];

        indices.resize(mesh->mNumFaces * 3);
        uint32_t count = 0;

        for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
            const aiFace& face = mesh->mFaces[i];

            if (face.mNumIndices > 3) {
                SRHalt("Mesh isn't triangulated!");
                return empty;
            }

            memcpy(&indices[count], face.mIndices, sizeof(uint32_t) * face.mNumIndices);

            count += face.mNumIndices;
        }

        indices.resize(count);
    #endif

        m_indices[id] = std::move(indices);

        return m_indices[id];
    }

    uint32_t RawMesh::GetVerticesCount(uint32_t id) const {
    #ifdef SR_UTILS_ASSIMP
        if (!m_scene || id >= m_scene->mNumMeshes) {
            SRAssert2(false, "Out of range or invalid scene!");
            return {};
        }

        return m_scene->mMeshes[id]->mNumVertices;
    #else
        return 0;
    #endif
    }

    uint32_t RawMesh::GetIndicesCount(uint32_t id) const {
    #ifdef SR_UTILS_ASSIMP
        if (!m_scene || id >= m_scene->mNumMeshes) {
            SRAssert2(false, "Out of range or invalid scene!");
            return {};
        }

        auto&& mesh = m_scene->mMeshes[id];

        uint32_t sum = 0;

        for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
            sum += mesh->mFaces[i].mNumIndices;
        }

        return sum;
    #else
        return 0;
    #endif
    }

    float_t RawMesh::GetScaleFactor() const {
        float_t factor = 0.f;

    #ifdef SR_UTILS_ASSIMP
        if (m_scene->mMetaData->Get("UnitScaleFactor", factor))
           return static_cast<double_t>(factor);
    #endif

        SRHalt0();

        return 1.f;
    }

    SR_UTILS_NS::Path RawMesh::GetAssociatedPath() const {
        return ResourceManager::Instance().GetResPath();
    }

    uint32_t RawMesh::GetAnimationsCount() const {
    #ifdef SR_UTILS_ASSIMP
        if (!m_scene) {
            SRHalt("Invalid scene!");
            return 0;
        }

        return m_scene->mNumAnimations;
    #endif
        return 0;
    }

    const ska::flat_hash_map<SR_UTILS_NS::StringAtom, uint32_t>& RawMesh::GetBones(uint32_t id) const {
        static const auto&& def = ska::flat_hash_map<SR_UTILS_NS::StringAtom, uint32_t>();

        if (id >= m_bones.size()) {
            return def;
        }

        return m_bones.at(id);
    }

    const SR_MATH_NS::Matrix4x4& RawMesh::GetBoneOffset(SR_UTILS_NS::StringAtom name) const {
        static const auto&& def = SR_MATH_NS::Matrix4x4::Identity();

        auto&& pIt = m_boneOffsetsMap.find(name);
        if (pIt == m_boneOffsetsMap.end()) {
            return def;
        }

        return pIt->second;
    }

    const SR_MATH_NS::Matrix4x4& RawMesh::GetBoneTransform(uint32_t index) const {
        static const auto&& def = SR_MATH_NS::Matrix4x4::Identity();
        if (index >= m_boneTransforms.size()) SR_UNLIKELY_ATTRIBUTE {
            SRHalt("Out of range!");
            return def;
        }
        return m_boneTransforms[index];
    }

    const SR_MATH_NS::Matrix4x4& RawMesh::GetBoneTransform(SR_UTILS_NS::StringAtom name) const {
        static const auto&& def = SR_MATH_NS::Matrix4x4::Identity();

        auto&& pIt = m_boneTransformsMap.find(name);
        if (pIt == m_boneTransformsMap.end()) {
            return def;
        }

        return pIt->second;
    }

    void RawMesh::CalculateBones() {
    #ifdef SR_UTILS_ASSIMP
        m_bones.resize(m_scene->mNumMeshes);
        m_indices.resize(m_scene->mNumMeshes);

        for (uint32_t meshId = 0; meshId < m_scene->mNumMeshes; ++meshId) {
            auto&& pMesh = m_scene->mMeshes[meshId];

            for (uint32_t boneId = 0; boneId < pMesh->mNumBones; ++boneId) {
                auto&& name = SR_UTILS_NS::StringAtom(pMesh->mBones[boneId]->mName.data);

                if (m_bones[meshId].count(name) == 1) {
                    SR_WARN("RawMesh::CalculateBones() : bone already exists! \n\tName: " + name.ToString());
                    continue;
                }
                m_bones[meshId].insert(std::make_pair(name, static_cast<uint32_t>(m_bones[meshId].size())));
            }
        }
    #endif
    }

    void RawMesh::CalculateAnimations() {
    #ifdef SR_UTILS_ASSIMP
        if (!m_params.animation || !m_scene) {
            return;
        }

        for (uint32_t i = 0; i < m_scene->mNumAnimations; ++i) {
            auto&& pAnimation = m_scene->mAnimations[i];
            m_animations[SR_HASH_STR(pAnimation->mName.C_Str())] = pAnimation;
        }
    #endif
    }

    void RawMesh::OptimizeSkeleton() {
        m_optimizedBones.clear();

        for (auto&& mesh : m_bones) {
            for (auto&& [hashName, index] : mesh) {
                if (m_optimizedBones.count(hashName) == 1) {
                    index = m_optimizedBones[hashName];
                    continue;
                }
                m_optimizedBones[hashName] = index;
            }
        }
    }

    void RawMesh::CalculateOffsets() {
    #ifdef SR_UTILS_ASSIMP
        for (uint32_t meshId = 0; meshId < m_scene->mNumMeshes; ++meshId) {
            auto&& pMesh = m_scene->mMeshes[meshId];

            for (uint32_t boneId = 0; boneId < pMesh->mNumBones; ++boneId) {
                auto&& name = SR_UTILS_NS::StringAtom(pMesh->mBones[boneId]->mName.data);

                if (m_boneOffsetsMap.count(name) == 1) {
                    continue;
                }

                auto&& matrix = pMesh->mBones[boneId]->mOffsetMatrix;

                aiQuaternion rotation;
                aiVector3D scaling, translation;
                matrix.Decompose(scaling, rotation, translation);

                SR_MATH_NS::Matrix4x4 matrix4X4(
                        SR_MATH_NS::FVector3(translation.x, translation.y, translation.z),
                        SR_MATH_NS::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w),
                        SR_MATH_NS::FVector3(scaling.x, scaling.y, scaling.z)
                );

                m_boneOffsetsMap.insert(std::make_pair(name, matrix4X4));
            }
        }
    #endif

        m_boneOffsets.resize(m_boneOffsetsMap.size());

        for (auto&& [hashName, boneId] : m_optimizedBones) {
            if (boneId >= m_boneOffsets.size()) {
                m_boneOffsets.resize(boneId + 1);
            }
            m_boneOffsets[boneId] = GetBoneOffset(hashName);
        }
    }

    void RawMesh::CalculateTransforms() {
#ifdef SR_UTILS_ASSIMP
        std::map<SR_UTILS_NS::StringAtom, SR_MATH_NS::Matrix4x4> matrices;

        std::function<void(aiNode*, SR_MATH_NS::Matrix4x4)> processNode;

        processNode = [&matrices, &processNode](aiNode* pNode, SR_MATH_NS::Matrix4x4 matrix) {
            aiQuaternion rotation;
            aiVector3D scaling, translation;
            pNode->mTransformation.Decompose(scaling, rotation, translation);

            SR_MATH_NS::Matrix4x4 local(
                    SR_MATH_NS::FVector3(translation.x, translation.y, translation.z),
                    SR_MATH_NS::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w),
                    SR_MATH_NS::FVector3(scaling.x, scaling.y, scaling.z));

            matrices.insert(std::make_pair(SR_UTILS_NS::StringAtom(pNode->mName.C_Str()), local));

            for (uint32_t i = 0; i < pNode->mNumChildren; ++i) {
                processNode(pNode->mChildren[i], local);
            }
        };
        processNode(m_scene->mRootNode, SR_MATH_NS::Matrix4x4::Identity());

        for (uint32_t meshId = 0; meshId < m_scene->mNumMeshes; ++meshId) {
            auto&& pMesh = m_scene->mMeshes[meshId];

            for (uint32_t boneId = 0; boneId < pMesh->mNumBones; ++boneId) {
                auto&& name = SR_UTILS_NS::StringAtom(pMesh->mBones[boneId]->mName.data);

                if (m_boneTransformsMap.count(name) == 1 || matrices.count(name) == 0) {
                    continue;
                }

                m_boneTransformsMap.insert(std::make_pair(name, matrices.at(name)));
            }
        }
#endif

        m_boneTransforms.resize(m_boneTransformsMap.size());

        for (auto&& [hashName, boneId] : m_optimizedBones) {
            if (boneId >= m_boneTransforms.size()) {
                m_boneTransforms.resize(boneId + 1);
            }
            m_boneTransforms[boneId] = GetBoneTransform(hashName);
        }
    }

    uint32_t RawMesh::GetBoneIndex(SR_UTILS_NS::StringAtom name) const {
        auto&& pIt = m_optimizedBones.find(name);
        if (pIt == m_optimizedBones.end()) {
            return SR_ID_INVALID;
        }

        return pIt->second;
    }

    void RawMesh::NormalizeWeights() {
    #ifdef SR_UTILS_ASSIMP
        if (!m_scene) {
            SR_ERROR("RawMesh::NormalizeWeights() : scene is nullptr!");
            return;
        }

        for (uint32_t i = 0; i < m_scene->mNumMeshes; ++i) {
            NormalizeWeights(m_scene->mMeshes[i]);
        }
    #endif
    }

#ifdef SR_UTILS_ASSIMP
    uint32_t RawMesh::NormalizeWeights(const aiMesh* pMesh) {
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
#endif

    void RawMesh::ComputeConvexHull() {
        if (!m_params.convexHull) {
            return;
        }

        //std::vector<>

        for (uint16_t i = 0; i <= static_cast<uint16_t>(GetMeshesCount()); ++i) {
            auto&& computedVertices = SR_UTILS_NS::ComputeConvexHull(GetVertices(i));
            SR_NOOP;

        }

    }

    int32_t RawMesh::GetMeshId(SR_UTILS_NS::StringAtom name) const {
    #ifdef SR_UTILS_ASSIMP
        if (!m_scene) {
            SRHalt("Invalid scene!");
            return SR_ID_INVALID;
        }

        for (uint32_t i = 0; i < m_scene->mNumMeshes; ++i) {
            if (m_scene->mMeshes[i]->mName.C_Str() == name) {
                return static_cast<int32_t>(i);
            }
        }
    #endif
        return SR_ID_INVALID;
    }
}