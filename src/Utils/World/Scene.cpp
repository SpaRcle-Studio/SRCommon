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
#include <Utils/Serialization/SRASerialization.h>
#include <Utils/FileSystem/FileSystem.h>

#include <Enum/SceneLogicType.hpp>

#include <Codegen/Scene.generated.hpp>

namespace SR_WORLD_NS {
    Scene::Scene()
        : Super()
        , m_sceneUpdater(new SR_WORLD_NS::SceneUpdater(this))
        , m_pEntityController(SR_UTILS_NS::EntityController::MakeShared())
    { }

    Scene::~Scene() {
        SRAssert(m_isDestroyed || !m_isInitialized);

        if (m_isInitialized && Debug::Instance().GetLevel() >= Debug::Level::Low) {
            SR_LOG("Scene::~Scene() : free \"" + GetName() + "\" scene pointer...");
        }

        SRAssert(m_newQueue.empty());
        SRAssert(m_deleteQueue.empty());
        SRAssert(m_destroyedComponents.empty());
        SRAssert(m_freeObjIndices.size() == m_sceneObjects.size());

        SR_SAFE_DELETE_PTR(m_sceneUpdater);

        if (IsEntityRegistered()) {
            m_pEntityController->Unregister(GetEntityId());
        }
        m_pEntityController.AutoFree();
    }

    Scene::Ptr Scene::CreateEmptyScene() {
        if (Debug::Instance().GetLevel() >= Debug::Level::High) {
            SR_LOG("Scene::CreateEmptyScene() : creating empty scene...");
        }

        auto&& pScene = SceneAllocator::Instance().Allocate();

        if (!pScene) {
            SR_ERROR("Scene::CreateEmptyScene() : failed to allocate scene!");
            return nullptr;
        }

        return pScene;
    }

    GameObject::Ptr Scene::InstanceGameObject(SR_UTILS_NS::StringAtom name) {
        if (Debug::Instance().GetLevel() >= Debug::Level::High) {
            SR_LOG("Scene::InstanceGameObject() : instance \"" + name.ToStringRef() + "\" game object at \"" + GetName() + "\" scene.");
        }

        const GameObject::Ptr pGameObject = SRNew<GameObject>();
        pGameObject->SetName(name);

        RegisterSceneObject(StaticPointerCast<SceneObject>(pGameObject));

        return pGameObject;
    }

