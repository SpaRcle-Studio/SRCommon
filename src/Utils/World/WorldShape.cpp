//
// Created by Monika on 28.11.2026.
//

#include <Utils/World/WorldShape.h>
#include <Utils/World/Scene.h>
#include <Utils/ECS/Transform.h>

#include <Codegen/WorldShape.generated.hpp>

namespace SR_UTILS_NS {
    WorldShape::WorldShape()
        : Super()
    { }

    void WorldShape::OnEnable() {
        RegisterShape();
        Super::OnEnable();
    }

    void WorldShape::OnDisable() {
        UnregisterShape();
        Super::OnDisable();
    }

    void WorldShape::OnDestroy() {
        UnregisterShape();
        Super::OnDestroy();
    }

    void WorldShape::RegisterShape() {
        if (m_pRegisteredScene) {
            return;
        }

        if (auto&& pScene = TryGetScene()) {
            pScene->RegisterWorldShape(this);
            m_pRegisteredScene = pScene;
        }
    }

    void WorldShape::UnregisterShape() {
        if (!m_pRegisteredScene) {
            return;
        }

        m_pRegisteredScene->UnregisterWorldShape(this);
        m_pRegisteredScene = nullptr;
    }

    SR_MATH_NS::FVector3 WorldShape::GetCenter() const {
        if (auto&& pTransform = GetTransform()) {
            return pTransform->GetMatrix().GetTranslate();
        }
        return SR_MATH_NS::FVector3();
    }

    SR_MATH_NS::FVector3 WorldShape::GetUp(const SR_MATH_NS::FVector3& position) const {
        if (!IsSphere()) {
            return SR_MATH_NS::FVector3::Up();
        }

        const SR_MATH_NS::FVector3 fromCenter = position - GetCenter();
        const float_t length = fromCenter.Length();
        if (length < 1e-4f) {
            return SR_MATH_NS::FVector3::Up();
        }

        return fromCenter / length;
    }

    float_t WorldShape::GetHeight(const SR_MATH_NS::FVector3& position) const {
        if (!IsSphere()) {
            return position.y - m_groundHeight;
        }

        return (position - GetCenter()).Length() - m_radius;
    }

    float_t WorldShape::GetDistanceToSurface(const SR_MATH_NS::FVector3& position) const {
        return SR_MATH_NS::Abs(GetHeight(position));
    }

    SR_MATH_NS::FVector4 WorldShape::GetShaderParams(const SR_MATH_NS::FVector3& viewPosition) const {
        if (!IsSphere()) {
            return SR_MATH_NS::FVector4(0.f, m_groundHeight, 0.f, 0.f);
        }

        const SR_MATH_NS::FVector3 center = GetCenter() - viewPosition;
        return SR_MATH_NS::FVector4(center.x, center.y, center.z, m_radius);
    }

    const WorldShape* WorldShape::Find(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::FVector3& position) {
        if (!pScene) {
            return nullptr;
        }

        auto&& shapes = pScene->GetWorldShapes();
        if (shapes.empty()) {
            return nullptr;
        }

        if (shapes.size() == 1) {
            return shapes[0];
        }

        const WorldShape* pNearest = nullptr;
        float_t nearestDistance = SR_MATH_NS::FloatMAX;

        for (auto&& pShape : shapes) {
            const float_t distance = pShape->GetDistanceToSurface(position);
            if (distance < nearestDistance) {
                nearestDistance = distance;
                pNearest = pShape;
            }
        }

        return pNearest;
    }

    SR_MATH_NS::FVector3 WorldShape::FindUp(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::FVector3& position) {
        if (auto&& pShape = Find(pScene, position)) {
            return pShape->GetUp(position);
        }
        return SR_MATH_NS::FVector3::Up();
    }

    float_t WorldShape::FindHeight(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::FVector3& position) {
        if (auto&& pShape = Find(pScene, position)) {
            return pShape->GetHeight(position);
        }
        return position.y;
    }

    SR_MATH_NS::FVector4 WorldShape::FindShaderParams(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::FVector3& viewPosition) {
        if (auto&& pShape = Find(pScene, viewPosition)) {
            return pShape->GetShaderParams(viewPosition);
        }
        return SR_MATH_NS::FVector4(0.f);
    }
}
