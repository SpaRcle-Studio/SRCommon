//
// Created by Monika on 26.11.2023.
//

#include <Utils/ECS/IComponentable.h>
#include <Utils/ECS/Transform3D.h>
#include <Utils/Game/CameraFlyMover.h>
#include <Utils/World/Scene.h>

#include <Codegen/CameraFlyMover.generated.hpp>

namespace SR_UTILS_NS {
    void CameraFlyMover::OnDisable() { m_lock.reset(); }

    void CameraFlyMover::Update(float_t dt) {
        SR_TRACY_ZONE;

        if (m_lockCursor) {
            if (!m_lock) {
                const auto lockMode =
                    m_executeInEditorMode
                    ? SR_UTILS_NS::CursorLockMode::Editor
                    : SR_UTILS_NS::CursorLockMode::PlayMode;

                m_lock = SR_UTILS_NS::CursorLock(lockMode);
            }
        }
        else {
            m_lock.reset();
        }

        if (m_gameSpeedCorrection) {
            const float gameSpeed = GetScene()->GetSpeed();
            dt /= SR_MAX(gameSpeed, 0.0001f);
        }

        const float damping = std::exp(-m_velocityDegree * 10.f * dt);

        const float velocityAccel = m_moveSpeed * m_velocityFactor * 0.25f;
        const float seekSpeed = m_seekSpeed * 0.01f;
        const float wheelSpeed = m_wheelSpeed * 0.5f;

        const SR_MATH_NS::FVector2 rotateSpeed = m_rotateSpeed * 0.1f;

        if (!m_active) {
            m_velocity *= damping;
            return;
        }

        auto&& input = SR_UTILS_NS::Input::Instance();

        const auto mouseDrag = input.GetMouseDrag();
        const float wheel = input.GetMouseWheel();

        if (!input.GetKey(SR_UTILS_NS::KeyCode::LCtrl)) {
            bool accelerateX = false;
            bool accelerateZ = false;

            if (input.GetKey(SR_UTILS_NS::KeyCode::W)) {
                m_velocity += SR_UTILS_NS::Transform3D::FORWARD * velocityAccel * dt;
                accelerateZ = true;
            }

            if (input.GetKey(SR_UTILS_NS::KeyCode::S)) {
                m_velocity -= SR_UTILS_NS::Transform3D::FORWARD * velocityAccel * dt;
                accelerateZ = true;
            }

            if (input.GetKey(SR_UTILS_NS::KeyCode::A)) {
                m_velocity -= SR_UTILS_NS::Transform3D::RIGHT * velocityAccel * dt;
                accelerateX = true;
            }

            if (input.GetKey(SR_UTILS_NS::KeyCode::D)) {
                m_velocity += SR_UTILS_NS::Transform3D::RIGHT * velocityAccel * dt;
                accelerateX = true;
            }

            if (!accelerateX)
                m_velocity.x *= damping;

            if (!accelerateZ)
                m_velocity.z *= damping;
        }
        else {
            m_velocity *= damping;
        }

        m_velocity = m_velocity.Clamp(-m_maxVelocity, m_maxVelocity);

        if (!m_velocity.Empty()) {
            GetTransform()->Translate(m_velocity * dt * 100.f);
        }

        if (wheel != 0.f) {
            GetTransform()->Translate(
                SR_UTILS_NS::Transform3D::FORWARD * wheel * wheelSpeed
            );
        }

        if (!m_rightMouseButtonToRotate || input.GetKey(SR_UTILS_NS::KeyCode::MouseRight)) {
            const SR_MATH_NS::FVector2 rotateDir = mouseDrag * rotateSpeed;

            if (rotateDir != SR_MATH_NS::FVector2::Zero()) {
                GetTransform()->GlobalRotate(rotateDir.y, rotateDir.x, 0.0);
            }
        }

        if (input.GetKey(SR_UTILS_NS::KeyCode::MouseMiddle)) {
            auto right = SR_UTILS_NS::Transform3D::RIGHT * seekSpeed;
            auto up = SR_UTILS_NS::Transform3D::UP * seekSpeed;

            GetTransform()->Translate(
                (up * mouseDrag.y) +
                (right * -mouseDrag.x)
            );
        }
    }

    void CameraFlyMover::SetExecuteInEditMode(const bool enabled) {
        m_executeInEditorMode = enabled;
        if (auto&& pParent = TryGetParent()) {
            pParent->SetDirty(true);
        }
    }
} // namespace SR_UTILS_NS
