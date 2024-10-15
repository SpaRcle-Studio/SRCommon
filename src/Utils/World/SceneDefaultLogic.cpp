//
// Created by Monika on 23.12.2022.
//

#include <Utils/World/SceneDefaultLogic.h>

#include <Codegen/SceneDefaultLogic.generated.hpp>

namespace SR_WORLD_NS {
    SceneDefaultLogic::SceneDefaultLogic(const Super::ScenePtr& scene)
        : Super(scene)
    { }

    bool SceneDefaultLogic::Reload() {
        return Super::Reload();
    }

    void SceneDefaultLogic::Destroy() {
        Super::Destroy();
    }

    void SceneDefaultLogic::Update(float_t dt) {
        Super::Update(dt);
    }

    bool SceneDefaultLogic::Load(const Path& path) {
        return false;
    }

    bool SceneDefaultLogic::Save(const Path& path) {
        XMLSerializer serializer;

        auto&& rootObjects = m_scene->GetRootSceneObjects();
        for (auto&& object : rootObjects) {
            object->Save(serializer);
        }

        //return serializer.Save(path);
        return false;
    }
}