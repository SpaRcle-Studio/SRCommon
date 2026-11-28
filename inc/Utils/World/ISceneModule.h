//
// Created by Monika on 11.09.2026.
//

#ifndef SR_ENGINE_COMMON_I_SCENE_MODULE_H
#define SR_ENGINE_COMMON_I_SCENE_MODULE_H

#include <Utils/stdInclude.h>
#include <Utils/Math/Vector3.h>

namespace SR_UTILS_NS {
    class ISceneModule {
    public:
        virtual ~ISceneModule() = default;

    public:
        /// Начало координат мира сдвинуто (Scene::ShiftOrigin): корневые объекты уже переехали на -delta
        virtual void OnOriginShifted(const SR_MATH_NS::FVector3& delta) { }

    };
}

#endif //SR_ENGINE_COMMON_I_SCENE_MODULE_H
