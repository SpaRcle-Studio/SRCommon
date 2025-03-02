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

#include <Enum/SceneLogicType.hpp>

#include <Codegen/Scene.generated.hpp>

namespace SR_WORLD_NS {
    Scene::Scene()
        : Super()
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

    Scene::Ptr Scene::CreateEmptyScene() {
        if (Debug::Instance().GetLevel() >= Debug::Level::High) {
            SR_LOG("Scene::CreateEmptyScene() : creating empty scene...");
        }

        auto&& pScene = SceneAllocator::Instance().Allocate();

        if (!pScene) {
            SR_ERROR("Scene::CreateEmptyScene() : failed to allocate scene!");
            return Scene::Ptr();
        }

        return pScene;
    }

    GameObject::Ptr Scene::InstanceGameObject(SR_UTILS_NS::StringAtom name) {
        if (Debug::Instance().GetLevel() >= Debug::Level::High) {
            SR_LOG("Scene::InstanceGameObject() : instance \"" + name.ToStringRef() + "\" game object at \"" + GetName() + "\" scene.");
        }

        const GameObject::Ptr pGameObject = SRNew<GameObject>();
        pGameObject->SetName(name);

        RegisterSceneObject(pGameObject.StaticCast<SceneObject>());

        return pGameObject;
    }

    GameObject::Ptr Scene::FindOrInstanceGameObject(SR_UTILS_NS::StringAtom name) {
        if (auto&& pFound = Find(name).DynamicCast<GameObject>()) {
            return pFound;
        }

        return InstanceGameObject(name);
    }

    void Scene::SetPath(const Path& path) {
        m_path = path;
        m_absPath = GetAbsPath(path);
    }

    Scene::Ptr Scene::NewScene(const Path& path, SceneLogicType type) {
        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::New() : creating new scene...");
        }

        if (SR_PLATFORM_NS::IsExists(path)) {
            SRHalt("Scene::New() : scene already exists!\n\tPath: " + path.ToString());
            return Scene::Ptr();
        }

        auto&& pLogic = SR_UTILS_NS::Factory::Instance().Create<SceneLogic>("Scene{}Logic"_format(type));
        if (!pLogic) {
            SR_ERROR("Scene::New() : failed to create scene logic! Type: {}", type);
            return Scene::Ptr();
        }

        auto&& pScene = SceneAllocator::Instance().Allocate();

        if (!pScene) {
            SR_ERROR("Scene::New() : failed to allocate scene!");
            return Scene::Ptr();
        }

        SRAssert(!path.IsAbs());

        pScene->SetPath(path);
        pScene->m_logic = pLogic;
        pLogic->SetScene(pScene);

