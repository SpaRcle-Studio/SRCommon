//
// Created by Monika on 03.05.2023.
//

#ifndef SR_ENGINE_LOOKATCOMPONENT_H
#define SR_ENGINE_LOOKATCOMPONENT_H

#include <Utils/ECS/EntityRefOld.h>
#include <Utils/ECS/Component.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/ECS/Transform.h>

namespace SR_UTILS_NS {
    /// @category(Game)
    class SR_COMMON_DLL_API LookAtComponent final : public Component {
        SR_CLASS()
        using Super = Component;
    public:
        LookAtComponent();

    public:
        void Update(float_t dt) override;

        void OnDestroy() override;

        void SetTarget(const GameObject::Ptr& target) { m_target.SetPathTo(target->GetThis().DynamicCast<Entity>()); }
        void SetAxis(LookAtAxis axis);
        void SetExecuteInEditMode(bool enabled);
        void SetDelay(float_t delay) { m_delay = SR_MAX(delay, 0.f); }
        void SetMirror(bool mirror);
        void SetOffset(const SR_MATH_NS::FVector3& offset);

        SR_NODISCARD bool ExecuteInEditMode() const override { return m_editMode; }
        SR_NODISCARD bool GetMirror() const noexcept { return m_mirror; }
        SR_NODISCARD SR_MATH_NS::FVector3 GetOffset() const noexcept { return m_offset; }
        SR_NODISCARD SR_MATH_NS::FVector3 GetUp() const noexcept;
        SR_NODISCARD SR_MATH_NS::FVector3 GetDirection() const noexcept { return m_direction; }
        SR_NODISCARD SR_UTILS_NS::EntityRefOld& GetTarget() { return m_target; }
        SR_NODISCARD float_t GetDelay() const noexcept { return m_delay; }
        SR_NODISCARD LookAtAxis GetAxis() const noexcept { return m_axis; }

    private:
        SR_MATH_NS::FVector3 m_direction;
        bool m_reached = false;

        /// @property
        SR_UTILS_NS::EntityRefOld m_target;
        /// @property @getter(GetAxis) @setter(SetAxis)
        LookAtAxis m_axis = LookAtAxis::AxisY;
        /// @property @getter(GetOffset) @setter(SetOffset)
        SR_MATH_NS::FVector3 m_offset;
        /// @property @getter(GetMirror) @setter(SetMirror)
        bool m_mirror = false;
        /// @property @getter(ExecuteInEditMode) @setter(SetExecuteInEditMode)
        bool m_editMode = false;
        /// @property @getter(GetDelay) @setter(SetDelay)
        float_t m_delay = 0.f;

    };
}

#endif //SR_ENGINE_LOOKATCOMPONENT_H
