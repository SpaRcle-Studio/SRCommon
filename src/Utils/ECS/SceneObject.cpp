//
// Created by Monika on 04.10.2024.
//

#include <Utils/ECS/Component.h>
#include <Utils/ECS/LayerManager.h>
#include <Utils/ECS/Prefab.h>
#include <Utils/ECS/SceneObject.h>
#include <Utils/World/SceneUpdater.h>

#include <Codegen/SceneObject.generated.hpp>

namespace SR_UTILS_NS {
    SceneObject::SceneObject(ObjectNameT name) {
        SetName(name);
        UpdateEntityPath();
    }

    SceneObject::~SceneObject() {
        SRAssert2(m_children.empty(), "SceneObject has children!");
        if (GetPrefab()) {
            UnlinkPrefab();
        }
        SRAssert(!GetPrefab());
    }

    SR_HTYPES_NS::Marshal::Ptr SceneObject::SaveLegacy(SavableContext data) const {
        if (!(data.pMarshal = Entity::SaveLegacy(data))) {
            return data.pMarshal;
        }

        return Entity::SaveLegacy(data);
    }

    SceneObject::Ptr SceneObject::Copy(const ScenePtr& pScene, const SceneObject::Ptr& pObject) const {
        pObject->SetEnabled(IsEnabled());

        pObject->SetTag(GetTag());
        pObject->SetLayer(GetLayer());

        if (pScene) {
            pScene->RegisterSceneObject(pObject);
        }

        for (auto&& pComponent : m_components) {
            pObject->AddComponent(pComponent->CopyComponent());
        }

        for (auto&& children : GetChildrenRef()) {
            pObject->AddChild(children->Copy(pScene, nullptr));
        }

        if (IsPrefabOwner()) {
            pObject->SetPrefab(GetPrefab(), true);
        }

        return pObject;
    }

    SceneObject::Ptr SceneObject::Find(const std::string_view& name) const noexcept {
        return Find(SR_HASH_STR_REGISTER(name));
    }

    bool SceneObject::UpdateEntityPath() {
        SceneObject::Ptr pCurrent = this;
        EntityPath path;

        do {
            path.ConcatBack(pCurrent->GetEntityId());
            pCurrent = pCurrent->m_parent;

            if (pCurrent && pCurrent->GetEntityId() == GetEntityId()) {
                SRHalt("Recursive entity path!");
                return false;
            }
        } while (pCurrent.Valid());

        SetEntityPath(path);

        return true;
    }

    std::list<EntityBranch> SceneObject::GetEntityBranches() const {
        std::list<EntityBranch> branches;

        ForEachChild([&branches](const SceneObject::Ptr &ptr) {
            branches.emplace_back(ptr->GetEntityTree());
        });

        return std::move(branches);
    }

    bool SceneObject::MoveToTree(const SceneObject::Ptr &destination) {
        if (m_parent) {
            if (const SceneObject::Ptr pCopy = m_parent) {
                pCopy->RemoveChild(this);
            }
        }

        if (destination.Valid()){
            return destination->AddChild(this);
        }

        if (GetParent()){
            SRHalt("GameObject::MoveToTree() : GameObject has parent!");
            return false;
        }

        return true;
    }

    bool SceneObject::SetDirty(bool dirty) {
        if (IsDirty() == dirty) {
            return dirty;
        }

        if (IComponentable::SetDirty(dirty) && !dirty) {
            return true; /// несмогли очистить флаг, объект еще грязный
        }

        /// Грязный флаг передаем вверх, а чистый вниз.
        /// Это нужно для оптимизации
        if (dirty) {
            if (m_parent) {
                m_parent->SetDirty(dirty);
            }
            else if (m_scene) {
                /// дошли до верха иерархии, сообщаем о необходимости обновления дерева сцены
                m_scene->GetSceneUpdater()->SetDirty();
            }

            return true;
        }

        for (auto&& children : m_children) {
            if (children->SetDirty(dirty)) {
                IComponentable::SetDirty(true);
                return true; /// несмогли очистить флаг, объект еще грязный
            }
        }

        return false;
    }

    void SceneObject::SetPrefab(Prefab* pPrefab, bool isOwner) { /// NOLINT (recursion)
        SRAssert2(pPrefab, "Invalid prefab!");
        SRAssert2(!GetPrefab() && !IsPrefabOwner(), "Prefab is already set!");

        if (pPrefab && !GetPrefab()) {
            m_prefabInfo.pPrefab = pPrefab;
            m_prefabInfo.pPrefab->AddUsePoint();
        }
        else {
            return;
        }

        m_prefabInfo.isOwner = isOwner;

        for (auto&& child : m_children) {
            /// наткнулись на другой префаб
            if (child->IsPrefabOwner()) {
                continue;
            }
            child->SetPrefab(pPrefab, false);
        }
    }

