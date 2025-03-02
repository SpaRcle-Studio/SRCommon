//
// Created by Monika on 02.03.2025.
//

#ifndef SR_ENGINE_SCENE_LOGIC_TYPE_H
#define SR_ENGINE_SCENE_LOGIC_TYPE_H

#include <Utils/Common/Enumerations.h>

namespace SR_WORLD_NS {
    SR_ENUM_NS_CLASS_T(SceneLogicType, int16_t,
        Asset = 0,
        CubeChunk = 1,
        Prefab = 2
    )
}

#endif //SR_ENGINE_SCENE_LOGIC_TYPE_H
