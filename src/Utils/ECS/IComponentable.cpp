//
// Created by Monika on 12.12.2022.
//

#include <Utils/ECS/IComponentable.h>
#include <Utils/ECS/Component.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/World/Scene.h>
#include <Utils/World/SceneUpdater.h>

#include <Codegen/IComponentable.generated.hpp>

namespace SR_UTILS_NS {
    IComponentable::~IComponentable() {
        SRAssert(m_components.empty());
    }

    bool IComponentable::IsDirty() const noexcept {
        return m_dirty;
    }

    Component::Ptr IComponentable::GetOrCreateComponent(StringAtom name) {
        if (auto&& pComponent = GetComponent(name)) {
            return pComponent;
        }

        if (auto&& pComponent = SR_UTILS_NS::Factory::Instance().Create<Component>(name)) {
            if (AddComponent(pComponent)) {
                return pComponent;
            }
            SRHalt("IComponentable::GetOrCreateComponent() : failed to add component! Name: {}", name);
        }

        return nullptr;
    }

    Component::Ptr IComponentable::GetComponent(const std::string& name) {
        return GetComponent(StringAtom(name));
    }

    bool IComponentable::ContainsComponent(const std::string& name) {
        return GetComponent(name);
    }

    bool IComponentable::SetDirty(bool dirty) {
        if (m_hasNotAttachedComponents) {
            m_dirty = true;
            return true;
        }
        return (m_dirty = dirty);
    }

    Component::Ptr IComponentable::GetOrCreateComponent(const std::string& name) {
        return GetOrCreateComponent(StringAtom(name));
    }

    Component::Ptr IComponentable::GetComponent(StringAtom name) {
        for (auto&& pComponent : m_components) {
            if (pComponent->GetMeta()->GetFactoryName() != name) {
                continue;
            }

            return pComponent;
        }

        return nullptr;
    }

    bool IComponentable::HasComponent(const Component::Ptr& pComponent) const {
        for (auto&& pComponentInList : m_components) {
            if (pComponentInList == pComponent) {
                return true;
            }
        }
        return false;
    }

    int32_t IComponentable::GetComponentIndex(const Component::Ptr& pComponent) const {
        auto&& pIt = std::find(m_components.begin(), m_components.end(), pComponent);
        if (pIt == m_components.end()) {
            return -1;
        }
        return static_cast<int32_t>(std::distance(m_components.begin(), pIt));
    }

    bool IComponentable::MoveComponent(const Component::Ptr& pComponent, const int32_t offset) {
        auto&& pIt = std::ranges::find(m_components, pComponent);
        if (pIt == m_components.end()) {
            return false;
        }

        const int64_t currentIndex = std::distance(m_components.begin(), pIt);
        const int64_t newIndex = currentIndex + offset;

        if (newIndex < 0 || newIndex >= m_components.size()) {
            return false;
        }

        if (currentIndex == newIndex) {
            return true;
        }

        if (currentIndex < newIndex) {
            std::rotate(m_components.begin() + currentIndex, m_components.begin() + currentIndex + 1, m_components.begin() + newIndex + 1);
        }
        else {
            std::rotate(m_components.begin() + newIndex, m_components.begin() + currentIndex, m_components.begin() + currentIndex + 1);
        }

        SetDirty(true);

        return true;
    }

    void IComponentable::ForEachComponent(const std::function<bool(const Component::Ptr&)> &fun) const {
        for (uint32_t i = 0; i < m_components.size(); ++i) {
            auto&& pComponent = m_components[i];
            if (!fun(pComponent)) {
                break;
            }
        }
    }

    void IComponentable::ForEachComponent(const std::function<bool(Component::Ptr&)> &fun) {
        for (uint32_t i = 0; i < m_components.size(); ++i) {
            auto&& pComponent = m_components[i];
            if (!fun(pComponent)) {
                break;
            }
        }
    }

    bool IComponentable::AddComponent(const Component::Ptr& pComponent) {
        SR_TRACY_ZONE;

        if (!pComponent) {
            SRHalt("pComponent is nullptr!");
            return false;
        }

        if (!pComponent->IsEntityRegistered() && GetScene()) {
            const uint64_t entityId = pComponent->GetEntityId();
            pComponent->SetEntityId(SR_ID_INVALID);
            GetScene()->GetEntityController()->Register(pComponent.Get(), entityId);
        }

        m_components.emplace_back(pComponent);

        m_hasNotAttachedComponents = true;

        /// Definitely should be here. In other cases Parent is nullptr.
        /// Scene may not exist.
        pComponent->SetParent(this);

        pComponent->OnLoaded();

        /// pComponent->OnAttached();
        /// Здесь нельзя аттачить, иначе будет очень трудно отлавливаемый deadlock и много других проблем

        SetDirty(true);

        return true;
    }

