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
#include <Utils/Serialization/SRASerialization.h>

#include <Codegen/Component.generated.hpp>

namespace SR_UTILS_NS {
    Component::~Component() {
        /// если срабатывает ассерт, значит, вероятнее всего, какой-то игровой объект до сих пор удерживает компонент,
        /// а значит, будет падение.
        SRAssert(!HasParent());
    }

    void Component::SetParent(IComponentable* pParent) {
        SR_TRACY_ZONE;

        if ((m_parent = pParent)) {
            if (auto&& pSceneObject = dynamic_cast<SceneObject*>(m_parent)) {
                m_sceneObject = pSceneObject;
                m_scene = m_sceneObject->GetScene();
                return;
            }
            else if (auto&& pScene = dynamic_cast<SR_WORLD_NS::Scene*>(m_parent)) {
                m_sceneObject.Reset();
                m_scene = pScene;
                return;
            }
        }

        m_sceneObject.Reset();
        m_scene = nullptr;
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

    const Component::SceneObjectPtr& Component::GetSceneObject() const {
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
            return const_cast<Transform*>(m_sceneObject.StaticCast<GameObject>()->GetTransform().Get());
        }

        return nullptr;
    }

    int32_t Component::GetIndexInSceneUpdater() const noexcept { return m_indexInSceneUpdater; }

    bool Component::HasParent() const { return m_parent; }

    Component::Ptr Component::CloneComponent() const {
        SR_TRACY_ZONE;

        SR_UTILS_NS::SRASerializer serializer;
        Save(serializer);

        auto&& pDeserializer = serializer.CreateDeserializer();

        auto&& pMeta = GetMeta();

        Component::Ptr pComponent = SR_UTILS_NS::Factory::Instance().Create<Component>(pMeta->GetFactoryName());
        if (!pComponent) {
            SR_ERROR("Component::CloneComponent() : failed to create component of type: {}", pMeta->GetFactoryName());
            return nullptr;
        }

        pComponent->Load(*pDeserializer);
        return pComponent;
    }

    bool Component::IsUpdatable() const noexcept {
        return m_isStarted && m_isActive;
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

    void Component::OnMatrixDirty() { }
    void Component::OnPriorityChanged() { }

    void Component::OnLoaded() { m_isComponentLoaded = true; }
    void Component::OnAttached() { m_isAttached = true; SRAssert(GetParent()); }
    void Component::OnDestroy() { SetParent(nullptr); }

    void Component::Awake() { m_isAwake = true; }
    void Component::Update(float_t dt) { }
    void Component::FixedUpdate() { }
    void Component::LateUpdate() { }

    void Component::OnBeforeLayerChanged() { }
    void Component::OnLayerChanged() { }

    void Component::Start() {
        m_isStarted = true;
    }

    void Component::OnDetached() {
        if (m_indexInSceneUpdater != SR_ID_INVALID) {
            m_scene->GetSceneUpdater()->UnRegisterComponent(this);
        }
        m_isAttached = false;
    }

    void Component::OnCollisionEnter(const CollisionData& data) { }
    void Component::OnCollisionStay(const CollisionData& data) { }
    void Component::OnCollisionExit(const CollisionData& data) { }
    void Component::OnTriggerEnter(const CollisionData& data) { }
    void Component::OnTriggerStay(const CollisionData& data) { }
    void Component::OnTriggerExit(const CollisionData& data) { }

    void Component::SetIndexIdSceneUpdater(int32_t index) {
        m_indexInSceneUpdater = index;
    }

    SR_NODISCARD bool Component::IsComponentLoaded() const noexcept { return m_isComponentLoaded; }
    SR_NODISCARD bool Component::IsComponentValid() const noexcept { return m_parent; }
    SR_NODISCARD bool Component::IsAttached() const noexcept { return m_isAttached; }

    SR_NODISCARD bool Component::IsActive() const noexcept { return m_isActive; }
    SR_NODISCARD bool Component::IsEnabled() const noexcept { return m_isEnabled; }

    SR_NODISCARD bool Component::IsAwake() const noexcept { return m_isAwake; }
    SR_NODISCARD bool Component::IsStarted() const noexcept { return m_isStarted; }

    bool Component::ExecuteInEditMode() const {
        return false;
    }

    Math::FVector3 Component::GetBarycenter() const {
        return SR_MATH_NS::InfinityFV3;
    }

    Component *Component::BaseComponent() noexcept {
        return this;
    }

    IComponentable *Component::TryGetParent() const {
        return m_parent;
    }

    bool Component::HasScene() const {
        return TryGetScene();
    }

    Component::Component() = default;
}

