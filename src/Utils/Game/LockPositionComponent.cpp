//
// Created by Monika on 17.09.2025.
//

#include <Utils/Game/LockPositionComponent.h>

#include <Codegen/LockPositionComponent.generated.hpp>

namespace SR_UTILS_NS {
    void LockPositionComponent::Awake() {
        m_position = GetTransform()->GetTranslation();
        Super::Awake();
    }

    void LockPositionComponent::Update(float_t dt) {
        GetTransform()->SetTranslation(m_position);
        Super::Update(dt);
    }
}