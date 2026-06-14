//
// Created by Nikita on 30.11.2020.
//

#ifndef SR_ENGINE_SCENE_H
#define SR_ENGINE_SCENE_H

#include <Utils/ECS/IComponentable.h>

#include <Utils/Types/StringAtom.h>
#include <Utils/Types/DataStorage.h>
#include <Utils/Types/Vector.h>

#include <Utils/World/SceneLogicType.h>
#include <Utils/World/CameraData.h>

namespace SR_UTILS_NS {
    class SceneObject;
    class Prefab;
    class EntityController;
    class Component;
    class GameObject;
}

namespace SR_HTYPES_NS {
    class RawMesh;
}

namespace SR_WORLD_NS {
    class SceneLogic;
    class SceneUpdater;

    class SR_COMMON_DLL_API Scene : public SR_UTILS_NS::IComponentable {
        SR_CLASS()
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<Scene>;
        using SceneLogicPtr = SR_HTYPES_NS::SharedPtr<SceneLogic>;
        using Super = SR_UTILS_NS::IComponentable;
        using SceneObjectPtr = SR_HTYPES_NS::SharedPtr<SceneObject>;
        using SceneObjects = Vector<SceneObjectPtr>;
        using GameObjectPtr = SR_HTYPES_NS::SharedPtr<GameObject>;

        SR_MAYBE_UNUSED SR_INLINE_STATIC const std::string RuntimeScenePath = "Scenes/Runtime-cache-scene"; /// NOLINT
        SR_MAYBE_UNUSED SR_INLINE_STATIC const std::string NewScenePath = "Scenes/New-cache-scene"; /// NOLINT
        SR_MAYBE_UNUSED SR_INLINE_STATIC const std::string NewPrefabPath = "Scenes/New-cache-prefab"; /// NOLINT

    public:
        Scene();
        ~Scene() override;

    public:
        static Scene::Ptr CreateEmptyScene();
        static Scene::Ptr NewScene(const Path& path, SceneLogicType type);
        static Scene::Ptr LoadScene(const Path& path);
        static bool IsExists(const Path& path);
        static Path GetAbsPath(const Path& path);

        void Init();
        void Prepare();

        bool SaveScene();
        bool SaveSceneAt(const Path& path);
        bool Destroy();
        bool SetDirty(bool dirty) override;

        void SetPath(const Path& path);

    public:
        SR_NODISCARD std::string GetName() const;
        SR_NODISCARD const Path& GetPath() const { return m_path; }
        SR_NODISCARD Path GetAbsPath() const;
        SR_NODISCARD bool IsPrefab() const noexcept override;
        SR_NODISCARD SR_HTYPES_NS::DataStorage& GetDataStorage();
        SR_NODISCARD const SR_HTYPES_NS::DataStorage& GetDataStorage() const;
        SR_NODISCARD SR_INLINE SceneUpdater* GetSceneUpdater() const { return m_sceneUpdater; }
        SR_NODISCARD SceneLogicPtr GetLogicBase() const;
        SR_NODISCARD const SR_HTYPES_NS::SharedPtr<SR_UTILS_NS::EntityController>& GetEntityController() const;

        /// Запущена ли сцена
        SR_NODISCARD virtual bool IsPlayingMode() const { return false; }
        /// На паузе ли сцена (если запущена)
        SR_NODISCARD virtual bool IsPausedMode() const { return false; }
        SR_NODISCARD bool IsEditorMode() const;

        SceneObjects& GetRootSceneObjects();

        SceneObjectPtr FindByComponent(const std::string& name);
        SceneObjectPtr Find(const std::string& name);
        SceneObjectPtr Find(const char* name);
        SceneObjectPtr Find(uint64_t hashName);
        SceneObjectPtr Find(SR_UTILS_NS::StringAtom name);

        void RegisterSceneObject(const SceneObjectPtr& pSO);

        virtual SceneObjectPtr InstanceFromFile(const SR_UTILS_NS::Path& path);
        virtual SceneObjectPtr Instance(const Types::RawMesh* rawMesh);

        virtual GameObjectPtr FindOrInstanceGameObject(SR_UTILS_NS::StringAtom name);
        virtual GameObjectPtr InstanceGameObject(SR_UTILS_NS::StringAtom name);
        virtual GameObjectPtr GetMainCamera() const;

        IComponentable::ScenePtr GetScene() const override { return const_cast<ScenePtr>(this); }

        SR_NODISCARD float_t GetSpeed() const { return m_speed; }
        void SetSpeed(float_t speed) { m_speed = speed; }

    public:
        bool Remove(const SceneObjectPtr& gameObject);
        void Remove(const SR_HTYPES_NS::SharedPtr<Component>& pComponent);

        void OnChanged();

        bool Reload();

        void OnPostLoad() override;

    private:
        void RegisterSceneObjectImpl(const SceneObjectPtr& pSO);
        void ProcessNewSO(const SceneObjectPtr& pSO);

    private:
        mutable EntityReplaceMap m_registerEntityIdReplaceCache;
        mutable SR_HTYPES_NS::FlatHashMap<const void*, std::vector<Entity*>> m_registerEntityCache;

        SceneUpdater* m_sceneUpdater = nullptr;

        float_t m_speed = 1.f;

        bool m_isInitialized = false;
        bool m_isPreDestroyed = false;
        bool m_isDestroyed = false;

        std::atomic<bool> m_isHierarchyChanged = false;

        SR_HTYPES_NS::DataStorage m_dataStorage;

        std::list<uint64_t> m_freeObjIndices;
        std::list<SceneObjectPtr> m_newQueue;
        std::list<SceneObjectPtr> m_deleteQueue;

        std::list<SR_HTYPES_NS::SharedPtr<Component>> m_destroyedComponents;

        Path m_path;
        Path m_absPath;

        SceneObjects m_sceneObjects;

        SR_HTYPES_NS::SharedPtr<SR_UTILS_NS::EntityController> m_pEntityController;

    private:
        /// @property
        SR_HTYPES_NS::SharedPtr<SceneLogic> m_logic;
        /// @property @getter(GetRootSceneObjects)
        /// @condition(This.m_logic && This.m_logic->IsAllowedRootSave())
        SceneObjects m_root;

    };
}

#endif //SR_ENGINE_SCENE_H
