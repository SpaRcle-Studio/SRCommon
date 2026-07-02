//
// Created by Monika on 04.10.2024.
//

#include <Utils/ECS/Component.h>
#include <Utils/ECS/LayerManager.h>
#include <Utils/ECS/Prefab.h>
#include <Utils/ECS/SceneObject.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/Reflection/SRClassUtils.h>
#include <Utils/World/SceneUpdater.h>
#include <Utils/World/Scene.h>
#include <Utils/FileSystem/PathDataAccessor.h>
#include <Utils/Serialization/SRASerialization.h>

#include <Codegen/SceneObject.generated.hpp>

namespace SR_UTILS_NS {
    SceneObject::SceneObject()
        : Super()
    {
        SetLayer(SR_UTILS_NS::LayerManager::GetDefaultLayer());
        SetTag(SR_UTILS_NS::TagManager::GetDefaultTag());
    }

    SceneObject::SceneObject(StringAtom name)
        : Super()
        , m_name(name)
    {
        SetLayer(SR_UTILS_NS::LayerManager::GetDefaultLayer());
        SetTag(SR_UTILS_NS::TagManager::GetDefaultTag());
    }

    SceneObject::~SceneObject() {
        SRAssert2(m_children.empty(), "SceneObject has children!");
        if (GetPrefab()) {
            UnlinkPrefab();
        }
        SRAssert(!GetPrefab());
    }

    SceneObject::Ptr SceneObject::Find(const std::string_view& name) const noexcept {
        return Find(SR_HASH_STR_REGISTER(name));
    }

    SceneObject::Ptr SceneObject::CloneSceneObject() const {
        SR_TRACY_ZONE;

        SR_UTILS_NS::SceneObject::Ptr pClone = SR_UTILS_NS::Factory::Instance().Create<SceneObject>(GetMeta()->GetFactoryName());
        if (!pClone) {
            SRHalt("Failed to clone scene object! Factory returned nullptr!");
            return nullptr;
        }

        CloneTo(*pClone);

        return pClone;

        /// TODO: тут делается сериализация и десериализация, нужно сделать нормальный метод клонирования
        /*SR_UTILS_NS::SRASerializer serializer;
        Save(serializer);

        auto&& pDeserializer = serializer.CreateDeserializer();

        SR_UTILS_NS::SceneObject::Ptr pSceneObject;

        switch (GetSceneObjectType()) {
        case SceneObjectType::GameObject:
                pSceneObject = SR_UTILS_NS::GameObject::MakeShared<GameObject, SceneObject>();
                break;
            default:
                SRHalt("Unknown scene object type!");
                return nullptr;
        }

        pSceneObject->Load(*pDeserializer);
        return pSceneObject;*/
    }

    bool SceneObject::Load(IDeserializer& deserializer) {
        SR_TRACY_ZONE;

        if (!Super::Load(deserializer)) {
            return false;
        }

        static constexpr SR_UTILS_NS::SerializationId prefabId = SR_UTILS_NS::SerializationId::Create("prefab");
        std::string prefabPath;
        deserializer.ReadString(prefabPath, prefabId);

        if (!prefabPath.empty()) {
            if (auto&& pPrefab = CoreResLoader::Load<Prefab>(SR_UTILS_NS::Path(prefabPath))) {
                if (GetComponentsCount() > 0) {
                    SR_ERROR("SceneObject::Load() : prefab not loaded, but components are present! Path: {}", prefabPath);
                    RemoveComponents();
                }

                if (!GetChildrenRef().empty()) {
                    SR_ERROR("SceneObject::Load() : prefab not loaded, but children are present! Path: {}", prefabPath);
                    DestroyChildren();
                }

                m_isPrefabLoadingState = true;

                if (!pPrefab->LoadToSO(this)) {
                    SR_ERROR("SceneObject::Load() : failed to apply prefab! Path: {}", prefabPath);
                }
                else {
                    SetPrefab(pPrefab, true);
                }

                m_isPrefabLoadingState = false;
            }
            else {
                SR_ERROR("SceneObject::Load() : failed to load prefab! Path: {}", prefabPath);
            }
        }

        return true;
    }

