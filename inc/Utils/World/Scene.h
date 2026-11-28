//
// Created by Nikita on 30.11.2020.
//

#ifndef SR_ENGINE_SCENE_H
#define SR_ENGINE_SCENE_H

#include <Utils/ECS/IComponentable.h>

#include <Utils/Types/StringAtom.h>
#include <Utils/Types/Vector.h>

#include <Utils/World/SceneLogicType.h>
#include <Utils/World/CameraData.h>
#include <Utils/World/ISceneModule.h>

namespace SR_UTILS_NS {
    class SceneObject;
    class Prefab;
    class EntityController;
    class Component;
    class GameObject;
    class WorldShape;
    class FloatingOrigin;
}

namespace SR_HTYPES_NS {
    class RawMesh;
}

namespace SR_WORLD_NS {
    class SceneLogic;
    class SceneUpdater;

    /// @noCopyable @noMovable
    class SR_COMMON_DLL_API Scene : public SR_UTILS_NS::IComponentable {
        SR_CLASS()
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<Scene>;
        using SceneLogicPtr = SR_HTYPES_NS::SharedPtr<SceneLogic>;
        using Super = SR_UTILS_NS::IComponentable;
        using SceneObjectPtr = SR_HTYPES_NS::SharedPtr<SceneObject>;
        using SceneObjects = Vector<SceneObjectPtr>;
        using GameObjectPtr = SR_HTYPES_NS::SharedPtr<GameObject>;

        static const StringView RuntimeScenePath;
        static const StringView NewScenePath;
        static const StringView NewPrefabPath;

    public:
        Scene();
        ~Scene() override;

    public:
        static Scene::Ptr CreateEmptyScene();
        static Scene::Ptr NewScene(const Path& path, SceneLogicType type);
        static Scene::Ptr NewScene(SceneLogicType type);
        static Scene::Ptr LoadScene(const Path& path);
        static bool PlayScene(const Path& path);

        void Init();
        void Prepare();

        bool SaveScene();
        bool SaveSceneAt(const Path& path);
        bool Destroy();
        bool SetDirty(bool dirty) override;

        void SetPath(const Path& path);
        void SetEditorMode(bool isEditorMode) { m_isEditorMode = isEditorMode; }

    public:
        SR_NODISCARD String GetName() const;
        SR_NODISCARD const Path& GetPath() const { return m_path; }
        SR_NODISCARD Path GetAbsPath() const;
        SR_NODISCARD bool IsPrefab() const noexcept override;
        SR_NODISCARD SR_INLINE SceneUpdater* GetSceneUpdater() const { return m_sceneUpdater; }
        /// @method @evaluate
        SR_NODISCARD bool IsUIInputConsumed() const;
        SR_NODISCARD SceneLogicPtr GetLogicBase() const;
        SR_NODISCARD const SR_HTYPES_NS::SharedPtr<SR_UTILS_NS::EntityController>& GetEntityController() const;
        SR_NODISCARD ISceneModule* GetModule(StringView name) const;

        /// Запущена ли сцена
        SR_NODISCARD virtual bool IsPlayingMode() const { return false; }
        /// На паузе ли сцена (если запущена)
        SR_NODISCARD virtual bool IsPausedMode() const { return false; }
        SR_NODISCARD bool IsEditorMode() const;

        SceneObjects& GetRootSceneObjects();
        bool MoveRootSceneObject(const SceneObjectPtr& pSO, int32_t index);

        SceneObjectPtr FindByComponent(const String& name);
        SceneObjectPtr Find(const String& name);
        SceneObjectPtr Find(const char* name);
        SceneObjectPtr Find(uint64_t hashName);
        SceneObjectPtr Find(SR_UTILS_NS::StringAtom name);

        /// @method
        void RegisterSceneObject(const SR_HTYPES_NS::SharedPtr<SceneObject>& pSO);

        void SetModule(StringView name, ISceneModule* pModule);

        /// Активные формы мира (WorldShape регистрирует себя сам), запросы - через WorldShape::Find*
        void RegisterWorldShape(SR_UTILS_NS::WorldShape* pShape);
        void UnregisterWorldShape(SR_UTILS_NS::WorldShape* pShape);
        SR_NODISCARD const Vector<SR_UTILS_NS::WorldShape*>& GetWorldShapes() const noexcept { return m_worldShapes; }

        /// Активный FloatingOrigin (регистрирует себя сам, один на сцену), запросы - через FloatingOrigin::Find*
        bool RegisterFloatingOrigin(SR_UTILS_NS::FloatingOrigin* pFloatingOrigin);
        void UnregisterFloatingOrigin(SR_UTILS_NS::FloatingOrigin* pFloatingOrigin);
        SR_NODISCARD SR_UTILS_NS::FloatingOrigin* GetFloatingOrigin() const noexcept { return m_pFloatingOrigin; }

        /// Сдвиг начала координат: корневые 3D-объекты переезжают на -delta, затем OnOriginShifted(delta)
        /// получают модули сцены (физика) и все компоненты. Абсолютную позицию начала хранит FloatingOrigin
        void ShiftOrigin(const SR_MATH_NS::FVector3& delta);

        /// @method
        virtual SR_HTYPES_NS::SharedPtr<SceneObject> InstanceFromFile(const SR_UTILS_NS::Path& path);
        virtual SR_HTYPES_NS::SharedPtr<SceneObject> Instance(SR_HTYPES_NS::RawMesh* rawMesh);

        /// @method
        virtual SR_HTYPES_NS::SharedPtr<GameObject> FindOrInstanceGameObject(SR_UTILS_NS::StringAtom name);
        /// @method
        virtual SR_HTYPES_NS::SharedPtr<GameObject> InstanceGameObject(SR_UTILS_NS::StringAtom name);

        /// @method @evaluate
        virtual SR_HTYPES_NS::SharedPtr<GameObject> GetMainCamera() const;

        IComponentable::ScenePtr GetScenePtr() const override { return const_cast<ScenePtr>(this); }

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
        void NotifyOriginShifted(SceneObject& object, const SR_MATH_NS::FVector3& delta);

    private:
        mutable EntityReplaceMap m_registerEntityIdReplaceCache;
        mutable SR_HTYPES_NS::FlatHashMap<const void*, Vector<Entity*>> m_registerEntityCache;

        SceneUpdater* m_sceneUpdater = nullptr;

        float_t m_speed = 1.f;

        bool m_isEditorMode = false;
        bool m_isInitialized = false;
        bool m_isPreDestroyed = false;
        bool m_isDestroyed = false;

        std::atomic<bool> m_isHierarchyChanged = false;

        std::list<uint64_t> m_freeObjIndices;
        std::list<SceneObjectPtr> m_newQueue;
        std::list<SceneObjectPtr> m_deleteQueue;

        std::list<SR_HTYPES_NS::SharedPtr<Component>> m_destroyedComponents;

        Path m_path;

        SceneObjects m_sceneObjects;

        SR_HTYPES_NS::SharedPtr<SR_UTILS_NS::EntityController> m_pEntityController;
        Map<StringAtom, ISceneModule*> m_modules;
        Vector<SR_UTILS_NS::WorldShape*> m_worldShapes;
        SR_UTILS_NS::FloatingOrigin* m_pFloatingOrigin = nullptr;

    private:
        /// @property
        SR_HTYPES_NS::SharedPtr<SceneLogic> m_logic;
        /// @property @getter(GetRootSceneObjects)
        /// @condition(This.m_logic && This.m_logic->IsAllowedRootSave())
        SceneObjects m_root;

    };
}

#endif //SR_ENGINE_SCENE_H