    void SceneObject::UnlinkPrefab() { /// NOLINT (recursion)
        SRAssert2(GetPrefab(), "Is not a prefab!");

        m_prefabInfo.isOwner = false;

        if (m_prefabInfo.pPrefab) {
            m_prefabInfo.pPrefab->RemoveUsePoint();
            m_prefabInfo.pPrefab = nullptr;
        }

        for (auto&& pChild : m_children) {
            /// наткнулись на другой префаб или он не задан
            if (pChild->IsPrefabOwner() || !pChild->GetPrefab()) {
                continue;
            }
            pChild->UnlinkPrefab();
        }
    }

    void SceneObject::SetLayer(StringAtom layer) {
        SRAssert(!layer.Empty());

        if (LayerManager::GetDefaultLayer() == layer && m_parent) {
            layer = m_parent->GetLayer();
        }

        if (layer == m_layer && m_cachedLayer == m_layer) {
            return;
        }

        ForEachComponent([](Component* pComponent) -> bool {
            pComponent->OnBeforeLayerChanged();
            return true;
        });

        m_cachedLayer = m_layer = layer;

        ForEachComponent([](Component* pComponent) -> bool {
            pComponent->OnLayerChanged();
            return true;
        });

        for (auto&& pChild : m_children) {
            pChild->OnParentLayerChanged();
        }
    }

    SceneObject::Ptr SceneObject::Find(StringAtom name) const noexcept {
        for (auto&& pChild : m_children) {
            if (pChild->GetName() == name) {
                return pChild;
            }
        }

        return SceneObject::Ptr();
    }

    bool SceneObject::PostLoad(bool force) {
        if (!IComponentable::PostLoad(force)) {
            return false;
        }

        for (auto&& pChild : m_children) {
            pChild->PostLoad(true);
        }

        return true;
    }

    void SceneObject::SetEnabled(bool value) {
        if (m_isEnabled == value) {
            return;
        }

        m_isEnabled = value;

        SetDirty(true);
    }

    void SceneObject::CheckActivity(bool force) noexcept { /// NOLINT (recursion)
        if (!force && !IsDirty()) {
            return;
        }

        const bool isActivePrev = m_isActive;
        m_isActive = IsEnabled() && (!m_parent || m_parent->m_isActive);

        /// нет смысла продолжать цепочку, все и так выключено
        if (!m_isActive && m_isActive == isActivePrev) {
            return;
        }

        /// обновляем компоненты
        IComponentable::CheckActivity(force);

        for (auto&& pChild : m_children) {
            pChild->CheckActivity(true);
        }
    }

    void SceneObject::Awake(bool force, bool isPaused) noexcept { /// NOLINT (recursion)
        /// Проверяем на IsEnabled а не на IsActive,
        /// так как если родитель не активен, то метод не вызвался бы.
        if ((!force && !IsDirty()) || !IsEnabled()) {
            return;
        }

        IComponentable::Awake(force, isPaused);

        for (auto&& pChild : m_children) {
            pChild->Awake(true, isPaused);
        }
    }

    void SceneObject::Start(bool force) noexcept { /// NOLINT (recursion)
        if (!force && !IsDirty()) {
            return;
        }

        IComponentable::Start(force);

        for (auto&& pChild : m_children) {
            pChild->Start(true);
        }
    }

    void SceneObject::OnParentLayerChanged() { /// NOLINT (recursion)
        if (m_layer != LayerManager::GetDefaultLayer()) {
            return;
        }

        SRAssert(m_parent);
        SRAssert(!m_layer.Empty());

        if (m_cachedLayer == m_parent->m_cachedLayer) {
            return;
        }

        ForEachComponent([](Component* pComponent) -> bool {
            pComponent->OnBeforeLayerChanged();
            return true;
        });

        m_cachedLayer = m_parent->m_cachedLayer;

        ForEachComponent([](Component* pComponent) -> bool {
            pComponent->OnLayerChanged();
            return true;
        });

        for (auto&& pChild : m_children) {
            pChild->OnParentLayerChanged();
        }
    }


    void SceneObject::Destroy() {
        SR_TRACY_ZONE;

        if (m_isDestroyed) {
            SRHalt("SceneObject::Destroy() : \"" + m_name.ToString() + "\" game object already destroyed!");
            return;
        }

        m_isDestroyed = true;

        /// сцену не блокируем, предполагается, что и так в контексте заблокированной сцены работаем

        if (GetPrefab()) {
            UnlinkPrefab();
        }

        if (auto&& pParent = GetParent()) {
            auto&& pThisPtr = GetThis().DynamicCast<SceneObject>();
            pParent->RemoveChild(pThisPtr);
        }

        if (m_scene) {
            m_scene->Remove(GetThis().DynamicCast<SceneObject>());
            while (!m_children.empty()) {
                (*m_children.begin())->Destroy();
            }
        }
        else {
            while (!m_children.empty()) {
                auto&& pChild = *m_children.begin();
                if (pChild) {
                    pChild->Destroy();
                }
                else {
                    SRHalt("SceneObject::Destroy() : child is nullptr!");
                    m_children.erase(m_children.begin());
                }
            }

            DestroyComponents();
            DestroyImpl();
        }
    }

