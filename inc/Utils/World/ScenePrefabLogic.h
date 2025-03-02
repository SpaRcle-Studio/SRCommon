//
// Created by Monika on 24.12.2022.
//

#ifndef SR_ENGINE_SCENE_PREFAB_LOGIC_H
#define SR_ENGINE_SCENE_PREFAB_LOGIC_H

#include <Utils/World/SceneLogic.h>

namespace SR_WORLD_NS {
    class ScenePrefabLogic : public SceneLogic {
        using Super = SceneLogic;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<ScenePrefabLogic>;

    public:
        void OnPreSave() override;

        SR_NODISCARD SceneLogicType GetType() const noexcept override { return SceneLogicType::Prefab; }
        SR_NODISCARD StringAtom GetSceneExtension() const noexcept override { return "prefab"; }

    };
}

#endif //SR_ENGINE_SCENE_PREFAB_LOGIC_H
