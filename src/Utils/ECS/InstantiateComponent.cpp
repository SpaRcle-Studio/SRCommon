//
// Created by Monika on 29.01.2026.
//

#include <Utils/Game/InstantiateComponent.h>
#include <Utils/ECS/Prefab.h>

#include <Codegen/InstantiateComponent.generated.hpp>

namespace SR_UTILS_NS {
    void InstantiateComponent::DoInstantiate() {
        SR_TRACY_ZONE;

        auto&& pTarget = m_target.Get();
        if (!pTarget) {
            SR_WARN("InstantiateComponent::DoInstantiate() : target is nullptr!");
            return;
        }

        SR_UTILS_NS::Prefab::Ptr pPrefab;

        for (uint32_t i = 0; i < m_count; ++i) {
            SR_UTILS_NS::SceneObject::Ptr pInstance;

            if (m_instanceMode == InstantiateComponentInstanceMode::Clone) {
                auto&& pSource = m_source.Get();
                if (!pSource) {
                    SR_WARN("InstantiateComponent::DoInstantiate() : source is nullptr!");
                    return;
                }

                pInstance = pSource->CloneSceneObject();
                pTarget->AddChild(pInstance);
            }
            else if (m_instanceMode == InstantiateComponentInstanceMode::Prefab) {
                if (!pPrefab) {
                    pPrefab = SR_UTILS_NS::Prefab::Load(m_prefabId);
                    pPrefab->AddUsePoint();
                }

                if (!pPrefab) {
                    SR_WARN("InstantiateComponent::DoInstantiate() : failed to load prefab with id {}!", m_prefabId);
                    return;
                }

                pInstance = pPrefab->Instance(pTarget->GetScene());
                if (pInstance) {
                    pTarget->AddChild(pInstance);

                }
                else {
                    SR_WARN("InstantiateComponent::DoInstantiate() : failed to instance prefab with id {}!", m_prefabId);
                }
            }
            else {
                SRHalt("InstantiateComponent::DoInstantiate() : unknown instance mode!");
            }

            if (pInstance) {
                if (m_activityOnInstantiate) {
                    pInstance->SetEnabled(*m_activityOnInstantiate);
                }
                if (m_instanceName) {
                    pInstance->SetName(*m_instanceName);
                }
            }
        }

        if (pPrefab) {
            pPrefab->RemoveUsePoint();
        }
    }

    void InstantiateComponent::Start() {
        if (m_activateMode == InstantiateComponentActivateMode::OnStart) {
            DoInstantiate();
        }
        Super::Start();
    }

    void InstantiateComponent::OnEnable() {
        if (m_activateMode == InstantiateComponentActivateMode::OnEnable) {
            DoInstantiate();
        }
        Super::OnEnable();
    }
}