    void SceneObject::DestroyImpl() {
        /// это должно быть единственное место,
        /// где мы уничтожаем объект
        AutoFree();
    }

    void SceneObject::UpdateRoot() {
        SR_TRACY_ZONE;
        m_root = m_parent ? m_parent->m_root : this;
        for (auto&& pChild : m_children) {
            pChild->UpdateRoot();
        }
    }

    void SceneObject::SetTag(SR_UTILS_NS::StringAtom tag) {
        m_tag = tag;
    }

    StringAtom SceneObject::GetTag() const {
        return m_tag;
    }

    std::string SceneObject::GetEntityInfo() const {
        return "SceneObject: " + GetName();
    }

    SceneObject::Ptr SceneObject::Find(uint64_t hashName) const noexcept {
        for (auto&& pChild : m_children) {
            if (pChild->GetName() == hashName) {
                return pChild;
            }
        }

        return SceneObject::Ptr();
    }

    SceneObject::Ptr SceneObject::Find(const std::string& name) const noexcept {
        return Find(SR_HASH_STR(name));
    }

    bool SceneObject::AddChild(const SceneObject::Ptr& pChild) {
        if (pChild.Get() == this) {
            SRHalt("It is impossible to make the parent a child!");
            return false;
        }

        if (pChild->GetParent()) {
            SRHalt("Child has parent!");
            return false;
        }

        if (Contains(pChild)) {
            SRHalt("This child already exists in this game object!");
            return false;
        }

        if (!pChild->SetParent(GetThis().DynamicCast<SceneObject>())) {
            SR_WARN("SceneObject::AddChild() : failed to set parent!");
            return false;
        }

        m_children.emplace_back(pChild);

        pChild->OnParentLayerChanged();
        pChild->OnAttached();

        if (m_scene) {
            m_scene->OnChanged();
        }

        SetDirty(true);

        return true;
    }

    void SceneObject::SetName(const ObjectNameT name) {
        SR_TRACY_ZONE;
        m_name = name;
        if (m_scene) {
            m_scene->OnChanged();
        }
    }

    void SceneObject::SetIdInScene(uint64_t id) {
        m_idInScene = id;
    }

    void SceneObject::SetScene(ScenePtr pScene) {
        SRAssert(!m_scene);
        m_scene = pScene;
    }

    bool SceneObject::Contains(const SceneObject::Ptr& pChild) {
        SR_TRACY_ZONE;
        for (auto&& pChildren : m_children) {
            if (pChildren == pChild) {
                return true;
            }
        }
        return false;
    }

    bool SceneObject::SetParent(const SceneObject::Ptr& pParent) {
        SR_TRACY_ZONE;

        if (pParent == m_parent) {
            SRHalt("GameObject::SetParent() : parent is already set!");
            return false;
        }

        const SceneObject::Ptr pOldParent = m_parent;
        m_parent = pParent;

        UpdateRoot();

        if (!UpdateEntityPath()) {
            SRHalt("GameObject::SetParent() : failed to update entity path!");
            m_parent = pOldParent;
            UpdateRoot();
            return false;
        }

        if (m_scene) {
            m_scene->OnChanged();
        }

        if (!m_isDestroyed) {
            OnHierarchyChanged();
        }

        return true;
    }

    void SceneObject::RemoveChild(const SceneObject::Ptr& pChild) {
        SR_TRACY_ZONE;
        pChild->SetParent(SceneObject::Ptr());

        for (uint16_t i = 0; i < m_children.size(); ++i) {
            if (pChild == m_children[i]) {
                m_children.erase(m_children.begin() + i);
                return;
            }
        }

        SRHalt("SceneObject {} is not child for {}!", pChild->GetName().c_str(), GetName().c_str());
    }

    void SceneObject::RemoveAllChildren() {
        for (auto&& pChild : m_children) {
            pChild->SetParent(SceneObject::Ptr());
        }
        m_children.clear();
    }

    void SceneObject::ForEachChild(const std::function<void(SceneObject::Ptr&)>& fn) {
        for (auto&& child : m_children) {
            if (child.Valid()) {
                fn(child);
            }
        }
    }

    void SceneObject::ForEachChild(const std::function<void(const SceneObject::Ptr&)>& fn) const {
        for (auto&& child : m_children) {
            if (child.RecursiveLockIfValid()) {
                fn(child);
                child.Unlock();
            }
        }
    }
}
