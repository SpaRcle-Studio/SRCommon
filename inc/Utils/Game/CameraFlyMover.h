//
// Created by Monika on 05.03.2025.
//

#ifndef SR_ENGINE_UTILS_CAMERA_FLY_MOVER_H
#define SR_ENGINE_UTILS_CAMERA_FLY_MOVER_H

#include <Utils/Input/InputSystem.h>
#include <Utils/ECS/Component.h>

namespace SR_UTILS_NS {
    /// @category(Game) @nonMovable @noCopyable
    class CameraFlyMover : public Component {
        SR_CLASS()
        using Super = Component;
    public:
        void Update(float_t dt) override;
        void OnDisable() override;

        void SetVelocityFactor(const float_t factor) { m_velocityFactor = factor; }
        void SetActive(const bool active) { m_active = active; }
        void SetExecuteInEditMode(bool enabled);
        void SetRightMouseButtonToRotate(const bool enabled) { m_rightMouseButtonToRotate = enabled; }

        SR_NODISCARD bool ExecuteInEditMode() const override { return m_executeInEditorMode; }

    private:
        std::optional<CursorLock> m_lock;

    private:
        /// @property
        bool m_gameSpeedCorrection = true;
        /// @property
        bool m_active = true;
        /// @property
        bool m_lockCursor = false;
        /// @property
        bool m_rightMouseButtonToRotate = false;
        /// @property @setter(SetExecuteInEditMode)
        bool m_executeInEditorMode = false;
        /// @property @drag(0.1f)
        float_t m_velocityDegree = 1.0f;
        /// @property @drag(0.1f)
        float_t m_velocityFactor = 1.0f;
        /// @property @drag(1.0f)
        float_t m_seekSpeed = 1.0f;
        /// @property @drag(0.1f)
        float_t m_wheelSpeed = 1.0f;
        /// @property @drag(0.1f)
        SR_MATH_NS::FVector2 m_rotateSpeed = 1.0f;
        /// @property @drag(0.1f)
        float_t m_moveSpeed = 1.0f;
        /// @property
        SR_MATH_NS::FVector3 m_maxVelocity = SR_MATH_NS::FVector3(0.5f, 0.5f, 2.f);
        /// @property @dontSave @readonly
        SR_MATH_NS::FVector3 m_velocity;

    };
}

#endif //SR_ENGINE_UTILS_CAMERA_FLY_MOVER_H
