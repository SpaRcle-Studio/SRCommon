//
// Created by Monika on 12.12.2022.
//

#include <Utils/ECS/IComponentable.h>
#include <Utils/ECS/Component.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/World/Scene.h>
#include <Utils/World/SceneUpdater.h>

namespace SR_UTILS_NS {
    IComponentable::~IComponentable() {
        SRAssert(m_components.empty());
        SRAssert(m_loadedComponents.empty());
    }

    bool IComponentable::IsDirty() const noexcept {
        return m_dirty;
    }

    SR_HTYPES_NS::Marshal::Ptr IComponentable::SaveComponents(SavableContext data) const {
        if (!data.pMarshal) {
            data.pMarshal = new SR_HTYPES_NS::Marshal();
        }

        std::vector<SR_HTYPES_NS::Marshal::Ptr> components;
        components.reserve(m_components.size() + m_loadedComponents.size());

        const auto componentSaveData = SR_UTILS_NS::SavableContext(nullptr, data.flags);

        for (auto&& pComponent : m_components) {
            if (auto&& pMarshalComponent = pComponent->Save(componentSaveData)) {
                components.emplace_back(pMarshalComponent);
            }
        }

        for (auto&& pComponent : m_loadedComponents) {
            if (auto&& pMarshalComponent = pComponent->Save(componentSaveData)) {
                components.emplace_back(pMarshalComponent);
            }
        }

        data.pMarshal->Write(static_cast<uint16_t>(components.size()));

        for (auto&& pMarshalComponent : components) {
            data.pMarshal->Write<uint32_t>(pMarshalComponent->Size());
            data.pMarshal->Append(pMarshalComponent);
        }

        return data.pMarshal;
    }

    Component* IComponentable::GetOrCreateComponent(StringAtom name) {
        if (auto&& pComponent = GetComponent(name)) {
            return pComponent;
        }

        if (auto&& pComponent = ComponentManager::Instance().CreateComponentOfName(name)) {
            if (AddComponent(pComponent)) {
                return pComponent;
            }
            SRHalt("IComponentable::GetOrCreateComponent() : failed to add component!");
        }

        return nullptr;
    }

    Component* IComponentable::GetComponent(const std::string& name) {
        return GetComponent(StringAtom(name));
    }

    bool IComponentable::ContainsComponent(const std::string& name) {
        return GetComponent(name);
    }

    Component* IComponentable::GetOrCreateComponent(const std::string& name) {
        return GetOrCreateComponent(StringAtom(name));
    }

    Component* IComponentable::GetComponent(StringAtom name) {
        for (auto&& pComponent : m_loadedComponents) {
            if (pComponent->GetComponentName() != name) {
                continue;
            }

            return pComponent;
        }

        for (auto&& pComponent : m_components) {
            if (pComponent->GetComponentName() != name) {
                continue;
            }

            return pComponent;
        }

        return nullptr;
    }

    void IComponentable::ForEachComponent(const std::function<bool(Component *)> &fun) {
        for (uint32_t i = 0; i < m_components.size(); ++i) {
            auto&& pComponent = m_components[i];
            if (!fun(pComponent)) {
                break;
            }
        }

        for (auto pIt = m_loadedComponents.begin(); pIt != m_loadedComponents.end(); ++pIt) {
            if (!fun(*pIt)) {
                break;
            }
        }
    }

    bool IComponentable::AddComponent(Component* pComponent) {
        if (!pComponent) {
            SRHalt("pComponent is nullptr!");
            return false;
        }

        m_loadedComponents.emplace_back(pComponent);

        /// Definitely should be here. In other cases Parent is nullptr.
        /// Scene may not exist.
        pComponent->SetParent(this);

        pComponent->OnLoaded();

        /// pComponent->OnAttached();
        /// Здесь нельзя аттачить, иначе будет очень трудно отлавливаемый deadlock и много других проблем

        SetDirty(true);

        return true;
    }

    bool IComponentable::RemoveComponent(Component* pComponent) {
        auto&& pIt = std::find(m_components.begin(), m_components.end(), pComponent);

        if (pIt == m_components.end()) {
            auto&& pLoadedIt = std::find_if(m_loadedComponents.begin(), m_loadedComponents.end(), [&](auto&& pElement) {
                return pComponent == pElement;
            });

            if (pLoadedIt == m_loadedComponents.end()) {
                SR_ERROR("IComponentable::RemoveComponent() : component \"" + pComponent->GetComponentName().ToStringRef() + "\" not found!");
                return false;
            }
            else {
                m_loadedComponents.erase(pLoadedIt);
            }
        }
        else {
            m_components.erase(pIt);
        }

        SRAssert2(!pComponent->GetParent() || pComponent->GetParent() == this, "The component does not belong to the game object!");

        DestroyComponent(pComponent);

        return true;
    }

    bool IComponentable::PostLoad(bool force) {
        if (!force && !IsDirty()) {
            return false;
        }

        if (!m_loadedComponents.empty()) {
            SRAssert2(GetScene(), "Missing scene!");

            m_components.reserve(m_loadedComponents.size());

            while (!m_loadedComponents.empty()) {
                auto&& pLoadedCmp = m_loadedComponents.front();
                m_components.emplace_back(pLoadedCmp);

                /// Scene should already exist.
                pLoadedCmp->SetParent(this);

                pLoadedCmp->OnAttached();
                pLoadedCmp->OnMatrixDirty();

                m_loadedComponents.pop_front();
            }
        }

        return true;
    }

    void IComponentable::Awake(bool force, bool isPaused) noexcept {
        if (!force && !IsDirty()) {
            return;
        }

        for (uint32_t i = 0; i < m_components.size(); ++i) {
            auto&& pComponent = m_components[i];
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

        SetDirty(false);

        if (!IsActive()) {
            return;
        }

        for (uint32_t i = 0; i < m_components.size(); ++i) { /// NOLINT
            auto&& pComponent = m_components[i];
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

        for (uint32_t i = 0; i < m_components.size(); ++i) { /// NOLINT
            auto&& pComponent = m_components[i];
            if (!pComponent->IsAwake()) {
                continue;
            }

            pComponent->CheckActivity();
        }
    }

    void IComponentable::DestroyComponents() {
        SR_TRACY_ZONE;

        /// Используем такой проход, так как в процессе удаления может измениться список!
        for (uint32_t i = 0; i < m_components.size(); ++i) { /// NOLINT
            auto&& pComponent = m_components[i];
            DestroyComponent(pComponent);
        }

        for (auto&& pComponent : m_loadedComponents) {
            DestroyComponent(pComponent);
        }

        m_loadedComponents.clear();
        m_components.clear();
    }

    void IComponentable::DestroyComponent(Component* pComponent) {
        if (pComponent->IsAttached()) {
            pComponent->OnDetached();
        }

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

    void IComponentable::OnPriorityChanged() {
        if (IsDestroyed()) {
            return;
        }

        for (uint32_t i = 0; i < m_components.size(); ++i) {
            auto&& pComponent = m_components[i];
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
            pComponent->OnMatrixDirty();
        }
    }
}