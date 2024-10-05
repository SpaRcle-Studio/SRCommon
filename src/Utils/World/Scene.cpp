//
// Created by Nikita on 30.11.2020.
//

#include <Utils/World/Scene.h>

#include <Utils/World/Region.h>
#include <Utils/World/Chunk.h>
#include <Utils/World/SceneAllocator.h>
#include <Utils/World/SceneLogic.h>
#include <Utils/World/SceneCubeChunkLogic.h>
#include <Utils/World/SceneDefaultLogic.h>
#include <Utils/World/SceneUpdater.h>
#include <Utils/World/ScenePrefabLogic.h>

#include <Utils/Types/RawMesh.h>

#include <Utils/ECS/Component.h>
#include <Utils/ECS/GameObject.h>

#include <Utils/Platform/Platform.h>

namespace SR_WORLD_NS {
    Scene::Scene()
        : Super(this)
        , m_sceneUpdater(new SR_WORLD_NS::SceneUpdater(this))
    { }

    Scene::~Scene() {
        SRAssert(m_isDestroyed);

        if (Debug::Instance().GetLevel() >= Debug::Level::Low) {
            SR_LOG("Scene::~Scene() : free \"" + GetName() + "\" scene pointer...");
        }

        SRAssert(m_newQueue.empty());
        SRAssert(m_deleteQueue.empty());
        SRAssert(m_destroyedComponents.empty());
        SRAssert(m_freeObjIndices.size() == m_sceneObjects.size());

        SR_SAFE_DELETE_PTR(m_sceneUpdater);
    }

    GameObject::Ptr Scene::InstanceGameObject(SR_UTILS_NS::StringAtom name) {
        if (Debug::Instance().GetLevel() >= Debug::Level::High) {
            SR_LOG("Scene::Instance() : instance \"" + name.ToStringRef() + "\" game object at \"" + GetName() + "\" scene.");
        }

        const GameObject::Ptr pGameObject = new GameObject(name);
        const SceneObject::Ptr pSceneObject = pGameObject.StaticCast<SceneObject>();

        RegisterSceneObject(pSceneObject);
        return pGameObject;
    }

    GameObject::Ptr Scene::FindOrInstanceGameObject(SR_UTILS_NS::StringAtom name) {
        if (auto&& pFound = Find(name).DynamicCast<GameObject>()) {
            return pFound;
        }

        return InstanceGameObject(name);
    }

    Scene::SceneObjectPtr Scene::Instance(SR_HTYPES_NS::Marshal& marshal) {
        /// TODO: Implement laod other types of objects
        return GameObject::Load(marshal, this).StaticCast<SceneObject>();
    }

    Scene::Ptr Scene::Empty() {
        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::Empty() : creating new empty scene...");
        }

        auto&& scene = SceneAllocator::Instance().Allocate();

        if (!scene) {
            SR_ERROR("Scene::New() : failed to allocate scene!");
            return Scene::Ptr();
        }

        scene->m_logic = new SceneDefaultLogic(scene);

