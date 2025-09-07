//
// Created by Monika on 26.11.2023.
//

#include <Utils/Game/CameraFlyMover.h>

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

        m_velocity *= m_velocityDegree;

        if (!m_velocity.Empty()) {
            GetTransform()->Translate(m_velocity);
        }

        if (!m_active) {
            return;
        }

        const float_t velocitySpeed = m_moveSpeed * m_velocityFactor;
        auto&& dir = SR_UTILS_NS::Input::Instance().GetMouseDrag();
        auto&& wheel = SR_UTILS_NS::Input::Instance().GetMouseWheel() * m_wheelSpeed * m_velocityFactor;

        if (!SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::Ctrl)) {
            if (SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::W)) {
                m_velocity += SR_UTILS_NS::Transform3D::FORWARD * velocitySpeed;
            }

            if (SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::S)) {
                m_velocity -= SR_UTILS_NS::Transform3D::FORWARD * velocitySpeed;
            }

            if (SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::A)) {
                m_velocity -= SR_UTILS_NS::Transform3D::RIGHT * velocitySpeed;
            }

            if (SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::D)) {
                m_velocity += SR_UTILS_NS::Transform3D::RIGHT * velocitySpeed;
            }
        }

        if (wheel != 0) {
            GetTransform()->Translate(SR_UTILS_NS::Transform3D::FORWARD * wheel);
        }

        if (!m_rightMouseButtonToRotate || SR_UTILS_NS::Input::Instance().GetKey(SR_UTILS_NS::KeyCode::MouseRight)) {
            GetTransform()->GlobalRotate(dir.y * m_rotateSpeed, dir.x * m_rotateSpeed, 0.0);
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