//
// Created by Codex on 15.11.2026.
//

#include <Utils/UI/LayoutTestMetrics.h>
#include <Utils/ECS/TransformRect.h>

#include <Codegen/LayoutTestMetrics.generated.hpp>

namespace SR_UTILS_NS {
    void LayoutTestMetrics::CalculateLayoutInput(const bool horizontal) {
        if (horizontal) {
            return;
        }
        if (auto&& pRect = GetTransformAs<TransformRect>()) {
            const float_t columns = SR_MAX(1.f, std::floor(pRect->GetLayoutRect().Width() / SR_MAX(0.001f, m_glyphWidth)));
            m_preferredHeight = SR_MAX(1.f, std::ceil(static_cast<float_t>(m_glyphCount) / columns)) * m_lineHeight;
        }
    }
}
