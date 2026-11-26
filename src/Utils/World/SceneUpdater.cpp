//
// Created by Monika on 18.12.2022.
//

#include <Utils/World/SceneUpdater.h>
#include <Utils/World/Scene.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/ECS/Component.h>
#include <Utils/Input/InputSystem.h>
#include <Utils/Types/Function.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/UI/LayoutRebuilder.h>
#include <Utils/UI/UIInputAdapter.h>
#include <Utils/ECS/TransformRect.h>

namespace SR_WORLD_NS {
    SceneUpdater::SceneUpdater(Scene *pScene)
        : Super()
        , m_scene(pScene)
    {
        m_updatableComponents.reserve(1024);
    }

    void SceneUpdater::Build(bool isPaused) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        if (!m_dirty) {
            return;
        }

        m_dirty = false;

        m_lastBuildTimePoint = SR_HTYPES_NS::Time::Instance().Now();

        auto&& root = m_scene->GetRootSceneObjects();

        m_scene->PostLoad(false);
        m_scene->Awake(false, isPaused);
        m_scene->CheckActivity(false);
        m_scene->Start(false);

        for (auto&& pObject : root) {
            pObject->PostLoad(false);
        }

        for (auto&& pObject : root) {
            pObject->Awake(false, isPaused);
        }

        for (auto&& pObject : root) {
            pObject->CheckActivity(false);
        }

