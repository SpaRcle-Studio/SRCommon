//
// Created by Monika on 17.09.2025.
//

#ifndef SR_ENGINE_UTILS_LOCK_POSITION_COMPONENT_H
#define SR_ENGINE_UTILS_LOCK_POSITION_COMPONENT_H

#include <Utils/ECS/Component.h>
#include <Utils/ECS/Transform3D.h>

namespace SR_UTILS_NS {
    /// @category(Game)
    class SR_COMMON_DLL_API LockPositionComponent final : public Component {
        SR_CLASS()
        using Super = Component;
    public:
        void Awake() override;
        void Update(float_t) override;

    private:
        /// @property @dontSave @readOnly
        SR_MATH_NS::FVector3 m_position;

    };
}

#endif //SR_ENGINE_UTILS_LOCK_POSITION_COMPONENT_H
