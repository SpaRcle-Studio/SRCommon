//
// Created by Monika on 23.08.2022.
//

#ifndef SR_ENGINE_TRANSFORM_ZERO_H
#define SR_ENGINE_TRANSFORM_ZERO_H

#include <Utils/ECS/Transform.h>

namespace SR_UTILS_NS {
    class GameObject;

    class SR_DLL_EXPORT TransformZero : public Transform {
        SR_CLASS()
        friend class GameObject;
    public:
        SR_NODISCARD Measurement GetMeasurement() const override { return Measurement::SpaceZero; }

    };

    class SR_DLL_EXPORT TransformHolder : public Transform {
        SR_CLASS()
        friend class GameObject;
    public:
        SR_NODISCARD Measurement GetMeasurement() const override { return Measurement::SpaceZero; }

    };
}


#endif //SR_ENGINE_TRANSFORM_ZERO_H
