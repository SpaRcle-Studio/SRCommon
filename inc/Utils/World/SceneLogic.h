//
// Created by Monika on 22.12.2022.
//

#ifndef SR_ENGINE_SCENE_LOGIC_H
#define SR_ENGINE_SCENE_LOGIC_H

#include <Utils/Types/SharedPtr.h>
#include <Utils/Serialization/Serializable.h>
#include <Utils/World/SceneLogicType.h>
#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS {
    class SceneObject;
}

namespace SR_WORLD_NS {
    class Scene;

    /// @abstract
    class SceneLogic : public SR_UTILS_NS::Serializable, public SR_HTYPES_NS::SharedPtr<SceneLogic> {
        SR_CLASS()
        using Super = SR_HTYPES_NS::SharedPtr<SceneLogic>;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<SceneLogic>;
        using ScenePtr = SR_HTYPES_NS::SharedPtr<Scene>;
        using SceneObjectPtr = SR_HTYPES_NS::SharedPtr<SceneObject>;
        using SceneObjects = std::vector<SceneObjectPtr>;
        using OriginType = SceneLogic;

    public:
        SceneLogic();

    public:
        SR_NODISCARD virtual bool IsAllowedRootSave() const noexcept { return true; }

        SR_NODISCARD virtual SceneLogicType GetType() const noexcept {
            SRHalt("Abstract method called!");
            return SceneLogicType::SceneLogicTypeMAX;
        }

        SR_NODISCARD virtual StringAtom GetSceneExtension() const noexcept {
            SRHalt("Abstract method called!");
            return StringAtom();
        }

        SR_NODISCARD const ScenePtr& GetScene() const noexcept { return m_scene; }
        SR_NODISCARD virtual SR_UTILS_NS::Path GetSceneDataPath(const SR_UTILS_NS::Path& path) const { return path; }

        virtual void SetScene(const ScenePtr& pScene);

        virtual void Update(float_t dt) { }
        virtual void Destroy() { }
        virtual void Prepare() { }

        virtual void InitLogic() { SRAssert(m_scene); }
        virtual bool SaveLogic(ISerializer& serializer, const Path& path) { return true; }
        virtual bool LoadLogic(IDeserializer& deserializer, const Path& path) { return true; }

        virtual bool Reload() { return true; }

    protected:
        ScenePtr m_scene;
        mutable std::recursive_mutex m_mutex;

    };
}

#endif //SR_ENGINE_SCENE_LOGIC_H
