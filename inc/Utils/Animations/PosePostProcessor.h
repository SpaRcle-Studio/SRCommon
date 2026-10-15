//
// Created by Monika on 15.10.2026.
//

#ifndef SR_ENGINE_UTILS_POSE_POST_PROCESSOR_H
#define SR_ENGINE_UTILS_POSE_POST_PROCESSOR_H

#include <Utils/ECS/Component.h>

namespace SR_ANIMATIONS_NS {
    /// Компонент, который дорабатывает позу скелета после анимации и IK.
    /// Если подключён к Animator, вызывается им, иначе обновляется сам в LateUpdate.
    /// @hidden @abstract @category(Animations)
    class SR_COMMON_DLL_API PosePostProcessor : public SR_UTILS_NS::Component {
        SR_CLASS()
        using Super = SR_UTILS_NS::Component;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<PosePostProcessor>;

    public:
        void LateUpdate() override;

        virtual void UpdatePose(float_t dt) { }

        void SetControlledByAnimator(bool isControlled) noexcept { m_isControlledByAnimator = isControlled; }
        SR_NODISCARD bool IsControlledByAnimator() const noexcept { return m_isControlledByAnimator; }

        /// Сцена не запущена или на паузе. Animator в этом режиме не обновляется
        SR_NODISCARD bool IsScenePaused() const;

    protected:
        bool m_isControlledByAnimator = false;

    };
}

#endif //SR_ENGINE_UTILS_POSE_POST_PROCESSOR_H