        return scene;
    }

    Scene::Ptr Scene::New(const Path& path) {
        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::New() : creating new scene...");
        }

        if (SR_PLATFORM_NS::IsExists(path)) {
            SRHalt("Scene::New() : scene already exists!\n\tPath: " + path.ToString());
            return Scene::Ptr();
        }

        auto&& scene = SceneAllocator::Instance().Allocate();

        if (!scene) {
            SR_ERROR("Scene::New() : failed to allocate scene!");
            return Scene::Ptr();
        }

        SRAssert(!path.IsAbs());

        scene->m_absPath = GetAbsPath(path);
        scene->m_path = path;
        scene->m_logic = SceneLogic::CreateByExt(scene, path.GetExtension());

        return scene;
    }

    Scene::Ptr World::Scene::Load(const Path& path) {
        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::Load() : loading scene...\n\tPath: " + path.ToString());
        }

        auto&& scene = SceneAllocator::Instance().Allocate();

        if (!scene) {
            SR_ERROR("Scene::Load() : failed to allocate scene!");
            return Scene::Ptr();
        }

        SRAssert(!path.IsAbs());

        scene->m_absPath = GetAbsPath(path);
        scene->m_path = path;
        scene->m_logic = SceneLogic::CreateByExt(scene, path.GetExtension());

        if (!scene->m_logic->Load(scene->m_absPath)) {
            SR_ERROR("Scene::Load() : failed to load scene logic!");

            scene.AutoFree([](SR_WORLD_NS::Scene* pScene) {
                pScene->Destroy();
                delete pScene;
            });

            return Scene::Ptr();
        }

        return scene;
    }

    bool Scene::Destroy() {
        if (m_isDestroyed) {
            SR_ERROR("Scene::Destroy() : scene \"" + GetName() + "\" already destroyed!");
            return false;
        }

        m_isPreDestroyed = true;

        IComponentable::DestroyComponents();

        m_logic.AutoFree([](auto&& pLogic) {
            pLogic->Destroy();
            delete pLogic;
        });

        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            const uint64_t count = m_sceneObjects.size() - m_freeObjIndices.size();
            SR_LOG("Scene::Destroy() : complete unloading!");
            SR_LOG("Scene::Destroy() : destroying \"" + GetName() + "\" scene contains "+ std::to_string(count) +" game objects...");
        }

        for (const auto pObject : GetRootSceneObjects()) { // NOLINT нужно удалять только по копии
            pObject->Destroy();
        }

        Prepare();

        if (m_sceneObjects.size() != m_freeObjIndices.size()) {
            SRHalt("Scene::Destroy() : after destroying the root objects, "
                                       "there are {} objects left!", m_sceneObjects.size() - m_freeObjIndices.size());
        }

        m_isDestroyed = true;
        m_isHierarchyChanged = true;

        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::Destroy() : scene successfully destroyed!");
        }

        return true;
    }

    Scene::SceneObjects& Scene::GetRootSceneObjects() {
        if (!m_isHierarchyChanged) {
            return m_rootObjects;
        }

        m_rootObjects.clear();
        m_rootObjects.reserve(m_sceneObjects.size() / 2);

        for (auto&& gameObject : m_sceneObjects) {
            if (!gameObject) {
                continue;
            }

            if (!gameObject->GetParent()) {
                m_rootObjects.emplace_back(gameObject);
            }
        }

        m_isHierarchyChanged = false;

        return m_rootObjects;
    }

    SceneObject::Ptr Scene::FindByComponent(const std::string &name) {
        for (auto&& pObject : m_sceneObjects) {
            if (pObject->ContainsComponent(name)) {
                return pObject;
            }
        }

        return SceneObject::Ptr();
    }

    void Scene::OnChanged() {
        m_isHierarchyChanged = true;
    }

    bool Scene::Save() {
        return SaveAt(m_path);
    }

    bool Scene::SaveAt(const Path& path) {
        SR_INFO(SR_FORMAT("Scene::SaveAt() : saving scene...\n\tPath: {}", path.CStr()));

        SRAssert(!path.IsAbs());

        if (m_path.GetExtensionView() != path.GetExtensionView()) {
            SR_ERROR("Scene::SaveAt() : different extensions!\n\tSave path: " + path.ToString() + "\n\tScene path: " + m_path.ToString());
            return false;
        }

        if (!m_logic->Save(GetAbsPath(path))) {
            SR_ERROR("Scene::SaveAt() : failed to save scene logic!");
            return false;
        }

        return true;
    }

    bool Scene::Remove(const SceneObjectPtr& gameObject) {
        SRAssert(!m_isDestroyed);

        m_deleteQueue.emplace_back(gameObject);

        const uint64_t idInScene = gameObject->GetIdInScene();

        if (idInScene >= m_sceneObjects.size()) {
            for (auto pIt = m_newQueue.begin(); pIt != m_newQueue.end(); ++pIt) {
                if (*pIt == gameObject) {
                    m_newQueue.erase(pIt);

                    SetDirty(true);
                    OnChanged();

                    return true;
                }
            }

            SRHalt("Scene::Remove() : invalid game object id!");
            return false;
        }

        if (m_sceneObjects.at(idInScene) != gameObject) {
            SRHalt("Scene::Remove() : game objects do not match!");
            return false;
        }

        m_sceneObjects.at(idInScene) = SceneObject::Ptr();
        m_freeObjIndices.emplace_back(idInScene);

        SetDirty(true);
        OnChanged();

        return true;
    }

    SceneObject::Ptr Scene::Instance(const SR_HTYPES_NS::RawMesh* pRawMesh) {
        SRHalt("Method isn't implemented!");
        return SceneObject::Ptr();
    }

    SceneObject::Ptr Scene::InstanceFromFile(const std::string& path) {
        auto&& extension = SR_UTILS_NS::StringUtils::GetExtensionFromFilePath(path);

        if (extension == Prefab::EXTENSION) {
            auto&& pPrefab = Prefab::Load(path);

            if (pPrefab) {
                auto&& instanced = pPrefab->Instance(this);
                pPrefab->CheckResourceUsage();
                return instanced;
            }

            return SceneObject::Ptr();
        }

        if (auto&& raw = SR_HTYPES_NS::RawMesh::Load(path)) {
            SceneObject::Ptr root = Instance(raw);

            if (raw->GetCountUses() == 0) {
                raw->Destroy();
            }

            return root;
        }

        return SceneObject::Ptr();
    }

    bool Scene::Reload() {
        SR_INFO("Scene::Reload() : reload scene...");
        return m_logic->Reload();
    }

    SceneObject::Ptr Scene::Find(uint64_t hashName) {
        for (auto&& pObject : m_sceneObjects) {
            /// блокировать объекты не нужно, так как уничтожиться они могут только из сцены
            /// Но стоит предусмотреть защиту от одновременного изменения имени
            if (pObject && pObject->GetName() == hashName) {
                return pObject;
            }
        }

        return SceneObject::Ptr();
    }

    Scene::SceneObjectPtr Scene::Find(SR_UTILS_NS::StringAtom name) {
        return Find(name.GetHash());
    }

    SceneObject::Ptr Scene::Find(const std::string& name) {
        return Find(SR_HASH_STR(name));
    }

    SceneObject::Ptr Scene::Find(const char* name) {
        return Find(SR_UTILS_NS::StringAtom(name));
    }

    std::string Scene::GetName() const {
        return m_path.GetBaseName();
    }

    bool Scene::IsPrefab() const {
        return m_logic.DynamicCast<ScenePrefabLogic>();
    }

    void Scene::RegisterSceneObject(const Scene::SceneObjectPtr& ptr) {
        SRAssert(!m_isPreDestroyed);
        SRAssert(!ptr->GetScene());

        m_newQueue.emplace_back(ptr);

        ptr->SetScene(this);

        for (auto&& pChild : ptr->GetChildrenRef()) {
            RegisterSceneObject(pChild);
        }

        SetDirty(true);
        OnChanged();
    }

    void Scene::Prepare() {
        SR_TRACY_ZONE;

        if (auto&& pLogic = GetLogicBase()) {
            pLogic->PostLoad();
        }

        if (!m_deleteQueue.empty() || !m_newQueue.empty() || !m_destroyedComponents.empty()) {
            SetDirty(true);
            OnChanged();
        }

        if (m_isPreDestroyed) {
            while (!m_newQueue.empty()) {
                auto&& pGameObject = m_newQueue.front();

                if (pGameObject) {
                    pGameObject->Destroy();
                }
                else {
                    m_newQueue.pop_front();
                }
            }
        } 
        else {
            for (auto&& gameObject : m_newQueue) {
                const uint64_t id = m_freeObjIndices.empty() ? m_sceneObjects.size() : m_freeObjIndices.front();

                gameObject->SetIdInScene(id);

                if (m_freeObjIndices.empty()) {
                    m_sceneObjects.emplace_back(gameObject);
                }
                else {
                    m_sceneObjects[m_freeObjIndices.front()] = gameObject;
                    m_freeObjIndices.erase(m_freeObjIndices.begin());
                }
            }
        }

        m_newQueue.clear();

        for (auto&& gameObject : m_deleteQueue) {
            gameObject->DestroyComponents();
        }

        for (auto&& pComponent : m_destroyedComponents) {
            pComponent->OnDestroy();
        }

        m_destroyedComponents.clear();

        for (auto&& gameObject : m_deleteQueue) {
            gameObject->DestroyImpl();
        }

        m_deleteQueue.clear();
    }

    void Scene::Remove(Component* pComponent) {
        m_destroyedComponents.emplace_back(pComponent);
    }

    Path Scene::GetAbsPath() const {
        return m_absPath;
    }

    bool Scene::IsExists(const Path& path) {
        return GetAbsPath(path).Exists();
    }

    Path Scene::GetAbsPath(const Path& path) {
        if (path.Contains(RuntimeScenePath.ToStringRef()) ||
            path.Contains(NewScenePath.ToStringRef()) ||
            path.Contains(NewPrefabPath.ToStringRef())
        ) {
            return SR_UTILS_NS::ResourceManager::Instance().GetCachePath().Concat(path);
        }

        return SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat(path);
    }

    bool Scene::SetDirty(bool dirty) {
        m_sceneUpdater->SetDirty();
        return IComponentable::SetDirty(dirty);
    }

    void Scene::Init() {
        if (m_logic) {
            m_logic->Init();
        }
    }
}