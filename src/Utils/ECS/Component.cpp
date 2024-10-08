//
// Created by Nikita on 27.11.2020.
//

#include <Utils/ECS/Component.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/ECS/Transform2D.h>
#include <Utils/ECS/ComponentManager.h>
#include <Utils/Types/Thread.h>
#include <Utils/World/Scene.h>
#include <Utils/World/SceneUpdater.h>

namespace SR_UTILS_NS {
    Component::~Component() {
        /// если срабатывает ассерт, значит, вероятнее всего, какой-то игровой объект до сих пор удерживает компонент,
        /// а значит, будет падение.
        SRAssert(!HasParent());
        m_properties.ClearContainer();
    }

    SR_HTYPES_NS::Marshal::Ptr Component::SaveLegacy(SavableContext data) const {
        if (!(data.pMarshal = Entity::SaveLegacy(data))) {
            return data.pMarshal;
        }

        data.pMarshal->Write<uint64_t>(GetComponentName().GetHash());
        data.pMarshal->Write(IsEnabled());
        data.pMarshal->Write<uint16_t>(GetEntityVersion());

        if (!SR_UTILS_NS::ComponentManager::Instance().HasLoader(GetComponentName())) {
            GetComponentProperties().SaveProperty(*data.pMarshal);
        }

        return data.pMarshal;
    }

    void Component::SetParent(IComponentable* pParent) {
        if ((m_parent = pParent)) {
            if (auto&& pSceneObject = dynamic_cast<SR_UTILS_NS::SceneObject*>(m_parent)) {
                m_sceneObject = pSceneObject;
                m_scene = m_sceneObject->GetScene();
            }
            else {
                m_sceneObject.Reset();
                m_scene = dynamic_cast<SR_WORLD_NS::Scene*>(m_parent);
            }

            if (m_isComponentLoaded && !m_scene) {
                SRHalt("Missing scene!");
            }
        }
        else {
            m_sceneObject.Reset();
            m_scene = nullptr;
        }
    }

    void Component::SetEnabled(bool value) {
        if (m_isEnabled == value) {
            return;
        }

        m_isEnabled = value;

        if (m_parent) {
            m_parent->SetDirty(true);
        }
    }

    void Component::CheckActivity() {
        /// если родителя нет, или он отличается от ожидаемого, то будем считать, что родитель активен.
        /// сцена выключенной (в понимании игровых объектов) быть не может.
        const bool isActive = m_isEnabled && (!m_sceneObject || m_sceneObject->IsActive());
        if (isActive == m_isActive) {
            return;
        }

        if ((m_isActive = isActive)) {
            m_scene->GetSceneUpdater()->RegisterComponent(this);
            OnEnable();
        }
        else {
            if (m_indexInSceneUpdater != SR_ID_INVALID) {
                m_scene->GetSceneUpdater()->UnRegisterComponent(this);
            }
            OnDisable();
        }
    }

    Component::ScenePtr Component::GetScene() const {
        if (auto&& pScene = TryGetScene()) {
            return pScene;
        }

        SRHalt("The component have not a valid scene!");

        return nullptr;
    }

    Component::ScenePtr Component::TryGetScene() const {
        return m_scene;
    }

    Component::GameObjectPtr Component::GetGameObject() const {
        SRAssert(m_parent);
        return m_sceneObject.DynamicCast<GameObject>();
    }

    Component::SceneObjectPtr Component::GetSceneObject() const {
        SRAssert(m_parent);
        return m_sceneObject;
    }

    IComponentable* Component::GetParent() const {
        SRAssert(m_parent);
        return m_parent;
    }

    SceneObject::Ptr Component::GetRoot() const {
        SRAssert(m_parent);

        if (!m_sceneObject) {
            return SceneObject::Ptr();
        }

        SceneObject::Ptr pRoot = m_sceneObject;

        while (pRoot) {
            if (auto&& parent = pRoot->GetParent()) {
                pRoot = parent;
            }
            else {
                break;
            }
        }

        return pRoot;
    }

    Transform* Component::GetTransform() const noexcept {
        SRAssert(m_parent);

        if (m_sceneObject && m_sceneObject->GetSceneObjectType() == SceneObjectType::GameObject) {
            return m_sceneObject.StaticCast<GameObject>()->GetTransform();
        }

        return nullptr;
    }

    std::string Component::GetEntityInfo() const {
        return "Component: " + GetComponentName().ToStringRef();
    }

    bool Component::IsUpdatable() const noexcept {
        return m_isStarted && m_isActive;
    }

    Component* Component::CopyComponent() const {
        auto&& pComponent = SR_UTILS_NS::ComponentManager::Instance().CreateComponentOfName(GetComponentName());
        if (!pComponent) {
            return nullptr;
        }

        pComponent->SetEnabled(IsEnabled());

        /// TODO: non-optimized way
        SR_HTYPES_NS::Marshal marshal;
        GetComponentProperties().SaveProperty(marshal);
        marshal.SetPosition(0);
        pComponent->GetComponentProperties().LoadProperty(marshal);

        return pComponent;
    }

    bool Component::IsPlayingMode() const {
        if (auto&& pScene = TryGetScene()) {
            return pScene->IsPlayingMode();
        }

        return false;
    }

    bool Component::IsPausedMode() const {
        if (auto&& pScene = TryGetScene()) {
            return pScene->IsPausedMode();
        }

        return false;
    }

    void Component::Detach() {
        if (m_parent && IsAttached()) {
            m_parent->RemoveComponent(this);
        }
    }

    void Component::OnEnable() {

    }

    void Component::OnDisable() {

    }

    void Component::Start() {
        m_isStarted = true;
    }

    void Component::OnDetached() {
        if (m_indexInSceneUpdater != SR_ID_INVALID) {
            m_scene->GetSceneUpdater()->UnRegisterComponent(this);
        }
        m_isAttached = false;
    }
}

