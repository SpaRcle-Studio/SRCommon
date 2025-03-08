//
// Created by Monika on 26.11.2023.
//

#include <Utils/Game/CameraFlyMover.h>

#include <Codegen/CameraFlyMover.generated.hpp>

namespace SR_UTILS_NS {
    void CameraFlyMover::SetExecuteInEditMode(const bool enabled) {
        m_executeInEditorMode = enabled;
        if (auto&& pParent = TryGetParent()) {
            pParent->SetDirty(true);
        }
    }
}