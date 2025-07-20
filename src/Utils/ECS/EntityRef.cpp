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

    const Entity::Ptr& EntityRefBase::GetEntity() const noexcept {
        Resolve();
        return m_pEntity;
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
            SR_WARN("EntityRefBase::Resolve() : Entity with id {} not found!", m_entityId);
            return;
        }

        if (pEntity->GetMeta()->GetFactoryName() != GetTypeName()) SR_UNLIKELY_ATTRIBUTE {
            SR_WARN("EntityRefBase::Resolve() : Entity with id {} is not of type {}! Actual type: {}",
                    m_entityId, GetTypeName(), pEntity->GetMeta()->GetFactoryName());
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