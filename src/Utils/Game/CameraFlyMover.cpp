//
// Created by Monika on 26.11.2023.
//

#include <Utils/Game/CameraFlyMover.h>
#include <Utils/ECS/Transform3D.h>
#include <Utils/ECS/IComponentable.h>

#include <Codegen/CameraFlyMover.generated.hpp>

namespace SR_UTILS_NS {
    void CameraFlyMover::OnDisable() {
        m_lock.reset();
    }

    void CameraFlyMover::FixedUpdate() {
        if (m_lockCursor) {
            if (!m_lock) {
                const auto lockMode = m_executeInEditorMode ? SR_UTILS_NS::CursorLockMode::Editor : SR_UTILS_NS::CursorLockMode::PlayMode;
                m_lock = SR_UTILS_NS::CursorLock(lockMode);
            }
        }
        else {
            m_lock.reset();
        }

        if (!m_velocity.Empty()) {
            GetTransform()->Translate(m_velocity);
        }

        if (!m_active) {
            m_velocity *= m_velocityDegree;
            return;
        }

        const float_t velocitySpeed = m_moveSpeed * m_velocityFactor;
        auto&& dir = SR_UTILS_NS::Input::Instance().GetMouseDrag();
        auto&& wheel = SR_UTILS_NS::Input::Instance().GetMouseWheel() * m_wheelSpeed;

        if (!SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::Ctrl)) {
            bool accelerateX = false;
            bool accelerateZ = false;

            if (SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::W)) {
                m_velocity.z = SR_MAX(m_velocity.z, 0.f);
                m_velocity += SR_UTILS_NS::Transform3D::FORWARD * velocitySpeed;
                accelerateZ = true;
            }

            if (SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::S)) {
                m_velocity.z = SR_MIN(m_velocity.z, 0.f);
                m_velocity -= SR_UTILS_NS::Transform3D::FORWARD * velocitySpeed;
                accelerateZ = true;
            }

            if (SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::A)) {
                m_velocity.x = SR_MIN(m_velocity.x, 0.f);
                m_velocity -= SR_UTILS_NS::Transform3D::RIGHT * velocitySpeed;
                accelerateX = true;
            }

            if (SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::D)) {
                m_velocity.x = SR_MAX(m_velocity.x, 0.f);
                m_velocity += SR_UTILS_NS::Transform3D::RIGHT * velocitySpeed;
                accelerateX = true;
            }

            if (!accelerateX) {
                m_velocity.x *= m_velocityDegree;
            }

            if (!accelerateZ) {
                m_velocity.z *= m_velocityDegree;
            }
        }
        else {
            m_velocity *= m_velocityDegree;
        }

        m_velocity = m_velocity.Clamp(-m_maxVelocity, m_maxVelocity);

        if (wheel != 0) {
            GetTransform()->Translate(SR_UTILS_NS::Transform3D::FORWARD * wheel);
        }

        if (!m_rightMouseButtonToRotate || SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::MouseRight)) {
            GetTransform()->GlobalRotate(dir.y * m_rotateSpeed.y, dir.x * m_rotateSpeed.x, 0.0);
        }

        if (SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::MouseMiddle)) {
            auto right = SR_UTILS_NS::Transform3D::RIGHT * m_seekSpeed;
            auto up = SR_UTILS_NS::Transform3D::UP * m_seekSpeed;

            GetTransform()->Translate((up * dir.y) + (right * -dir.x));
        }
    }

    void CameraFlyMover::SetExecuteInEditMode(const bool enabled) {
        m_executeInEditorMode = enabled;
        if (auto&& pParent = TryGetParent()) {
            pParent->SetDirty(true);
        }
    }
}