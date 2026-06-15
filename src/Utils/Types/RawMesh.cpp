//
// Created by Monika on 24.03.2022.
//

#include <Utils/Types/RawMesh.h>
#include <Utils/Common/Features.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/FileSystem/AssimpCache.h>

#ifdef SR_UTILS_ASSIMP
    #include <Utils/Resources/AssimpTools.h>

    #include <assimp/scene.h>
    #include <assimp/postprocess.h>
    #include <assimp/Importer.hpp>
    #include <assimp/config.h>
    #include <assimp/include/assimp/Exporter.hpp>
    #include <assimp/include/assimp/cexport.h>
#endif

#include <Codegen/RawMesh.generated.hpp>

namespace SR_HTYPES_NS {
#ifdef SR_UTILS_ASSIMP
    /// aiProcess_GenNormals
    SR_INLINE_STATIC int SR_RAW_MESH_ASSIMP_FLAGS = aiProcess_GenSmoothNormals | aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_SortByPType | aiProcess_GlobalScale;
    SR_INLINE_STATIC int SR_RAW_MESH_ASSIMP_ANIMATION_FLAGS = aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder;
#endif

    RawMesh::RawMesh()
        : Super()
    {
    #ifdef SR_UTILS_ASSIMP
        m_importer = new Assimp::Importer();
    #endif
    }

    RawMesh::~RawMesh() {
    #ifdef SR_UTILS_ASSIMP
        delete m_importer;

        if (m_fromCache) {
            m_pMappedFileCache.Reset();
            m_scene = nullptr;
        }
    #endif
    }