        for (auto&& pObject : root) {
            pObject->Start(false);
        }
    }

    void SceneUpdater::Update(float_t dt, bool isPaused, bool updateLogic) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        m_isPaused = isPaused;

        m_uiInputRouter.Prepare(isPaused);

        if (!isPaused) {
            RebuildDirtyLayouts();
            m_uiInputRouter.AfterLayout(false);
            m_uiInputRouter.Process(*m_scene, SR_UTILS_NS::PointerInput::Instance().GetEvents(), SR_UTILS_NS::UIInputAdapter::ReadKeyboard());
        }

        if (!updateLogic) {
            return;
        }

        for (uint32_t i = 0; i < m_componentsPoolSize; ++i) {
            auto&& pComponent = m_updatableComponents[i];
            if (!pComponent) {
                continue;
            }

            if (isPaused && !pComponent->ExecuteInEditMode()) {
                continue;
            }

            pComponent->Update(dt);
        }
    }

    void SceneUpdater::FixedUpdate(bool isPaused) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        for (uint32_t i = 0; i < m_componentsPoolSize; ++i) {
            auto&& pComponent = m_updatableComponents[i];
            if (!pComponent) {
                continue;
            }

            if (isPaused && !pComponent->ExecuteInEditMode()) {
                continue;
            }

            pComponent->FixedUpdate();
        }
    }

    void SceneUpdater::LateUpdate(bool isPaused, bool updateLogic) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        if (updateLogic) {
            for (uint32_t i = 0; i < m_componentsPoolSize; ++i) {
                auto&& pComponent = m_updatableComponents[i];
                if (!pComponent) {
                    continue;
                }

                if (isPaused && !pComponent->ExecuteInEditMode()) {
                    continue;
                }

                pComponent->LateUpdate();
            }
        }

        RebuildDirtyLayouts();
        m_uiInputRouter.AfterLayout(isPaused);
    }

    void SceneUpdater::MarkLayoutDirty(SR_UTILS_NS::SceneObject& object, const bool fromTransform) {
        SR_TRACY_ZONE;
        // Non-transform dirtiness can also change component or sibling traversal order.
        m_uiInputRouter.Invalidate(!fromTransform);
        if ((fromTransform && m_rebuildingLayout) || object.IsDestroyed()
            || object.GetMeta() != SR_UTILS_NS::GameObject::GetMetaStatic()) {
            return;
        }
        auto&& pObject = static_cast<SR_UTILS_NS::GameObject*>(&object);
        auto&& pTransform = pObject->TryGetTransform();
        if (!pTransform || !pTransform->GetMeta()->IsSameOrInherited(SR_UTILS_NS::TransformRect::GetClassStaticName())) {
            return;
        }

        SR_UTILS_NS::SceneObject* pRoot = pObject;
        while (auto pParent = pRoot->GetParent()) {
            if (pParent->GetMeta() != SR_UTILS_NS::GameObject::GetMetaStatic()) {
                break;
            }
            auto&& pParentObject = static_cast<SR_UTILS_NS::GameObject*>(pParent.Get());
            auto&& pParentTransform = pParentObject->TryGetTransform();
            if (!pParentTransform || !pParentTransform->GetMeta()->IsSameOrInherited(SR_UTILS_NS::TransformRect::GetClassStaticName())) {
                break;
            }
            pRoot = pParentObject;
        }
        if (m_queuedLayoutRoots.insert(pRoot).second) {
            m_dirtyLayoutRoots.emplace_back(pRoot);
        }
    }

    void SceneUpdater::RebuildDirtyLayouts() {
        if (m_rebuildingLayout || m_dirtyLayoutRoots.empty()) {
            return;
        }
        SR_TRACY_ZONE;
        m_dirtyLayoutRoots.swap(m_layoutRootsToRebuild);
        m_queuedLayoutRoots.clear();
        m_layoutRootsInPass.clear();
        for (auto&& pRoot : m_layoutRootsToRebuild) {
            if (pRoot && !pRoot->IsDestroyed() && pRoot->IsActive()) {
                m_layoutRootsInPass.insert(pRoot.Get());
            }
        }
        m_rebuildingLayout = true;
        for (auto&& pRoot : m_layoutRootsToRebuild) {
            if (pRoot && !pRoot->IsDestroyed() && pRoot->IsActive()) {
                bool coveredByAncestor = false;
                for (auto pParent = pRoot->GetParent(); pParent; pParent = pParent->GetParent()) {
                    if (m_layoutRootsInPass.contains(pParent.Get())) {
                        coveredByAncestor = true;
                        break;
                    }
                }
                if (coveredByAncestor) {
                    continue;
                }
                SR_UTILS_NS::LayoutRebuilder::Rebuild(*pRoot, m_layoutContext);
                ++m_layoutRebuildCount;
            }
        }
        m_rebuildingLayout = false;
        m_layoutRootsToRebuild.clear();
    }

    void SceneUpdater::SetDirty() {
        m_dirty = true;
        m_uiInputRouter.Invalidate(true);
    }

    void SceneUpdater::RegisterComponent(SR_UTILS_NS::Component* pComponent) {
        m_uiInputRouter.Register(*pComponent);
        if (pComponent->GetLayoutElement() || pComponent->GetLayoutController() || pComponent->GetLayoutIgnorer()) {
            pComponent->SetLayoutDirty();
        }
        SetDirty();

        SRAssert2(pComponent->GetIndexInSceneUpdater() == SR_ID_INVALID, "Double component registration!");

        if (m_freeComponentIds.empty()) {
            pComponent->SetIndexIdSceneUpdater(static_cast<int32_t>(m_componentsPoolSize));
            ++m_componentsPoolSize;
            m_updatableComponents.emplace_back(pComponent);
        }
        else {
            const auto index = static_cast<int32_t>(m_freeComponentIds.back());
            m_freeComponentIds.pop_back();
            pComponent->SetIndexIdSceneUpdater(index);
            m_updatableComponents[index] = pComponent;
        }
    }

    void SceneUpdater::UnRegisterComponent(SR_UTILS_NS::Component* pComponent) {
        m_uiInputRouter.Unregister(*pComponent);
        if (pComponent->GetLayoutElement() || pComponent->GetLayoutController() || pComponent->GetLayoutIgnorer()) {
            pComponent->SetLayoutDirty();
        }
        SetDirty();

        auto&& index = pComponent->GetIndexInSceneUpdater();

        if (static_cast<uint32_t>(index) >= m_componentsPoolSize) {
            SRHalt("Invalid component index!");
            return;
        }

        pComponent->SetIndexIdSceneUpdater(SR_ID_INVALID);
        m_freeComponentIds.emplace_back(index);
        m_updatableComponents[index] = nullptr;
    }
}
