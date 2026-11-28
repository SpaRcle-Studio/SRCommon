//
// Created by Monika on 28.11.2026.
//

#include <Utils/World/FloatingOrigin.h>
#include <Utils/World/Scene.h>

#include <Codegen/FloatingOrigin.generated.hpp>

namespace SR_UTILS_NS {
    FloatingOrigin::FloatingOrigin()
        : Super()
    { }

    void FloatingOrigin::OnEnable() {
        RegisterOrigin();
        Super::OnEnable();
    }

    void FloatingOrigin::OnDisable() {
        UnregisterOrigin();
        Super::OnDisable();
    }

    void FloatingOrigin::OnDestroy() {
        UnregisterOrigin();
        Super::OnDestroy();
    }

    void FloatingOrigin::RegisterOrigin() {
        if (m_pRegisteredScene) {
            return;
        }

        if (auto&& pScene = TryGetScene(); pScene && pScene->RegisterFloatingOrigin(this)) {
            m_pRegisteredScene = pScene;
        }
    }

    void FloatingOrigin::UnregisterOrigin() {
        if (!m_pRegisteredScene) {
            return;
        }

        m_pRegisteredScene->UnregisterFloatingOrigin(this);
        m_pRegisteredScene = nullptr;
    }

    bool FloatingOrigin::UpdateObserver(const SR_MATH_NS::FVector3& observerPosition, bool isRunning) {
        if (!m_pRegisteredScene || !observerPosition.IsFinite()) {
            return false;
        }

        if (!isRunning && !m_shiftInEditMode) {
            return false;
        }

        const float_t threshold = SR_MAX(m_threshold, 1.f);
        if (observerPosition.SqrMagnitude() <= threshold * threshold) {
            return false;
        }

        /// Шаг округления - кратно чанку / периоду текстур: сдвинутые сетки совпадают со старыми
        SR_MATH_NS::FVector3 delta = observerPosition;
        if (m_step > 0.f) {
            delta = SR_MATH_NS::FVector3(
                std::round(observerPosition.x / m_step) * m_step,
                std::round(observerPosition.y / m_step) * m_step,
                std::round(observerPosition.z / m_step) * m_step
            );
        }

        if (delta.Empty()) {
            return false;
        }

        ShiftOrigin(delta);

        return true;
    }

    void FloatingOrigin::ShiftOrigin(const SR_MATH_NS::FVector3& delta) {
        SR_TRACY_ZONE;

        auto&& pScene = TryGetScene();
        if (!pScene) {
            return;
        }

        pScene->ShiftOrigin(delta);

        m_originX += static_cast<double_t>(delta.x);
        m_originY += static_cast<double_t>(delta.y);
        m_originZ += static_cast<double_t>(delta.z);

        SR_LOG("FloatingOrigin::ShiftOrigin() : shifted by ({}, {}, {}), absolute origin: ({}, {}, {})",
            delta.x, delta.y, delta.z, m_originX, m_originY, m_originZ);
    }

    SR_MATH_NS::Vector3<double_t> FloatingOrigin::GetOrigin() const noexcept {
        return SR_MATH_NS::Vector3<double_t>(m_originX, m_originY, m_originZ);
    }

    SR_MATH_NS::Vector3<double_t> FloatingOrigin::ToAbsolute(const SR_MATH_NS::FVector3& local) const noexcept {
        return SR_MATH_NS::Vector3<double_t>(
            m_originX + static_cast<double_t>(local.x),
            m_originY + static_cast<double_t>(local.y),
            m_originZ + static_cast<double_t>(local.z)
        );
    }

    SR_MATH_NS::FVector3 FloatingOrigin::ToLocal(const SR_MATH_NS::Vector3<double_t>& absolute) const noexcept {
        return SR_MATH_NS::FVector3(
            static_cast<float_t>(absolute.x - m_originX),
            static_cast<float_t>(absolute.y - m_originY),
            static_cast<float_t>(absolute.z - m_originZ)
        );
    }

    SR_MATH_NS::FVector3 FloatingOrigin::GetOriginApprox() const noexcept {
        return SR_MATH_NS::FVector3(static_cast<float_t>(m_originX), static_cast<float_t>(m_originY), static_cast<float_t>(m_originZ));
    }

    FloatingOrigin* FloatingOrigin::Find(const SR_WORLD_NS::Scene* pScene) {
        return pScene ? pScene->GetFloatingOrigin() : nullptr;
    }

    SR_MATH_NS::Vector3<double_t> FloatingOrigin::FindOrigin(const SR_WORLD_NS::Scene* pScene) {
        if (auto&& pFloatingOrigin = Find(pScene)) {
            return pFloatingOrigin->GetOrigin();
        }
        return SR_MATH_NS::Vector3<double_t>(0.0, 0.0, 0.0);
    }

    SR_MATH_NS::Vector3<double_t> FloatingOrigin::FindAbsolute(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::FVector3& local) {
        if (auto&& pFloatingOrigin = Find(pScene)) {
            return pFloatingOrigin->ToAbsolute(local);
        }
        return local.CastToDouble();
    }

    SR_MATH_NS::FVector3 FloatingOrigin::FindLocal(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::Vector3<double_t>& absolute) {
        if (auto&& pFloatingOrigin = Find(pScene)) {
            return pFloatingOrigin->ToLocal(absolute);
        }
        return absolute.Cast<float_t>();
    }
}
