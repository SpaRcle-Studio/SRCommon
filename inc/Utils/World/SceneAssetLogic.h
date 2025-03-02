//
// Created by Monika on 02.03.2025.
//

#ifndef SR_ENGINE_SCENE_ASSET_LOGIC_H
#define SR_ENGINE_SCENE_ASSET_LOGIC_H

#include <Utils/World/SceneLogic.h>

namespace SR_WORLD_NS {
    class SceneAssetLogic final : public SceneLogic {
        SR_CLASS()
        using Super = SceneLogic;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<SceneAssetLogic>;

    public:
        SR_NODISCARD StringAtom GetSceneExtension() const noexcept override { return "scene"; }
        SR_NODISCARD SceneLogicType GetType() const noexcept override { return SceneLogicType::Asset; }

    };
}

#endif //SR_ENGINE_SCENE_ASSET_LOGIC_H