    void IComponentable::RemoveComponents() {
        SR_TRACY_ZONE;

        /// Используем такой проход, так как в процессе удаления может измениться список!
        for (uint32_t i = 0; i < m_components.size(); ++i) { /// NOLINT
            auto&& pComponent = m_components[i];
            DestroyComponent(pComponent);
        }

        m_components.clear();
    }

    bool IComponentable::RemoveComponent(const Component::Ptr& pComponent) {
        auto&& pIt = std::find(m_components.begin(), m_components.end(), pComponent);

        if (pIt == m_components.end()) {
            SR_ERROR("IComponentable::RemoveComponent() : component \"{}\" not found!", pComponent->GetMeta()->GetFactoryName());
            return false;
        }
        m_components.erase(pIt);

        SRAssert2(!pComponent->GetParent() || pComponent->GetParent() == this, "The component does not belong to the game object!");

        DestroyComponent(pComponent);

        return true;
    }

    bool IComponentable::PostLoad(bool force) {
        if (!force && !IsDirty()) {
            return false;
        }

        SR_TRACY_ZONE;

        if (!m_components.empty()) {
            SRAssert2(GetScene(), "Missing scene!");

            for (uint32_t i = 0; i < m_components.size(); ++i) {
                auto&& pComponent = m_components[i];
                if (pComponent->IsAttached()) {
                    continue;
                }

                /// Scene should already exist.
                pComponent->SetParent(this);

                pComponent->OnAttached();
                pComponent->OnMatrixDirty();
            }

            m_hasNotAttachedComponents = false;
        }

        return true;
    }

    void IComponentable::Awake(bool force, bool isPaused) noexcept {
        if (!force && !IsDirty()) {
            return;
        }

        SR_TRACY_ZONE;

        for (uint32_t i = 0; i < m_components.size(); ++i) {
            auto&& pComponent = m_components[i];

            if (!pComponent->IsAttached()) {
                continue;
            }

            if (!pComponent->IsEnabled()) {
                continue;
            }

            if (isPaused && !pComponent->ExecuteInEditMode()) {
                continue;
            }

            if (pComponent->IsAwake()) {
                continue;
            }

            pComponent->Awake();
        }
    }

    void IComponentable::Start(bool force) noexcept {
        if (!force && !IsDirty()) {
            return;
        }

        SR_TRACY_ZONE;

        SetDirty(false);

        if (!IsActive()) {
            return;
        }

        for (uint32_t i = 0; i < m_components.size(); ++i) { /// NOLINT
            auto&& pComponent = m_components[i];

            if (!pComponent->IsAttached()) {
                continue;
            }

            if (!pComponent->IsEnabled()) {
                continue;
            }

            if (!pComponent->IsAwake()) {
                continue;
            }

            if (pComponent->IsStarted()) {
                continue;
            }

            pComponent->Start();
        }
    }

    void IComponentable::CheckActivity(bool force) noexcept {
        if (!force && !IsDirty()) {
            return;
        }

        SR_TRACY_ZONE;

        for (uint32_t i = 0; i < m_components.size(); ++i) { /// NOLINT
            auto&& pComponent = m_components[i];

            if (!pComponent->IsAttached()) {
                continue;
            }

            if (!pComponent->IsAwake()) {
                continue;
            }

            pComponent->CheckActivity();
        }
    }

    void IComponentable::DestroyComponent(const Component::Ptr& pComponent) {
        if (pComponent->IsAttached()) {
            pComponent->OnDetached();
        }

        pComponent->UnregisterEntity();

        if (auto&& pScene = GetScene()) {
            pScene->Remove(pComponent);
        }
        else {
            pComponent->OnDestroy();
        }
        SetDirty(true);
    }

    IComponentable::ScenePtr IComponentable::GetScene() const {
        SRHalt("Not implemented!");
        return nullptr;
    }

    void IComponentable::OnPostLoad() {
        Super::OnPostLoad();

        m_hasNotAttachedComponents = !m_components.empty();

        for (uint32_t i = 0; i < m_components.size(); ++i) {
            m_components[i]->SetParent(this);
            m_components[i]->OnLoaded();
        }
    }

    void IComponentable::OnPriorityChanged() {
        if (IsDestroyed()) {
            return;
        }

        for (uint32_t i = 0; i < m_components.size(); ++i) {
            auto&& pComponent = m_components[i];

            if (!pComponent->IsAttached()) {
                continue;
            }

            pComponent->OnPriorityChanged();
        }
    }

    void IComponentable::OnMatrixDirty() {
        if (IsDestroyed()) SR_UNLIKELY_ATTRIBUTE {
            return;
        }

        const auto size = static_cast<uint32_t>(m_components.size());
        for (uint32_t i = 0; i < size; ++i) {
            auto&& pComponent = m_components[i];

            if (!pComponent->IsAttached()) SR_UNLIKELY_ATTRIBUTE {
                continue;
            }

            pComponent->OnMatrixDirty();
        }
    }
}