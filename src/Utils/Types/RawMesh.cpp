//
// Created by Monika on 24.03.2022.
//

#include <Utils/Types/RawMesh.h>
#include <Utils/Common/Features.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/FileSystem/AssimpCache.h>
#include <Utils/Serialization/SRASerialization.h>
#include <Utils/Animations/SkeletonRig.h>
#include <Utils/TaskManager/TaskManager.h>

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
    { }

    RawMesh::~RawMesh() {
    #ifdef SR_UTILS_ASSIMP
        if (m_assimp.pImporter) {
            delete m_assimp.pImporter;
        }
    #endif
    }

    bool RawMesh::Unload() {
        bool hasErrors = !IResource::Unload();

        if (IsLoaded()) {
            SR_LOG("RawMesh::Unload() : unloading raw mesh \"{}\"...", GetResourceId());
        }

        if (m_syncLoadTaskId) {
            SR_LOG("RawMesh::Unload() : the model is still loading asynchronously! Waiting for the loading to finish...\n\tPath: {}", GetResourcePath());
            SR_UTILS_NS::TaskManager::Instance().Wait(*m_syncLoadTaskId);
            m_syncLoadTaskId = std::nullopt;
        }

    #ifdef SR_UTILS_ASSIMP
        if (m_assimp.pImporter) {
            m_assimp.pImporter->FreeScene();
        }
        m_assimp = {};
    #endif

        m_sceneStructure = {};
        m_metaInfo = RawMeshMetaInfo();

        return !hasErrors;
    }

    bool RawMesh::Load() {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT(GetResourcePath());

        bool hasErrors = !IResource::Load();

        LoadMetaInfo();

        if (m_syncLoadTaskId) {
            SRHalt("RawMesh::Load() : model is already in loading state!");
            return false;
        }

        const bool asyncLoadSupport = SR_UTILS_NS::Features::Instance().Enabled("ModelsAsyncLoad", true);

        if (asyncLoadSupport && GetMetaInfo().loadMode == ResourceLoadMode::Async) {
            m_syncLoadTaskId = SR_UTILS_NS::TaskManager::Instance().ExecuteAsync([this](auto&& state) {
                SR_TRACY_ZONE_N("Async model load");
                SR_TRACY_ZONE_TEXT(GetResourcePath());

                if (!m_metaInfo.rig.GetId().empty()) {
                    if (auto&& pRig = m_metaInfo.rig.GetResource()) {
                        if (pRig->GetSkeleton().GetRawMesh() && !pRig->GetSkeleton().GetRawMesh()->CanBeUsed()) {
                            state = SR_UTILS_NS::TaskState::Waiting;
                            return;
                        }
                    }
                }
                state = SR_UTILS_NS::TaskState::Launched;

                LoadMeshData(GetResourcePath(), m_sceneStructure, m_assimp, m_params);
                AddUsePoint();
                SR_UTILS_NS::TaskManager::Instance().ExecuteNextFrame([this, pStrong = GetThis()]() {
                    m_syncLoadTaskId = std::nullopt;
                    Broadcast(SR_UTILS_NS::IResource::RELOAD_BEGIN_EVENT);
                    Broadcast(SR_UTILS_NS::IResource::RELOAD_DONE_EVENT);
                    RemoveUsePoint();
                });
            }, SR_UTILS_NS::TaskPriority::High);
            SR_UTILS_NS::TaskManager::Instance().SetDescription(*m_syncLoadTaskId, GetResourceId());
            return true;
        }
        else if (!LoadMeshData(GetResourcePath(), m_sceneStructure, m_assimp, m_params)) {
            hasErrors = true;
        }

        return !hasErrors;
    }

    void RawMesh::LoadMetaInfo() {
        SR_TRACY_ZONE;

        m_metaInfo = RawMeshMetaInfo();

        if (m_params.animation) {
            return;
        }

        auto&& metaPath = ResourceManager::Instance().GetResPath().Concat(GetResourcePath()).ConcatExt(".meta");
        if (!metaPath.IsFile()) {
            return;
        }

        SR_UTILS_NS::SRADeserializer deserializer;
        if (!deserializer.LoadFromFile(metaPath) || !m_metaInfo.Load(deserializer)) {
            SR_WARN("RawMesh::LoadMetaInfo() : failed to load meta info from file: {}", metaPath);
            m_metaInfo = RawMeshMetaInfo();
        }
    }

    const RawMeshHolder* RawMesh::GetRigSkeleton() const {
        if (m_metaInfo.rig.GetId().empty()) {
            return nullptr;
        }

        auto&& pRig = m_metaInfo.rig.GetResource();
        if (!pRig) {
            SR_ERROR("RawMesh::GetRigSkeleton() : failed to load rig! Rig: {}\n\tPath: {}", m_metaInfo.rig.GetId(), GetResourcePath());
            return nullptr;
        }

        auto&& skeleton = pRig->GetSkeleton();
        if (!skeleton.IsValidMeshId()) {
            SR_ERROR("RawMesh::GetRigSkeleton() : rig has invalid skeleton! Rig: {}\n\tPath: {}", m_metaInfo.rig.GetId(), GetResourcePath());
            return nullptr;
        }

        return &skeleton;
    }

    uint32_t RawMesh::GetMeshesCount() const {
        return GetSceneStructure().GetMeshes().size();
    }

    StringView RawMesh::GetGeometryName(uint32_t id) const {
        if (GetResourceLoadState() == IResource::LoadState::Error) {
            return {};
        }

        const_cast<RawMesh&>(*this).ForceLoad();

    #ifdef SR_UTILS_ASSIMP
        if (SRVerify(GetAssimpScene() && id < m_assimp.pScene->mNumMeshes)) {
            return m_assimp.pScene->mMeshes[id]->mName.C_Str();
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

        auto&& meshes = GetSceneStructure().GetMeshes();
        if (id >= meshes.size()) {
            return empty;
        }

        if (!meshes[id].indices.empty()) {
            return meshes[id].indices;
        }

        SR_HTYPES_NS::FastMemoryArray<uint32_t> indices;

    #ifdef SR_UTILS_ASSIMP
        if (!m_assimp.pScene || id >= m_assimp.pScene->mNumMeshes) {
            SRHalt("Out of range or invalid scene!");
            return empty;
        }

        auto&& mesh = m_assimp.pScene->mMeshes[id];

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

        const_cast<MeshSceneStructure::MeshData&>(meshes[id]).indices = std::move(indices);
        return meshes[id].indices;
    }

    uint32_t RawMesh::GetVerticesCount(uint32_t id) const {
        if (GetResourceLoadState() == IResource::LoadState::Error) {
            return 0;
        }

    #ifdef SR_UTILS_ASSIMP
        if (!m_assimp.pScene || id >= m_assimp.pScene->mNumMeshes) {
            SRHalt("Out of range or invalid scene!");
            return {};
        }

        return m_assimp.pScene->mMeshes[id]->mNumVertices;
    #else
        return 0;
    #endif
    }

    uint32_t RawMesh::GetIndicesCount(uint32_t id) const {
        if (GetResourceLoadState() == IResource::LoadState::Error) {
            return 0;
        }

    #ifdef SR_UTILS_ASSIMP
        if (!m_assimp.pScene || id >= m_assimp.pScene->mNumMeshes) {
            SRAssert2(false, "Out of range or invalid scene!");
            return {};
        }

        auto&& mesh = m_assimp.pScene->mMeshes[id];

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
        auto&& meshes = GetSceneStructure().GetMeshes();
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
        if (m_assimp.pScene && m_assimp.pScene->mMetaData->Get("UnitScaleFactor", factor))
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
        if (!m_assimp.pScene) {
            SRHalt("Invalid scene!");
            return 0;
        }

        return m_assimp.pScene->mNumAnimations;
    #endif
        return 0;
    }

    Vector<SR_UTILS_NS::StringAtom> RawMesh::GetAnimationNames() const {
    #ifdef SR_UTILS_ASSIMP
        if (!m_assimp.pScene) {
            SRHalt("Invalid scene!");
            return {};
        }

        Vector<SR_UTILS_NS::StringAtom> names;
        names.reserve(m_assimp.pScene->mNumAnimations);

        for (uint32_t i = 0; i < m_assimp.pScene->mNumAnimations; ++i) {
            names.emplace_back(m_assimp.pScene->mAnimations[i]->mName.C_Str());
        }

        return names;
    #endif
        return {};
    }

    const MeshSceneStructure::MeshData& RawMesh::GetMeshData(uint32_t id) const {
        static const MeshSceneStructure::MeshData empty;
        auto&& meshes = GetSceneStructure().GetMeshes();
        if (id >= meshes.size()) {
            return empty;
        }
        return meshes[id];
    }

    int32_t RawMesh::GetMeshId(SR_UTILS_NS::StringAtom name) const {
    #ifdef SR_UTILS_ASSIMP
        if (!GetAssimpScene()) {
            SRHalt("Invalid scene!");
            return SR_ID_INVALID;
        }

        for (uint32_t i = 0; i < m_assimp.pScene->mNumMeshes; ++i) {
            if (m_assimp.pScene->mMeshes[i]->mName.C_Str() == name) {
                return static_cast<int32_t>(i);
            }
        }
    #endif
        return SR_ID_INVALID;
    }

#ifdef SR_UTILS_ASSIMP
    const void* RawMesh::GetAssimpScene() const noexcept {
        if (m_syncLoadTaskId) {
            SRHalt("RawMesh::GetAssimpScene() : the model is still loading asynchronously!");
            return nullptr;
        }
        return m_assimp.pScene;
    }
#endif

    bool RawMesh::IsAllowedToRevive() const {
        return true;
    }

    bool RawMesh::CanBeUsed() const {
        if (m_syncLoadTaskId) {
            return false;
        }
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
        auto&& meshes = GetSceneStructure().GetMeshes();
        if (!m_assimp.pScene || id >= meshes.size()) {
            SRHalt("Out of range or invalid scene!");
            return empty;
        }

        auto&& vertexBuffers = const_cast<MeshSceneStructure::MeshData&>(meshes[id]).vertexBuffers;
        auto&& pIt = std::ranges::find_if(vertexBuffers, [&layout](const SR_UTILS_NS::VertexDataBuffer& buffer) {
            return buffer.layout.Compare(layout);
        });

        if (pIt != vertexBuffers.end()) {
            return *pIt;
        }

        vertexBuffers.reserve(8);
        if (vertexBuffers.empty()) {
            auto&& bones = GetMeshData(id).bones;
            auto&& pRigSkeleton = GetRigSkeleton();
            auto&& buffer = SR_UTILS_NS::VertexDataBuffer::AllocateFromAssimp(m_assimp.pScene->mMeshes[id], [&bones, pRigSkeleton](SR_UTILS_NS::StringAtom name)-> uint32_t {
                if (pRigSkeleton) {
                    auto&& rigBoneInfo = pRigSkeleton->GetRawMesh()->GetBoneInfo(pRigSkeleton->GetMeshId(), name);
                    return rigBoneInfo.boneId.has_value() ? rigBoneInfo.boneId.value() : SR_ID_INVALID;
                }
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
        if (m_syncLoadTaskId) {
            SRHalt("RawMesh::GetSceneStructure() : the model is still loading asynchronously!");
            static MeshSceneStructure def;
            return def;
        }
        return m_sceneStructure;
    }

    const SR_MATH_NS::AABB& RawMesh::GetAABB(uint32_t id) const {
        static const SR_MATH_NS::AABB empty;
        auto&& meshes = GetSceneStructure().GetMeshes();
        if (id >= meshes.size()) {
            return empty;
        }
        return meshes[id].aabb;
    }

    bool RawMesh::LoadMeshData(const Path& resPath, MeshSceneStructure& sceneStructure, MeshAssimpInfo& assimp, const RawMeshParams& params) {
        Path&& path = CoreResLoader::GetResPath().Concat(resPath);
        Path&& cache = CoreResLoader::GetCachePath().Concat("Models").Concat(resPath);

        if (params.animation) {
            cache = cache.ConcatExt("animation");
        }

        SR_MAYBE_UNUSED Path&& binary = cache.ConcatExt("cache");
        SR_MAYBE_UNUSED Path&& hashFile = cache.ConcatExt("hash");

        SR_MAYBE_UNUSED const uint64_t resourceHash = path.GetFileHash();
        SR_MAYBE_UNUSED bool supportFastLoad = SR_UTILS_NS::Features::Instance().Enabled("FastModelsLoad", false);
        SR_MAYBE_UNUSED bool needFastLoad = supportFastLoad;

    #ifdef SR_UTILS_ASSIMP
        bool fromCache = false;
    retry:
        if (needFastLoad && resourceHash == SR_UTILS_NS::FileSystem::ReadHashFromFile(hashFile)) {
            if ((assimp.pScene = SR_UTILS_NS::AssimpCache::Instance().Load(binary, assimp.mappedFileCache))) {
                fromCache = true;
            }
            else {
                needFastLoad = false;
                goto retry;
            }
        }
        else {
            String buffer;
            if (!SR_UTILS_NS::FileSystem::ReadFile(path, buffer)) {
                SR_ERROR("RawMesh::LoadMeshData() : failed to read file!\n\tPath: {}", path);
                return false;
            }

            if (!assimp.pImporter) {
                assimp.pImporter = new Assimp::Importer();
            }
            assimp.pScene = assimp.pImporter->ReadFileFromMemory(buffer.data(), buffer.size(), params.animation ? SR_RAW_MESH_ASSIMP_ANIMATION_FLAGS : SR_RAW_MESH_ASSIMP_FLAGS);

            if (!assimp.pScene) {
                SR_ERROR("RawMesh::LoadMeshData() : failed to load file!\n\tPath: {}\n\tReason: {}", path, assimp.pImporter->GetErrorString());
                return false;
            }

            for (uint32_t i = 0; i < assimp.pScene->mNumMeshes; ++i) {
                AssimpTools::NormalizeWeights(assimp.pScene->mMeshes[i]);
            }

            if (needFastLoad) {
                SR_UTILS_NS::FileSystem::WriteHashToFile(hashFile, resourceHash);
            }
        }

        if (assimp.pScene && !fromCache && supportFastLoad) {
            SR_UTILS_NS::AssimpCache::Instance().Save(binary, assimp.pScene);
        }

        if (assimp.pScene) {
            sceneStructure.ImportFromAssimp(assimp.pScene, params.animation);
        }
        else {
            SR_ERROR("RawMesh::LoadMeshData() : failed to read file! \n\tPath: " + path.ToString() + "\n\tReason: " + assimp.pImporter->GetErrorString());
            return false;
        }
        return true;
    #endif

        return false;
    }

    void RawMesh::ForceLoad() {
        if (CanBeUsed()) {
            return;
        }
        SR_LOG("RawMesh::ForceLoad() : force loading raw mesh \"{}\"...", GetResourceId());
        SR_UTILS_NS::TaskManager::Instance().Wait(*m_syncLoadTaskId);
        SR_UTILS_NS::TaskManager::Instance().Update();
    }

    const IResourceVariant* RawMesh::GetVariant() const {
        return &m_params;
    }

    bool RawMeshParams::operator==(const RawMeshParams &rhs) const {
        return animation == rhs.animation && convexHull == rhs.convexHull;
    }

    SRHashType RawMeshParams::GetHash() const {
        return SR_HASH(*this);
    }
}