    GameObject::Ptr Scene::FindOrInstanceGameObject(SR_UTILS_NS::StringAtom name) {
        if (auto&& pFound = DynamicPointerCast<GameObject>(Find(name))) {
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

        if (SR_UTILS_NS::FileSystem::IsFileExists(path)) {
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

    Scene::Ptr Scene::LoadScene(const Path& rawPath) {
        SR_TRACY_ZONE;

        auto&& resourceManager = SR_UTILS_NS::ResourceManager::Instance();
        const SR_UTILS_NS::Path path = rawPath.RemoveSubPath(resourceManager.GetResPathRef());

        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            SR_LOG("Scene::Load() : loading scene...\n\tPath: " + path.ToString());
        }

        static auto&& destroySceneFn = [](Scene::Ptr& pScene) {
            pScene.AutoFree([](SR_WORLD_NS::Scene* pScene, auto&& pControl) {
                pScene->Destroy();
                pControl->deleter(static_cast<void*>(pScene));
            });
        };

        auto&& pScene = SceneAllocator::Instance().Allocate();

        if (!pScene) {
            SR_ERROR("Scene::Load() : failed to allocate scene!");
            return Scene::Ptr();
        }

        pScene->SetPath(path);

        SRADeserializer deserializer;
        if (!deserializer.LoadFromFile(pScene->m_absPath)) {
            SR_ERROR("Scene::Load() : failed to load scene!\n\tPath: " + pScene->m_absPath.ToString());
            destroySceneFn(pScene);
            return Scene::Ptr();
        }

        pScene->Load(deserializer);

        if (!pScene->IsEntityRegistered()) {
            const EntityId entityId = pScene->GetEntityId();
            pScene->SetEntityId(SR_ID_INVALID);
            pScene->GetEntityController()->Register(StaticPointerCast<Entity>(pScene), entityId);
        }

        if (!pScene->m_logic || !pScene->m_logic->LoadLogic(deserializer, pScene->m_absPath)) {
            SR_ERROR("Scene::Load() : failed to load scene logic!");
            destroySceneFn(pScene);
            return Scene::Ptr();
        }

        return pScene;
    }

    bool Scene::Destroy() {
        SR_TRACY_ZONE;

        if (m_isDestroyed) {
            SR_ERROR("Scene::Destroy() : scene \"" + GetName() + "\" already destroyed!");
            return false;
        }

        m_isPreDestroyed = true;

        RemoveComponents();

        m_logic.AutoFree([](auto&& pLogic, auto&& pControl) {
            pLogic->Destroy();
            pControl->deleter(static_cast<void*>(pLogic));
        });

        if (Debug::Instance().GetLevel() > Debug::Level::None) {
            const uint64_t count = m_sceneObjects.size() - m_freeObjIndices.size();
            SR_LOG("Scene::Destroy() : complete unloading!");
            SR_LOG("Scene::Destroy() : destroying \"" + GetName() + "\" scene contains "+ std::to_string(count) +" game objects...");
        }

        for (const auto pObject : GetRootSceneObjects()) { // NOLINT нужно удалять только по копии
            pObject->Destroy();
        }

        if (m_isInitialized) {
            Prepare();
        }

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
        SR_TRACY_ZONE;

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
        SR_TRACY_ZONE;
        return SaveSceneAt(m_path);
    }

    bool Scene::SaveSceneAt(const Path& path) {
        SR_TRACY_ZONE;
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

    SceneObject::Ptr Scene::Instance(const SR_HTYPES_NS::RawMesh*) {
        SRHalt("Method isn't implemented!");
        return SceneObject::Ptr();
    }

    SceneObject::Ptr Scene::InstanceFromFile(const SR_UTILS_NS::Path& path) {
        if (path.GetExtensionView() == Prefab::GetMetaStatic()->GetExtension()) {
            if (auto&& pPrefab = CoreResLoader::Load<Prefab>(path)) {
                auto&& pInstanced = pPrefab->Instance(this);
                pPrefab->CheckResourceUsage();
                return pInstanced;
            }

            return SceneObject::Ptr();
        }

        if (auto&& raw = CoreResLoader::Load<SR_HTYPES_NS::RawMesh>(path)) {
            SceneObject::Ptr root = Instance(raw.Get());

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
        SR_TRACY_ZONE;

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
        return DynamicPointerCast<ScenePrefabLogic>(m_logic);
    }

    void Scene::RegisterSceneObject(const Scene::SceneObjectPtr& pSO) {
        SR_TRACY_ZONE;

        SRAssert2(m_registerEntityCache.empty(), "Scene::RegisterSceneObject() : cache is not empty!");

        RegisterSceneObjectImpl(pSO);
        pSO->SetScene(this);

        pSO->OnRootRegistered();

        EntityController& entityController = *GetEntityController();

        for (const auto& entities : m_registerEntityCache | std::views::values) {
            m_registerEntityIdReplaceCache.clear();

            for (auto&& pEntity : entities) {
                const EntityId oldEntityId = pEntity->GetEntityId();
                pEntity->SetEntityId(SR_ID_INVALID);
                const EntityId newEntityId = entityController.Register(pEntity, oldEntityId);

                if (oldEntityId != SR_ID_INVALID) {
                    m_registerEntityIdReplaceCache[oldEntityId] = newEntityId;
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

        if (m_isInitialized) {
            m_newQueue.emplace_back(pSO);
        }
        else {
            ProcessNewSO(pSO);
        }

        m_registerEntityCache[pSO->GetPrefab().GetRawPtr()].emplace_back(static_cast<Entity*>(const_cast<SceneObject*>(pSO.Get())));
        for (auto&& pComponent : pSO->GetComponents()) {
            m_registerEntityCache[pSO->GetPrefab().GetRawPtr()].emplace_back(static_cast<Entity*>(const_cast<Component*>(pComponent.Get())));
        }

        for (auto&& pChild : pSO->GetChildrenRef()) {
            RegisterSceneObjectImpl(pChild);
        }

        SetDirty(true);
        OnChanged();
    }

    void Scene::Prepare() {
        SR_TRACY_ZONE;

        SRAssert2(m_isInitialized, "Scene::Prepare() : scene is not initialized!");

        if (auto&& pLogic = GetLogicBase()) {
            pLogic->Prepare();
        }

        if (!m_deleteQueue.empty() || !m_newQueue.empty() || !m_destroyedComponents.empty()) {
            SetDirty(true);
            OnChanged();
        }

        if (m_isPreDestroyed) {
            while (!m_newQueue.empty()) {
                if (auto&& pSO = m_newQueue.front()) {
                    pSO->Destroy();
                }
                else {
                    m_newQueue.pop_front();
                }
            }
        } 
        else {
            for (auto&& pSO : m_newQueue) {
                ProcessNewSO(pSO);
            }
        }

        m_newQueue.clear();

        for (auto&& pSO : m_deleteQueue) {
            pSO->RemoveComponents();
        }

        for (auto&& pComponent : m_destroyedComponents) {
            pComponent->OnDestroy();
        }

        m_destroyedComponents.clear();

        for (auto&& pSO : m_deleteQueue) {
            pSO->DestroyImpl();
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

    Scene::SceneLogicPtr Scene::GetLogicBase() const {
        return m_logic;
    }

    Path Scene::GetAbsPath(const Path& path) {
        if (path.Contains(RuntimeScenePath) ||
            path.Contains(NewScenePath) ||
            path.Contains(NewPrefabPath)
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
            m_logic->InitLogic();
        }

        m_isInitialized = true;
    }

    void Scene::ProcessNewSO(const Scene::SceneObjectPtr &pSO) {
        const uint64_t id = m_freeObjIndices.empty() ? m_sceneObjects.size() : m_freeObjIndices.front();

        pSO->SetIdInScene(id);

        if (m_freeObjIndices.empty()) {
            m_sceneObjects.emplace_back(pSO);
        }
        else {
            m_sceneObjects[m_freeObjIndices.front()] = pSO;
            m_freeObjIndices.erase(m_freeObjIndices.begin());
        }
    }

    SR_HTYPES_NS::DataStorage &Scene::GetDataStorage() {
        return m_dataStorage;
    }

    const SR_HTYPES_NS::DataStorage &Scene::GetDataStorage() const {
        return m_dataStorage;
    }

    const SR_HTYPES_NS::SharedPtr<SR_UTILS_NS::EntityController>& Scene::GetEntityController() const {
        return m_pEntityController;
    }

    bool Scene::IsEditorMode() const {
        static const StringAtom editorModeKey = "EditorMode";
        return GetDataStorage().GetValueDef<bool>(editorModeKey, false);
    }

    Scene::GameObjectPtr Scene::GetMainCamera() const {
        return nullptr;
    }
}