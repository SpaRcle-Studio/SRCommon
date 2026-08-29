//
// Created by Monika on 16.07.2025.
//

#include <Utils/ECS/EntityRef.h>
#include <Utils/ECS/EntityController.h>

#include <Codegen/EntityRef.generated.hpp>

namespace SR_UTILS_NS {
    EntityRefBase::EntityRefBase()
        : Super()
    { }

    StringAtom EntityRefBase::GetTypeName() const noexcept {
        return StringAtom();
    }

    Entity::Ptr EntityRefBase::GetEntity() const noexcept {
        Resolve();
        return m_pEntity;
    }

    bool EntityRefBase::IsApplicable(const SR_HTYPES_NS::SharedPtr<Entity>& pEntity) const noexcept {
        if (!pEntity) {
            SRHalt("EntityRefBase::IsApplicable() : entity is null!");
            return false;
        }

        if (!pEntity->GetMeta()) {
            SRHalt("EntityRefBase::IsApplicable() : entity with id {} has no meta!", pEntity->GetEntityId());
            return false;
        }

        return pEntity->GetMeta()->IsSameOrInherited(GetTypeName());
    }

    void EntityRefBase::OnEntityIdReplaced(const EntityReplaceMap& replaceMap) {
        if (m_entityId == SR_ID_INVALID) SR_UNLIKELY_ATTRIBUTE {
            return;
        }

        if (auto&& pIt = replaceMap.find(m_entityId); pIt != replaceMap.end()) {
            m_entityId = pIt->second;
            m_pEntity = nullptr;
        }
    }

    void EntityRefBase::Resolve() const noexcept {
        if (m_pEntity || m_entityId == SR_ID_INVALID) SR_LIKELY_ATTRIBUTE {
            return;
        }

        auto&& pActiveController = EntityController::GetActiveController();
        if (!pActiveController) SR_UNLIKELY_ATTRIBUTE {
            return;
        }

        auto&& pEntity = pActiveController->FindById(m_entityId);
        if (!pEntity) SR_UNLIKELY_ATTRIBUTE {
            SR_WARN("EntityRefBase::Resolve() : failed to find entity with id {}!", m_entityId);
            m_pEntity = nullptr;
            return;
        }

        if (!IsApplicable(pEntity)) SR_UNLIKELY_ATTRIBUTE {
            SR_WARN("EntityRefBase::Resolve() : entity with id {} is not applicable for this reference!", m_entityId);
            return;
        }

        m_pEntity = pEntity;
    }

    void EntityRefBase::SetEntityId(EntityId entityId) noexcept {
        if (m_entityId == entityId) SR_LIKELY_ATTRIBUTE {
            return;
        }

        m_entityId = entityId;
        m_pEntity = nullptr;
        Resolve();
    }

    EntityRefBase::operator bool() const noexcept {
        return GetEntity();
    }

    bool EntityRefBase::IsResolved() const noexcept {
        return m_pEntity;
    }
}