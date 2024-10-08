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
    }

    Prefab* Prefab::Load(const Path& rawPath) {
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

    bool Prefab::Unload() {
        if (m_data) {
            m_data->Destroy();
            m_data = nullptr;
        }

        return IResource::Unload();
    }

    bool Prefab::Load() {
        Path&& path = Path(GetResourceId());
        if (!path.IsAbs()) {
            path = ResourceManager::Instance().GetResPath().Concat(path);
        }

        auto&& marshal = SR_HTYPES_NS::Marshal::Load(path);
        if (!marshal.Valid()) {
            SR_ERROR("Prefab::Load() : failed to load marshal data!\n\tPath: " + path.ToString());
            return false;
        }

        /// TODO: implement load other types of data
        m_data = SR_UTILS_NS::GameObject::Load(marshal, nullptr).StaticCast<SceneObject>();

        if (!m_data.Valid()) {
            m_loadState = LoadState::Error;
            SR_ERROR("Prefab::Load() : failed to load game object from marshal data!");
            return false;
        }

        return IResource::Load();
    }

    Prefab::SceneObjectPtr Prefab::Instance(const Prefab::ScenePtr& scene) const {
        if (m_data) {
            auto&& pInstanced = m_data->Copy(scene, nullptr);
            pInstanced->SetPrefab(const_cast<Prefab*>(this), true);
            return pInstanced;
        }

        return Prefab::SceneObjectPtr();
    }
}