    bool SceneObject::MoveToTree(const SceneObject::Ptr& pDestination) {
        if (m_parent) {
            if (const SceneObject::Ptr pCopy = m_parent) {
                pCopy->RemoveChild(this);
            }
        }

        if (pDestination) {
            return pDestination->AddChild(this);
        }
        else {
            /// Если объект был выключен, нужно просигналить сцене чтобы она его включила.
            SetDirty(true);
        }

        if (GetParent()) {
            SRHalt("SceneObject::MoveToTree() : SO has parent!");
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

    void SceneObject::SetPrefab(const Prefab::Ptr& pPrefab, bool isOwner) { /// NOLINT (recursion)
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

        ForEachComponent([](const Component::Ptr& pComponent) -> bool {
            pComponent->OnBeforeLayerChanged();
            return true;
        });

        m_cachedLayer = m_layer = layer;

        ForEachComponent([](const Component::Ptr& pComponent) -> bool {
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

    Path SceneObject::GetPrefabPath() const {
        return m_prefabInfo.pPrefab ? m_prefabInfo.pPrefab->GetResourcePath() : Path();
    }

    bool SceneObject::PostLoad(bool force) {
        SR_TRACY_ZONE;

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

        SR_TRACY_ZONE;

        const bool isActivePrev = m_isActive;
        m_isActive = IsEnabled() && (!m_parent || m_parent->m_isActive);

        /// нет смысла продолжать цепочку, все и так выключено
        if (!m_isActive && m_isActive == isActivePrev) {
            return;
        }

        if (m_isActive != isActivePrev) {
            if (m_isActive) {
                OnEnabled();
            }
            else {
                OnDisabled();
            }
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

        SR_TRACY_ZONE;

        IComponentable::Awake(force, isPaused);

        for (auto&& pChild : m_children) {
            pChild->Awake(true, isPaused);
        }
    }

    void SceneObject::Start(bool force) noexcept { /// NOLINT (recursion)
        if (!force && !IsDirty()) {
            return;
        }

        SR_TRACY_ZONE;

        IComponentable::Start(force);

        for (auto&& pChild : m_children) {
            pChild->Start(true);
        }
    }

    void SceneObject::OnParentTagChanged() { /// NOLINT (recursion)
        if (m_tag != TagManager::Instance().GetDefaultTag()) {
            return;
        }

        if (!m_parent) {
            return; /// находимся в состоянии загрузки объекта
        }

        if (m_cachedTag == m_parent->m_cachedTag) {
            return;
        }

        m_cachedTag = m_parent->m_cachedTag;

        for (auto&& pChild : m_children) {
            pChild->OnParentTagChanged();
        }
    }

    void SceneObject::OnParentLayerChanged() { /// NOLINT (recursion)
        if (m_layer != LayerManager::GetDefaultLayer()) {
            return;
        }

        if (!m_parent) {
            return; /// находимся в состоянии загрузки объекта
        }

        SRAssert(!m_layer.Empty());

        if (m_cachedLayer == m_parent->m_cachedLayer) {
            return;
        }

        ForEachComponent([](const Component::Ptr& pComponent) -> bool {
            pComponent->OnBeforeLayerChanged();
            return true;
        });

        m_cachedLayer = m_parent->m_cachedLayer;

        ForEachComponent([](const Component::Ptr& pComponent) -> bool {
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

        if (GetPrefab()) {
            UnlinkPrefab();
        }

        if (auto&& pParent = GetParent()) {
            pParent->RemoveChild(this);
        }

        UnregisterEntity();

        if (m_scene) {
            m_scene->Remove(DynamicPointerCast<SceneObject>(GetThis()));
            while (!m_children.empty()) {
                if (const auto pParent = (*m_children.begin())->GetParent().Get(); pParent != this) {
                    SRHalt("SceneObject::Destroy() : child has different parent!");
                    m_children.erase(m_children.begin());
                    continue;
                }
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

            RemoveComponents();
            DestroyImpl();
        }
    }

    void SceneObject::DestroyImpl() {
        /// это должно быть единственное место,
        /// где мы уничтожаем объект
        GetThis().AutoFree();
    }

    void SceneObject::OnPostLoad() {
        for (auto&& pChild : m_children) {
            pChild->SetParent(StaticPointerCast<SceneObject>(GetThis()));
        }
        Super::OnPostLoad();
    }

    void SceneObject::UpdateRoot() {
        SR_TRACY_ZONE;
        m_root = m_parent ? m_parent->m_root : this;
        for (auto&& pChild : m_children) {
            pChild->UpdateRoot();
        }
    }

    void SceneObject::SetTag(SR_UTILS_NS::StringAtom tag) {
        SRAssert(!tag.Empty());

        if (TagManager::GetDefaultTag() == tag && m_parent) {
            tag = m_parent->GetTag();
        }

        if (tag == m_tag && m_cachedTag == m_tag) {
            return;
        }

        m_cachedTag = m_tag = tag;

        for (auto&& pChild : m_children) {
            pChild->OnParentTagChanged();
        }
    }

    SR_UTILS_NS::EntityIdList SceneObject::GetEntityIdList() const {
        SR_UTILS_NS::EntityIdList list(m_scene->GetEntityController());

        list.Add(GetEntityId());

        for (auto&& pComponent : m_components) {
            list.Add(pComponent->GetEntityId());
        }

        for (auto&& pChild : m_children) {
            list.Add(pChild->GetEntityId());
        }

        return list;
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

        if (!pChild->IsEntityRegistered() && m_scene) {
            m_scene->RegisterSceneObject(pChild);
        }

        if (!pChild->SetParent(DynamicPointerCast<SceneObject>(GetThis()))) {
            SR_WARN("SceneObject::AddChild() : failed to set parent!");
            return false;
        }

        m_children.emplace_back(pChild);

        pChild->OnParentLayerChanged();
        pChild->OnParentTagChanged();
        pChild->OnAttachedToParent();

        if (m_scene) {
            m_scene->OnChanged();
        }

        if (!IsActive()) {
            /// Если объект был выключен, то сцена не будет вызывать CheckActivity(), так как родитель уже выключен
            /// Значит, нужно вручную вызвать CheckActivity() для дочернего объекта и его дерева
            pChild->SetDirty(true);
            pChild->CheckActivity(false);
        }
        else {
            /// В случае если не активны, то ребенок сам позвонит в SetDirty() и поднимет флаг вверх
            /// А в данном кейсе чтобы сцена знала, что объект изменился, и если у нас дочерний объект был выключен,
            /// то сцена его включит
            SetDirty(true);
        }

        return true;
    }

    void SceneObject::SetName(const ObjectNameT name) {
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

        for (auto&& pChild : m_children) {
            pChild->SetScene(pScene);
        }
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
        if (pParent == m_parent) {
            return false;
        }

        SR_TRACY_ZONE;

        const SceneObject::Ptr pOldParent = m_parent;
        m_parent = pParent;

        OnParentChanged(pOldParent);

        UpdateRoot();

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

        pChild->SetParent(nullptr);

        for (uint16_t i = 0; i < m_children.size(); ++i) {
            if (pChild == m_children[i]) {
                m_children.erase(m_children.begin() + i);
                return;
            }
        }

        SRHalt("SceneObject {} is not child for {}!", pChild->GetName().c_str(), GetName().c_str());
    }

    void SceneObject::RemoveChildren() {
        SR_TRACY_ZONE;
        while (!m_children.empty()) {
            auto&& pChild = *m_children.begin();
            if (pChild) {
                RemoveChild(pChild);
            }
            else {
                SRHalt("SceneObject::RemoveChildren() : child is nullptr!");
                m_children.erase(m_children.begin());
            }
        }
    }

    void SceneObject::DestroyChildren() {
        SR_TRACY_ZONE;
        while (!m_children.empty()) {
            auto&& pChild = *m_children.begin();
            if (pChild) {
                pChild->Destroy();
            }
            else {
                SRHalt("SceneObject::DestroyChildren() : child is nullptr!");
                m_children.erase(m_children.begin());
            }
        }
    }

    void SceneObject::VerifyAfterLoad(SerializableVerifyContext& context) const noexcept {
        /*for (auto&& pChild : m_children) {
            if (!pChild) {
                context.AddError(SR_FORMAT("Child is nullptr on \"{}\"", GetName().ToStringView()));
            }
            else {
                pChild->VerifyAfterLoad(context);
            }
        }*/
        return Super::VerifyAfterLoad(context);
    }

    void SceneObject::ForEachChild(const std::function<void(SceneObject::Ptr&)>& fn) {
        SR_TRACY_ZONE;
        for (auto&& child : m_children) {
            if (child.Valid()) {
                fn(child);
            }
        }
    }

    void SceneObject::ForEachChild(const std::function<void(const SceneObject::Ptr&)>& fn) const {
        for (auto&& child : m_children) {
            if (child) {
                fn(child);
            }
        }
    }

    int32_t SceneObject::GetChildIndex(const SceneObject& child) const {
        for (int32_t i = 0; i < m_children.size(); ++i) {
            if (m_children[i] == child.GetThis()) {
                return i;
            }
        }
        return SR_ID_INVALID;
    }

    void SceneObject::OnRootRegistered() {
        SR_TRACY_ZONE;

        for (auto&& pChild : m_children) {
            pChild->OnParentLayerChanged();
        }

        for (auto&& pChild : m_children) {
            pChild->OnParentTagChanged();
        }
    }

    void SceneObject::OnMaskDirty() {
        for (auto&& pComponent : m_components) {
            pComponent->OnMaskDirty();
        }
    }

    void SceneObject::CloneTo(SRClass& clone) const {
        Super::CloneTo(clone);

        SceneObject& sceneObjectClone = static_cast<SceneObject&>(clone);
        sceneObjectClone.m_prefabInfo.isOwner = m_prefabInfo.isOwner;
        sceneObjectClone.m_prefabInfo.pPrefab = m_prefabInfo.pPrefab;
        if (sceneObjectClone.m_prefabInfo.pPrefab) {
            sceneObjectClone.m_prefabInfo.pPrefab->AddUsePoint();
        }

        for (auto&& pChild : sceneObjectClone.m_children) {
            pChild->SetParent(StaticPointerCast<SceneObject>(sceneObjectClone.GetThis()));
        }
    }

    SceneObject::Ptr SceneObject::FindRecursively(StringAtom name) const noexcept {
        SR_TRACY_ZONE;

        for (auto&& pChild : m_children) {
            if (pChild->GetName() == name) {
                return pChild;
            }

            if (const auto pFound = pChild->FindRecursively(name)) {
                return pFound;
            }
        }

        return SceneObject::Ptr();
    }
}
