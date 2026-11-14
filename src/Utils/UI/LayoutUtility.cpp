//
// Created by Codex on 15.11.2026.
//

#include <Utils/UI/LayoutUtility.h>
#include <Utils/UI/ILayoutElement.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/ECS/Component.h>

namespace SR_UTILS_NS::LayoutUtility {
    Metrics GetMetrics(const GameObject& object, const bool horizontal) {
        SR_TRACY_ZONE;
        Metrics result;
        int32_t priorities[3] = { INT32_MIN, INT32_MIN, INT32_MIN };
        float_t values[3] = { 0.f, 0.f, 0.f };
        for (auto&& pComponent : object.GetComponents()) {
            if (!pComponent->IsActive()) {
                continue;
            }
            auto&& pElement = const_cast<Component*>(pComponent.Get())->GetLayoutElement();
            if (!pElement) {
                continue;
            }
            const float_t inputs[3] = {
                horizontal ? pElement->GetMinWidth() : pElement->GetMinHeight(),
                horizontal ? pElement->GetPreferredWidth() : pElement->GetPreferredHeight(),
                horizontal ? pElement->GetFlexibleWidth() : pElement->GetFlexibleHeight()
            };
            const int32_t priority = pElement->GetLayoutPriority();
            for (uint32_t i = 0; i < 3; ++i) {
                if (inputs[i] < 0.f || priority < priorities[i]) {
                    continue;
                }
                if (priority > priorities[i] || inputs[i] > values[i]) {
                    values[i] = inputs[i];
                    priorities[i] = priority;
                }
            }
        }
        result.minimum = values[0];
        result.preferred = SR_MAX(result.minimum, values[1]);
        result.flexible = values[2];
        return result;
    }

    bool IsIgnored(const GameObject& object) {
        bool hasIgnorer = false;
        for (auto&& pComponent : object.GetComponents()) {
            auto&& pIgnorer = pComponent->GetLayoutIgnorer();
            if (pIgnorer) {
                hasIgnorer = true;
                if (!pIgnorer->IsLayoutIgnored()) {
                    return false;
                }
            }
        }
        return hasIgnorer;
    }
}
