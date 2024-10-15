//
// Created by Monika on 22.12.2022.
//

#ifndef SR_ENGINE_SCENELOGIC_H
#define SR_ENGINE_SCENELOGIC_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/Types/SafePointer.h>

namespace SR_UTILS_NS {
    class GameObject;
}

namespace SR_WORLD_NS {
    class Scene;

    /// @abstract
    class SceneLogic : public SR_HTYPES_NS::SharedPtr<SceneLogic>, public SR_UTILS_NS::Serializable {
        SR_CLASS()
        using Super = SR_HTYPES_NS::SharedPtr<SceneLogic>;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<SceneLogic>;
        using ScenePtr = SR_HTYPES_NS::SharedPtr<Scene>;
        using SceneObjectPtr = SR_HTYPES_NS::SharedPtr<SceneObject>;
        using SceneObjects = std::vector<SceneObjectPtr>;

    public:
        explicit SceneLogic(const ScenePtr& scene);
        ~SceneLogic() override = default;

    public:
        /// Метод всегда вернет валидную логику сцены
        static SceneLogic::Ptr CreateByExt(const ScenePtr& scene, const std::string& ext);

    public:
        SR_NODISCARD virtual bool IsDefault() const noexcept { return false; }

        virtual void Init() { }
        virtual void Update(float_t dt) { }
        virtual void Destroy() { }
        virtual void PostLoad() { }

        virtual bool Reload() { return true; }

        virtual bool Load(const Path& path);
        virtual bool Save(const Path& path);

    protected:
        ScenePtr m_scene;
        mutable std::recursive_mutex m_mutex;

    };
}

#endif //SR_ENGINE_SCENELOGIC_H