    bool RawMesh::Unload() {
        bool hasErrors = !IResource::Unload();

    #ifdef SR_UTILS_ASSIMP
        if (m_importer) {
            m_importer->FreeScene();
        }

        if (m_fromCache) {
            m_pMappedFileCache.Reset();
            m_scene = nullptr;
        }

        m_animations.clear();
    #endif

        m_meshes.clear();

        m_fromCache = false;

        m_optimizedBones.clear();

        m_boneOffsetsMap.clear();
        m_boneOffsets.clear();

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

        SR_MAYBE_UNUSED bool supportFastLoad = SR_UTILS_NS::Features::Instance().Enabled("FastModelsLoad", false);
        SR_MAYBE_UNUSED bool needFastLoad = supportFastLoad;

    #ifdef SR_UTILS_ASSIMP
    retry:
        if (needFastLoad && resourceHash == SR_UTILS_NS::FileSystem::ReadHashFromFile(hashFile)) {
            if ((m_scene = SR_UTILS_NS::AssimpCache::Instance().Load(binary, m_pMappedFileCache))) {
                m_fromCache = true;
            }
            else {
                needFastLoad = false;
                goto retry;
            }
        }
        else {
            String buffer;
            if (!SR_UTILS_NS::FileSystem::ReadFile(path, buffer)) {
                SR_ERROR("RawMesh::Load() : failed to read file!\n\tPath: {}", path);
                return false;
            }

            if (m_importer) {
                std::string ext = path.GetExtension();
                std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                /// Mixamo/FBX часто используют pivot/pre-rotation.
                /// Для МЕША preserve pivots обычно нужен (иначе skinning/offsets могут разъехаться и геометрия "схлопнется").
                /// Для АНИМАЦИИ preserve pivots часто ломает каналы (кости "выворачивает"), поэтому отключаем только в animation-режиме.
                if (ext == "fbx") {
                    //m_importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, m_params.animation ? false : true);
                }
            }

            m_scene = m_importer->ReadFileFromMemory(buffer.data(), buffer.size(), m_params.animation ? SR_RAW_MESH_ASSIMP_ANIMATION_FLAGS : SR_RAW_MESH_ASSIMP_FLAGS);

            if (!m_scene) {
                SR_ERROR("RawMesh::Load() : failed to load file!\n\tPath: {}\n\tReason: {}", path, m_importer->GetErrorString());
                return false;
            }

            if (m_params.convexHull) {
                ComputeConvexHull();
            }

            NormalizeWeights();

            if (needFastLoad) {
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

    std::string_view RawMesh::GetGeometryName(uint32_t id) const {
        if (GetResourceLoadState() == IResource::LoadState::Error) {
            return {};
        }

    #ifdef SR_UTILS_ASSIMP
        if (SRVerify(m_scene && id < m_scene->mNumMeshes)) {
            return m_scene->mMeshes[id]->mName.C_Str();
        }
    #endif
        return std::string_view();
    }

    const SR_HTYPES_NS::FastMemoryArray<uint32_t>& RawMesh::GetIndices(uint32_t id) const {
        SR_TRACY_ZONE;

        static SR_HTYPES_NS::FastMemoryArray<uint32_t> empty;

        if (id >= m_meshes.size()) {
            SRHalt("Out of range!");
            return empty;
        }

        if (!m_meshes[id].indices.empty()) {
            return m_meshes[id].indices;
        }

        SR_HTYPES_NS::FastMemoryArray<uint32_t> indices;

    #ifdef SR_UTILS_ASSIMP
        if (!m_scene || id >= m_scene->mNumMeshes) {
            SRHalt("Out of range or invalid scene!");
            return empty;
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

        m_meshes[id].indices = std::move(indices);
        return m_meshes[id].indices;
    }

    uint32_t RawMesh::GetVerticesCount(uint32_t id) const {
        if (GetResourceLoadState() == IResource::LoadState::Error) {
            return 0;
        }

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
        if (GetResourceLoadState() == IResource::LoadState::Error) {
            return 0;
        }

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
        if (GetResourceLoadState() == IResource::LoadState::Error) {
            return 1.f;
        }

        float_t factor = 0.f;

    #ifdef SR_UTILS_ASSIMP
        if (m_scene && m_scene->mMetaData->Get("UnitScaleFactor", factor))
           return static_cast<float_t>(factor);
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

    Vector<SR_UTILS_NS::StringAtom> RawMesh::GetAnimationNames() const {
    #ifdef SR_UTILS_ASSIMP
        if (!m_scene) {
            SRHalt("Invalid scene!");
            return {};
        }

        Vector<SR_UTILS_NS::StringAtom> names;
        names.reserve(m_scene->mNumAnimations);

        for (uint32_t i = 0; i < m_scene->mNumAnimations; ++i) {
            names.emplace_back(m_scene->mAnimations[i]->mName.C_Str());
        }

        return names;
    #endif
        return {};
    }

    const SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, uint32_t>& RawMesh::GetBones(uint32_t id) const {
        static const auto&& def = SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, uint32_t>();
        if (id >= m_meshes.size()) {
            return def;
        }
        return m_meshes.at(id).bones;
    }

    const SR_MATH_NS::Matrix4x4& RawMesh::GetBoneOffset(SR_UTILS_NS::StringAtom name) const {
        static const auto&& def = SR_MATH_NS::Matrix4x4::Identity();

        auto&& pIt = m_boneOffsetsMap.find(name);
        if (pIt == m_boneOffsetsMap.end()) {
            return def;
        }

        return pIt->second;
    }

    void RawMesh::CalculateBones() {
        SR_TRACY_ZONE;
    #ifdef SR_UTILS_ASSIMP
        m_meshes.resize(m_scene->mNumMeshes);

        for (uint32_t meshId = 0; meshId < m_scene->mNumMeshes; ++meshId) {
            auto&& pMesh = m_scene->mMeshes[meshId];
            auto&& bones = m_meshes[meshId].bones;

            for (uint32_t boneId = 0; boneId < pMesh->mNumBones; ++boneId) {
                auto&& name = SR_UTILS_NS::StringAtom(pMesh->mBones[boneId]->mName.data);

                if (bones.count(name) == 1) {
                    SR_WARN("RawMesh::CalculateBones() : bone already exists! \n\tName: " + name.ToString());
                    continue;
                }

                const auto size = static_cast<uint32_t>(bones.size());
                bones.insert(std::make_pair(name, size));
            }
        }
    #endif
    }

    void RawMesh::CalculateAnimations() {
        SR_TRACY_ZONE;

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
        SR_TRACY_ZONE;

        m_optimizedBones.clear();

        for (auto&& mesh : m_meshes) {
            for (auto&& [hashName, index] : mesh.bones) {
                if (m_optimizedBones.count(hashName) == 1) {
                    index = m_optimizedBones[hashName];
                    continue;
                }
                m_optimizedBones[hashName] = index;
            }
        }
    }

    void RawMesh::CalculateOffsets() {
        SR_TRACY_ZONE;

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

    uint32_t RawMesh::GetBoneIndex(SR_UTILS_NS::StringAtom name) const {
        auto&& pIt = m_optimizedBones.find(name);
        if (pIt == m_optimizedBones.end()) {
            return SR_ID_INVALID;
        }

        return pIt->second;
    }

    void RawMesh::NormalizeWeights() {
        SR_TRACY_ZONE;

    #ifdef SR_UTILS_ASSIMP
        if (SRVerify(m_scene)) {
            for (uint32_t i = 0; i < m_scene->mNumMeshes; ++i) {
                AssimpTools::NormalizeWeights(m_scene->mMeshes[i]);
            }
        }
    #endif
    }

    void RawMesh::ComputeConvexHull() {
        SR_TRACY_ZONE;
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

    const Vector<SR_MATH_NS::Matrix4x4>& RawMesh::GetBoneOffsets() const {
        return m_boneOffsets;
    }

    const SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, uint16_t> &RawMesh::GetOptimizedBones() const {
        return m_optimizedBones;
    }

#ifdef SR_UTILS_ASSIMP
    const void* RawMesh::GetAssimpScene() const noexcept {
        return m_scene;
    }
#endif

    bool RawMesh::IsAllowedToRevive() const {
        return true;
    }

    std::string_view RawMesh::GetRootBoneName() const {
    #ifdef SR_UTILS_ASSIMP
        if (SRVerify(m_scene && m_scene->mRootNode)) {
            return std::string_view(m_scene->mRootNode->mName.C_Str(), m_scene->mRootNode->mName.length);
        }
    #endif
        return {};
    }

    void RawMesh::SetVariant(const SR_UTILS_NS::IResourceVariant& variant) {
        m_params = static_cast<const RawMeshParams&>(variant);
    }

    const SR_UTILS_NS::VertexDataBuffer& RawMesh::GetVertexBuffer(uint32_t id, const SR_UTILS_NS::VertexLayoutDescription& layout) const {
        SR_TRACY_ZONE;

        static const auto&& empty = SR_UTILS_NS::VertexDataBuffer();
        if (GetResourceLoadState() == IResource::LoadState::Error) {
            return empty;
        }

    #ifdef SR_UTILS_ASSIMP
        if (!m_scene || id >= m_scene->mNumMeshes) {
            SRAssert2(false, "Out of range or invalid scene!");
            return empty;
        }

        m_meshes.resize(m_scene->mNumMeshes);
        auto&& vertexBuffers = m_meshes[id].vertexBuffers;
        auto&& pIt = std::ranges::find_if(vertexBuffers, [&layout](const SR_UTILS_NS::VertexDataBuffer& buffer) {
            return buffer.layout.Compare(layout);
        });

        if (pIt != vertexBuffers.end()) {
            return *pIt;
        }

        vertexBuffers.reserve(8);
        if (vertexBuffers.empty()) {
            auto&& buffer = SR_UTILS_NS::VertexDataBuffer::AllocateFromAssimp(m_scene->mMeshes[id], GetBones(id));
            vertexBuffers.emplace_back(std::move(buffer));
        }

        if (layout.attributesCount == 0) {
            return vertexBuffers.front();
        }

        vertexBuffers.emplace_back(vertexBuffers.front().TransitionToLayout(layout));
        return vertexBuffers.back();
    #else
        return empty;
    #endif
    }

    bool RawMesh::HasBones(uint32_t id) const {
        return !GetBones(id).empty();
    }

    bool RawMeshParams::operator==(const RawMeshParams &rhs) const {
        return animation == rhs.animation && convexHull == rhs.convexHull;
    }

    SRHashType RawMeshParams::GetHash() const {
        return SR_HASH(*this);
    }
}