//
// Created by Monika on 03.05.2023.
//

#include <Utils/Game/LookAtComponent.h>
#include <Utils/ECS/Transform3D.h>
#include <Utils/ECS/ComponentManager.h>

#include <Codegen/LookAtComponent.generated.hpp>

namespace SR_UTILS_NS {
    LookAtComponent::LookAtComponent()
        : Super()
    {
        m_target.SetOwner(GetThis());
    }

    void LookAtComponent::Update(float_t dt) {
        auto&& pTarget = m_target.GetGameObject();
        if (!pTarget) {
            Super::Update(dt);
            return;
        }

        auto&& pTargetTransform = pTarget->GetTransform();
        if (!pTargetTransform) {
            Super::Update(dt);
            return;
        }

        if (auto&& pTransform = GetTransform()) {
            auto&& sourceTranslate = pTransform->GetMatrix().GetTranslate() + m_offset;
            auto&& destinationTranslate = pTargetTransform->GetMatrix().GetTranslate();

            SR_MATH_NS::FVector3 direction;

            if (m_mirror) {
                direction = destinationTranslate - sourceTranslate;
            }
            else {
                direction = sourceTranslate - destinationTranslate;
            }

            if (direction == m_direction && m_reached) {
                Super::Update(dt);
                return;
            }

            m_direction = direction;

            const SR_MATH_NS::FVector3 up = GetUp();

            SR_MATH_NS::Quaternion q = SR_MATH_NS::Quaternion::LookAt(m_direction, up);

            if (!SR_EQUALS(m_delay, 0.f)) {
                auto&& sourceQ = pTransform->GetQuaternion();
                q = q.Slerp(q, SR_MIN(dt * (1.f / m_delay), 1.f));
                m_reached = q == sourceQ;
            }

            pTransform->SetRotation(q);
        }

        Super::Update(dt);
    }

    void LookAtComponent::OnDestroy() {
        Super::OnDestroy();

        GetThis().AutoFree([](auto&& pData) {
            delete pData;
        });
    }

    void LookAtComponent::SetExecuteInEditMode(bool enabled) {
        m_editMode = enabled;

        if (auto&& pParent = TryGetParent()) {
            pParent->SetDirty(true);
        }
    }

    void LookAtComponent::SetAxis(LookAtAxis axis) {
        m_axis = axis;
        m_reached = false;
    }

    void LookAtComponent::SetMirror(bool mirror) {
        m_mirror = mirror;
        m_reached = false;
    }

    void LookAtComponent::SetOffset(const SR_MATH_NS::FVector3& offset) {
        m_offset = offset;
        m_reached = false;
    }

    SR_MATH_NS::FVector3 LookAtComponent::GetUp() const noexcept {
        switch (m_axis) {
            case LookAtAxis::AxisX: return Transform3D::RIGHT;
            case LookAtAxis::AxisY: return Transform3D::UP;
            case LookAtAxis::AxisZ: return Transform3D::FORWARD;
            case LookAtAxis::InvAxisX: return -Transform3D::RIGHT;
            case LookAtAxis::InvAxisY: return -Transform3D::UP;
            case LookAtAxis::InvAxisZ: return -Transform3D::FORWARD;
            default:
                SRHalt0();
                return SR_MATH_NS::FVector3();
        }
    }
}