        return pScene;
    }

    Scene::Ptr Scene::LoadScene(const Path& path) {
        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::Load() : loading scene...\n\tPath: " + path.ToString());
        }

        static auto&& destroySceneFn = [](Scene::Ptr& pScene) {
            pScene.AutoFree([](SR_WORLD_NS::Scene* pScene) {
                pScene->Destroy();
                delete pScene;
            });
        };

        auto&& pScene = SceneAllocator::Instance().Allocate();

        if (!pScene) {
            SR_ERROR("Scene::Load() : failed to allocate scene!");
            return Scene::Ptr();
        }

        SRAssert(!path.IsAbs());

        pScene->SetPath(path);

        SRADeserializer deserializer;
        if (!deserializer.LoadFromFile(pScene->m_absPath)) {
            SR_ERROR("Scene::Load() : failed to load scene!\n\tPath: " + pScene->m_absPath.ToString());
            destroySceneFn(pScene);
            return Scene::Ptr();
        }

        pScene->Load(deserializer);

        if (!pScene->m_logic || !pScene->m_logic->LoadLogic(deserializer, pScene->m_absPath)) {
            SR_ERROR("Scene::Load() : failed to load scene logic!");
            destroySceneFn(pScene);
            return Scene::Ptr();
        }

        return pScene;
    }

    bool Scene::Destroy() {
        if (m_isDestroyed) {
            SR_ERROR("Scene::Destroy() : scene \"" + GetName() + "\" already destroyed!");
            return false;
        }

        m_isPreDestroyed = true;

        DestroyComponents();

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
            return m_root;
        }

        m_root.clear();
        m_root.reserve(m_sceneObjects.size() / 2);

        for (auto&& gameObject : m_sceneObjects) {
            if (!gameObject) {
                continue;
            }

            if (!gameObject->GetParent()) {
                m_root.emplace_back(gameObject);
            }
        }

        m_isHierarchyChanged = false;

        return m_root;
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

    bool Scene::SaveScene() {
        return SaveSceneAt(m_path);
    }

    bool Scene::SaveSceneAt(const Path& path) {
        SR_INFO(SR_FORMAT("Scene::SaveAt() : saving scene...\n\tPath: {}", path.CStr()));

        SRAssert(!path.IsAbs());

        if (m_path.GetExtensionView() != path.GetExtensionView()) {
            SR_ERROR("Scene::SaveAt() : different extensions!\n\tSave path: " + path.ToString() + "\n\tScene path: " + m_path.ToString());
            return false;
        }

        SRASerializer serializer;
        serializer.SetUseTabs(true);
        Save(serializer);

        if (!m_logic->SaveLogic(serializer, GetAbsPath(path))) {
            SR_ERROR("Scene::SaveAt() : failed to save scene logic!");
            return false;
        }

        if (!serializer.SaveToFile(m_logic->GetSceneDataPath(GetAbsPath(path)))) {
            SR_ERROR("Scene::SaveAt() : failed to save scene!");
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

    SceneObject::Ptr Scene::InstanceFromFile(const SR_UTILS_NS::Path& path) {
        if (path.GetExtensionView() == Prefab::EXTENSION) {
            if (auto&& pPrefab = Prefab::Load(path)) {
                auto&& pInstanced = pPrefab->Instance(this);
                pPrefab->CheckResourceUsage();
                return pInstanced;
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

    void Scene::OnPostLoad() {
        if (m_logic) {
            m_logic->SetScene(this);
        }

        for (auto&& pObject : m_root) {
            if (SRVerify2(pObject, "Scene::OnPostLoad() : invalid root object!")) {
                RegisterSceneObject(pObject);
            }
        }

        Super::OnPostLoad();
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

    bool Scene::IsPrefab() const noexcept {
        return m_logic.DynamicCast<ScenePrefabLogic>();
    }

    void Scene::RegisterSceneObject(const Scene::SceneObjectPtr& pSO) {
        SR_TRACY_ZONE;

        SRAssert2(m_registerEntityCache.empty(), "Scene::RegisterSceneObject() : cache is not empty!");

        RegisterSceneObjectImpl(pSO);
        pSO->SetScene(this);

        auto&& entityManager = SR_UTILS_NS::EntityManager::Instance();

        for (auto& entities : m_registerEntityCache | std::views::values) {
            m_registerEntityIdReplaceCache.clear();

            for (auto&& pEntity : entities) {
                const EntityId oldEntityId = pEntity->GetEntityId();
                pEntity->SetEntityId(ENTITY_ID_MAX);
                const EntityId newEntityId = entityManager.Register(pEntity, oldEntityId);

                if (oldEntityId != ENTITY_ID_MAX) {
                    m_registerEntityIdReplaceCache[newEntityId] = oldEntityId;
                }
            }

            if (!m_registerEntityIdReplaceCache.empty()) {
                for (auto&& pEntity : entities) {
                    pEntity->OnEntityIdReplaced(m_registerEntityIdReplaceCache);
                }
            }
        }

        m_registerEntityCache.clear();
    }

    void Scene::RegisterSceneObjectImpl(const Scene::SceneObjectPtr& pSO) {
        SR_TRACY_ZONE;

        SRAssert2(!m_isPreDestroyed, "Scene::RegisterSceneObjectImpl() : scene is pre destroyed!");
        SRAssert2(!pSO->GetScene(), "Scene::RegisterSceneObjectImpl() : object already registered!");
        SRAssert2(!pSO->IsEntityRegistered(), "Scene::RegisterSceneObjectImpl() : entity already registered!");

        m_newQueue.emplace_back(pSO);

        m_registerEntityCache[pSO->GetPrefab()].emplace_back(static_cast<Entity*>(const_cast<SceneObject*>(pSO.Get())));
        for (auto&& pComponent : pSO->GetComponents()) {
            m_registerEntityCache[pSO->GetPrefab()].emplace_back(static_cast<Entity*>(const_cast<Component*>(pComponent.Get())));
        }

        for (auto&& pChild : pSO->GetChildrenRef()) {
            RegisterSceneObjectImpl(pChild);
        }

        SetDirty(true);
        OnChanged();
    }

    void Scene::Prepare() {
        SR_TRACY_ZONE;

        if (auto&& pLogic = GetLogicBase()) {
            pLogic->Prepare();
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

    void Scene::Remove(const Component::Ptr& pComponent) {
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