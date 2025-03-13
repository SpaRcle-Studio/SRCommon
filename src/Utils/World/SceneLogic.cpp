//
// Created by Monika on 22.12.2022.
//

#include <Utils/World/SceneLogic.h>

#include <Utils/World/SceneDefaultLogic.h>
#include <Utils/World/SceneCubeChunkLogic.h>
#include <Utils/World/ScenePrefabLogic.h>

#include <Codegen/SceneLogic.generated.hpp>

namespace SR_WORLD_NS {
    SceneLogic::SceneLogic()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Manually)
    { }

    void SceneLogic::SetScene(const ScenePtr& pScene) {
        SRAssert2(!m_scene, "SceneLogic::SetScene() : scene already set!");
        m_scene = pScene;
    }
}
