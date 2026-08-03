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
    #include <assimp/include/assimp/Exporter.hpp>
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
    #endif

        m_sceneStructure = {};
        m_fromCache = false;

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

            m_scene = m_importer->ReadFileFromMemory(buffer.data(), buffer.size(), m_params.animation ? SR_RAW_MESH_ASSIMP_ANIMATION_FLAGS : SR_RAW_MESH_ASSIMP_FLAGS);

            if (!m_scene) {
                SR_ERROR("RawMesh::Load() : failed to load file!\n\tPath: {}\n\tReason: {}", path, m_importer->GetErrorString());
                return false;
            }

            if (m_params.convexHull) {
                ComputeConvexHull();
            }

            for (uint32_t i = 0; i < m_scene->mNumMeshes; ++i) {
                AssimpTools::NormalizeWeights(m_scene->mMeshes[i]);
            }

            if (needFastLoad) {
                SR_UTILS_NS::FileSystem::WriteHashToFile(hashFile, resourceHash);
            }
        }

        if (m_scene && !m_fromCache && supportFastLoad) {
            SR_UTILS_NS::AssimpCache::Instance().Save(binary, m_scene);
        }

        if (m_scene) {
            m_sceneStructure.ImportFromAssimp(m_scene, m_params.animation);
        }
        else {
            SR_ERROR("RawMesh::Load() : failed to read file! \n\tPath: " + path.ToString() + "\n\tReason: " + m_importer->GetErrorString());
            hasErrors |= true;
        }
    #endif

        return !hasErrors;
    }

    uint32_t RawMesh::GetMeshesCount() const {
        return m_sceneStructure.GetMeshes().size();
    }

    StringView RawMesh::GetGeometryName(uint32_t id) const {
        if (GetResourceLoadState() == IResource::LoadState::Error) {
            return {};
        }

    #ifdef SR_UTILS_ASSIMP
        if (SRVerify(m_scene && id < m_scene->mNumMeshes)) {
            return m_scene->mMeshes[id]->mName.C_Str();
        }
    #endif
        return StringView();
    }

    const SR_UTILS_NS::Vector<SR_MATH_NS::Matrix4x4>& RawMesh::GetBoneOffsetMatrices(uint32_t id) const {
        static const SR_UTILS_NS::Vector<SR_MATH_NS::Matrix4x4> empty;
        if (!SRVerify(id < GetMeshesCount())) {
            return empty;
        }

        auto&& mesh = GetMeshData(id);
        if (!mesh.maxBoneId) {
            return empty;
        }

        auto&& cache = mesh.boneOffsetMatricesCache;
        if (!cache.empty()) {
            return cache;
        }

        auto&& bones = GetMeshData(id).bones;
        cache.resize(mesh.maxBoneId.value() + 1);
        for (const auto& [boneName, boneInfo] : bones) {
            cache[boneInfo.boneId.value()] = boneInfo.offsetMatrix;
        }
        return cache;
    }

    const SR_HTYPES_NS::FastMemoryArray<uint32_t>& RawMesh::GetIndices(uint32_t id) const {
        SR_TRACY_ZONE;

        static SR_HTYPES_NS::FastMemoryArray<uint32_t> empty;

        auto&& meshes = m_sceneStructure.GetMeshes();
        if (id >= meshes.size()) {
            return empty;
        }

        if (!meshes[id].indices.empty()) {
            return meshes[id].indices;
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

        meshes[id].indices = std::move(indices);
        return meshes[id].indices;
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

    const MeshSceneStructure::BoneInfo& RawMesh::GetBoneInfo(uint32_t id, SR_UTILS_NS::StringAtom name) const {
        static const MeshSceneStructure::BoneInfo defValue;
        auto&& meshes = m_sceneStructure.GetMeshes();
        if (id >= meshes.size()) {
            return defValue;
        }

        auto&& bones = meshes[id].bones;
        if (auto&& pIt = bones.find(name); pIt != bones.end()) {
            return pIt->second;
        }
        return defValue;
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

    const MeshSceneStructure::MeshData& RawMesh::GetMeshData(uint32_t id) const {
        static const MeshSceneStructure::MeshData empty;
        auto&& meshes = m_sceneStructure.GetMeshes();
        if (id >= meshes.size()) {
            return empty;
        }
        return meshes[id];
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

#ifdef SR_UTILS_ASSIMP
    const void* RawMesh::GetAssimpScene() const noexcept {
        return m_scene;
    }
#endif

    bool RawMesh::IsAllowedToRevive() const {
        return true;
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
        auto&& meshes = m_sceneStructure.GetMeshes();
        if (!m_scene || id >= meshes.size()) {
            SRAssert2(false, "Out of range or invalid scene!");
            return empty;
        }

        auto&& vertexBuffers = meshes[id].vertexBuffers;
        auto&& pIt = std::ranges::find_if(vertexBuffers, [&layout](const SR_UTILS_NS::VertexDataBuffer& buffer) {
            return buffer.layout.Compare(layout);
        });

        if (pIt != vertexBuffers.end()) {
            return *pIt;
        }

        vertexBuffers.reserve(8);
        if (vertexBuffers.empty()) {
            auto&& bones = GetMeshData(id).bones;
            auto&& buffer = SR_UTILS_NS::VertexDataBuffer::AllocateFromAssimp(m_scene->mMeshes[id], [&bones](SR_UTILS_NS::StringAtom name)-> uint32_t {
                if (auto&& pIt = bones.find(name); pIt != bones.end() && pIt->second.boneId.has_value()) {
                    return pIt->second.boneId.value();
                }
                return SR_ID_INVALID;
            });
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
        return !GetMeshData(id).bones.empty();
    }

    const MeshSceneStructure& RawMesh::GetSceneStructure() const {
        return m_sceneStructure;
    }

    bool RawMeshParams::operator==(const RawMeshParams &rhs) const {
        return animation == rhs.animation && convexHull == rhs.convexHull;
    }

    SRHashType RawMeshParams::GetHash() const {
        return SR_HASH(*this);
    }
}