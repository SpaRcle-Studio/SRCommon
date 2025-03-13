//
// Created by Monika on 23.12.2022.
//

#include <Utils/ECS/Prefab.h>
#include <Utils/Resources/ResourceManager.h>

namespace SR_UTILS_NS {
    Prefab::Prefab()
        : IResource(SR_COMPILE_TIME_CRC32_TYPE_NAME(Prefab))
    { }

    Prefab::~Prefab() {
        if (m_data) {
            m_data->Destroy();
            m_data = nullptr;
        }
        SR_SAFE_DELETE_PTR(m_pDeserializer);
    }

    Prefab* Prefab::Load(const Path& rawPath) {
        SR_TRACY_ZONE;

        Prefab* pResource = nullptr;

        ResourceManager::Instance().Execute([&pResource, &rawPath]() {
            Path&& path = Path(rawPath).RemoveSubPath(ResourceManager::Instance().GetResPath());

            if ((pResource = ResourceManager::Instance().Find<Prefab>(path))) {
                return;
            }

            pResource = new Prefab();

            pResource->SetId(path.ToStringRef(), false /** auto register */);

            if (!pResource->Reload()) {
                SR_ERROR("RawMesh::Load() : failed to load prefab! \n\tPath: " + path.ToString());
                pResource->DeleteResource();
                pResource = nullptr;
                return;
            }

            /// отложенная ручная регистрация
            ResourceManager::Instance().RegisterResource(pResource);
        });

        return pResource;
    }

    bool Prefab::LoadToSO(const SceneObjectPtr& pSO) {
        SR_TRACY_ZONE;

        if (!m_data || !m_pDeserializer) {
            SR_ERROR("Prefab::LoadToSO() : prefab data or deserializer is nullptr!");
            return false;
        }

        if (!SRVerify2(pSO, "Prefab::LoadToSO() : scene object is nullptr!")) {
            return false;
        }

        m_pDeserializer->ResetWalker();

        if (!m_pDeserializer->BeginObject(SerializationId::Create("data"))) {
            SR_ERROR("Prefab::LoadToSO() : failed to load prefab data!");
            return false;
        }

        pSO->Load(*m_pDeserializer);

        m_pDeserializer->EndObject();

        return true;
    }

    bool Prefab::Unload() {
        if (m_data) {
            m_data->Destroy();
            m_data = nullptr;
        }

        SR_SAFE_DELETE_PTR(m_pDeserializer);

        return IResource::Unload();
    }

    bool Prefab::Load() {
        Path&& path = Path(GetResourceId());
        if (!path.IsAbs()) {
            path = ResourceManager::Instance().GetResPath().Concat(path);
        }

        SR_SAFE_DELETE_PTR(m_pDeserializer);
        m_pDeserializer = new SR_UTILS_NS::SRADeserializer();

        if (!m_pDeserializer->LoadFromFile(path)) {
            m_loadState = LoadState::Error;
            SR_ERROR("Prefab::Load() : failed to load prefab!\n\tPath: " + path.ToString());
            return false;
        }

        if (!m_pDeserializer->BeginObject(SerializationId::Create("info"))) {
            m_loadState = LoadState::Error;
            SR_ERROR("Prefab::Load() : failed to load prefab info!\n\tPath: " + path.ToString());
            return false;
        }

        SR_UTILS_NS::StringAtom type;
        m_pDeserializer->ReadString(type, SerializationId::Create("type"));

        m_pDeserializer->EndObject();

        if (type.Empty()) {
            m_loadState = LoadState::Error;
            SR_ERROR("Prefab::Load() : prefab type is empty!\n\tPath: " + path.ToString());
            return false;
        }

        m_data = SR_UTILS_NS::Factory::Instance().Create<SceneObject>(type);
        if (!m_data) {
            m_loadState = LoadState::Error;
            SR_ERROR("Prefab::Load() : failed to create scene object from type: " + type.ToString());
            return false;
        }

        if (m_pDeserializer->BeginObject(SerializationId::Create("data"))) {
            m_data->Load(*m_pDeserializer);
            m_pDeserializer->EndObject();
        }
        else {
            m_loadState = LoadState::Error;
            SR_ERROR("Prefab::Load() : failed to load prefab data!\n\tPath: " + path.ToString());
            return false;
        }

        return IResource::Load();
    }

    Prefab::SceneObjectPtr Prefab::Instance(const Prefab::ScenePtr& pScene) const {
        SR_TRACY_ZONE;

        if (m_data) {
            auto&& pClone = m_data->CloneSceneObject();
            pClone->SetPrefab(const_cast<Prefab*>(this), true);
            pScene->RegisterSceneObject(pClone);
            return pClone;
        }

        SRHalt("Prefab::Instance() : prefab data is nullptr!");

        return nullptr;
    }
}
