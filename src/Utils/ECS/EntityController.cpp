//
// Created by Monika on 08.01.2022.
//

#include <Utils/ECS/EntityController.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Common/VectorUtils.h>
#include <Utils/Common/Numeric.h>
#include <Utils/TypeTraits/SRClassMeta.h>

namespace SR_UTILS_NS {
    std::atomic<EntityController*> EntityController::ACTIVE_CONTROLLER = nullptr;

    EntityController::EntityController()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Manually)
    { }

    EntityController::~EntityController() {
        if (ACTIVE_CONTROLLER == this) {
            ACTIVE_CONTROLLER = nullptr;
        }

        if (!m_entities.empty()) {
            std::string ids;
            uint32_t index = 0;

            for (const auto& [id, pEntity] : m_entities) {
                if (SRVerify2(pEntity, "Invalid entity!")) {
                    ids += "\n\t[{}] = {}; Info = {}"_format(index++, id, pEntity->GetMeta()->GetFactoryName());
                }
            }

            SRHalt("EntityController() : entities were not destroyed! Crash is possible. Ids and info: " + ids);
        }
    }

    bool EntityController::Reserve(const EntityId& id) {
        SR_TRACY_ZONE;

        if (id == SR_ID_INVALID) {
            SRHalt("EntityController::Reserve() : trying to reserve invalid id!");
            return false;
        }

    #ifdef SR_DEBUG
        if (m_reserved.count(id)) {
            SRHalt("EntityController::Reserve() : entity already reserved! Id: " + std::to_string(id));
            return false;
        }
    #endif

        m_reserved.insert(id);

        return true;
    }

    Entity::Ptr EntityController::GetReserved(const EntityId& id, const EntityAllocator& allocator) {
        SR_TRACY_ZONE;

    #ifdef SR_DEBUG
        if (m_entities.count(id) != 0) {
            SRHalt("EntityController::GetReserved() : entity is registered! Something went wrong... \n\tId: " + std::to_string(id));
            return nullptr;
        }
    #endif

        if (m_reserved.count(id) != 0) {
            m_nextId = id;
            return allocator();
        }

        SRHalt("Entity id isn't reserved! \n\tId: " + std::to_string(id));

        return nullptr;
    }

    EntityId EntityController::Register(const Entity::Ptr& pEntity, EntityId wantedId) {
        SRAssert(pEntity);
        SRAssert2(!pEntity->IsEntityRegistered(), "Entity already registered!");

        if (pEntity->GetEntityId() != SR_ID_INVALID) SR_UNLIKELY_ATTRIBUTE {
            SRHalt("Entity already has id! Id: {}", pEntity->GetEntityId());
            return pEntity->GetEntityId();
        }

        EntityId id = m_nextId;

        if (wantedId != SR_ID_INVALID && !IsIdUsed(wantedId)) {
            id = wantedId;
            goto complete;
        }

        if (id != SR_ID_INVALID) {
            m_reserved.erase(m_nextId);
            m_nextId = SR_ID_INVALID;
            goto complete;
        }

    retry:
        id = Random::Instance().UInt64();

        if (m_entities.count(id) || m_reserved.count(id) || id == SR_ID_INVALID) {
            SR_WARN("EntityController::Register() : collision detected! Id: " + std::to_string(id));
            goto retry;
        }

    complete:
        m_entities.insert(std::make_pair(id, pEntity));

        pEntity->SetEntityId(id);
        pEntity->SetEntityController(this);

        return id;
    }

    void EntityController::Unregister(const EntityId& id) {
        SR_TRACY_ZONE;

        if (id == SR_ID_INVALID) {
            SRHalt("EntityController::Unregister() : trying to unregister entity with invalid id!");
            return;
        }

        auto&& pIt = m_entities.find(id);
        if (pIt == m_entities.end()) {
            SRHalt0();
            return;
        }

        pIt->second->SetEntityController(nullptr);
        m_entities.erase(id);
    }

    bool EntityController::IsIdUsed(const EntityId& id) const {
        return m_entities.count(id) != 0 || m_reserved.count(id) != 0;
    }


    Entity::Ptr EntityController::FindById(const EntityId& id) const {
        SR_TRACY_ZONE;

        if (!m_entities.count(id)) {
            return nullptr;
        }

        return m_entities.at(id);
    }

    bool EntityController::UnReserve(const EntityId &id) {
        if (TryUnReserve(id)) {
            return true;
        }

        SRHalt("Entity isn't reserved! Id: " + std::to_string(id));

        return false;
    }

    bool EntityController::TryUnReserve(const EntityId &id) {
        SR_TRACY_ZONE;

        if (m_reserved.count(id) == 0) {
            return false;
        }

        m_reserved.erase(id);

        return true;
    }

    EntityController* EntityController::GetActiveController() {
        return ACTIVE_CONTROLLER.load(std::memory_order_acquire);
    }

    void EntityController::SetActiveController() {
        ACTIVE_CONTROLLER.store(this, std::memory_order_release);
    }

    /// ----------------------------------------------------------------------------------------------------------------

    void EntityIdList::ReserveIds() {
        if (!SRVerify2(!m_isReserved, "Ids already reserved!")) {
            return;
        }

        if (SRVerify(m_pController)) {
            for (const EntityId& id : m_entityIds) {
                m_pController->Reserve(id);
            }
        }

        m_isReserved = true;
    }

    void EntityIdList::UnReserveIds() {
        if (!SRVerify2(m_isReserved, "Ids are not reserved!")) {
            return;
        }

        if (SRVerify(m_pController)) {
            for (const EntityId& id : m_entityIds) {
                m_pController->UnReserve(id);
            }
        }

        m_isReserved = false;
    }

    void EntityIdList::Clear() {
        SRAssert2(!m_isReserved, "Ids are reserved!");
        m_entityIds.clear();
    }

    void EntityIdList::Add(EntityId id) {
        SRAssert2(!m_isReserved, "Ids are reserved!");
        SRAssert2(id != SR_ID_INVALID, "Invalid id!");
        m_entityIds.emplace_back(id);
    }

    void EntityIdList::Add(const EntityIdList& list) {
        SRAssert2(!m_isReserved, "Ids are reserved!");
        m_entityIds.insert(m_entityIds.end(), list.m_entityIds.begin(), list.m_entityIds.end());
    }

    void EntityIdList::Add(const std::vector<EntityId>& list) {
        SRAssert2(!m_isReserved, "Ids are reserved!");
        m_entityIds.insert(m_entityIds.end(), list.begin(), list.end());
    }
}
