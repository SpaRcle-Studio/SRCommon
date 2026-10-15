//
// Created by Monika on 15.10.2026.
//

#include <Utils/Animations/PosePostProcessor.h>
#include <Utils/Types/Time.h>
#include <Utils/World/Scene.h>
#include <Utils/World/SceneUpdater.h>

#include <Codegen/PosePostProcessor.generated.hpp>

namespace SR_ANIMATIONS_NS {
    void PosePostProcessor::LateUpdate() {
        SR_TRACY_ZONE;
        Super::LateUpdate();

        /// На паузе Animator не обновляется, поэтому вызываем сами
        if (m_isControlledByAnimator && !IsScenePaused()) {
            return;
        }

        UpdatePose(SR_HTYPES_NS::Time::Instance().DeltaTime());
    }

    bool PosePostProcessor::IsScenePaused() const {
        auto&& pScene = TryGetScene();
        if (!pScene || !pScene->GetSceneUpdater()) {
            return true;
        }
        return pScene->GetSceneUpdater()->IsPaused();
    }
}
