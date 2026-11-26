//
// Created by Monika on 18.12.2022.
//

#ifndef SR_ENGINE_UTILS_SCENE_UPDATER_H
#define SR_ENGINE_UTILS_SCENE_UPDATER_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/Types/Time.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/FlatHashMap.h>
#include <Utils/UI/LayoutRebuilder.h>
#include <Utils/UI/UIInputRouter.h>

namespace SR_UTILS_NS {
    class Component;
    class SceneObject;
}

namespace SR_WORLD_NS {
    class Scene;

    class SR_COMMON_DLL_API SceneUpdater : public SR_UTILS_NS::NonCopyable {
        using Super = SR_UTILS_NS::NonCopyable;
    public:
        explicit SceneUpdater(Scene* pScene);

    public:
        void Build(bool isPaused);
        void Update(float_t dt, bool isPaused, bool updateLogic = true);
        void FixedUpdate(bool isPaused);
        void LateUpdate(bool isPaused, bool updateLogic = true);

        void SetDirty();
        void MarkLayoutDirty(SR_UTILS_NS::SceneObject& object, bool fromTransform = false);
        void RebuildDirtyLayouts();
        SR_NODISCARD uint64_t GetLayoutRebuildCount() const noexcept { return m_layoutRebuildCount; }

        void RegisterComponent(SR_UTILS_NS::Component* pComponent);
        void UnRegisterComponent(SR_UTILS_NS::Component* pComponent);

        SR_NODISCARD SR_UTILS_NS::TimePointType GetLastBuildTime() const { return m_lastBuildTimePoint; }
        /// Сцена не запущена или на паузе, обновляются только компоненты с ExecuteInEditMode
        SR_NODISCARD bool IsPaused() const noexcept { return m_isPaused; }
        SR_NODISCARD bool IsUIInputConsumed() const noexcept { return m_uiInputRouter.IsConsumed(); }
        void RequestUIFocus(SR_UTILS_NS::UIComponent& component) { m_uiInputRouter.RequestFocus(component); }
        void InvalidateUIInput(bool targetsDirty = false) { m_uiInputRouter.Invalidate(targetsDirty); }
        SR_NODISCARD uint64_t GetUIInputTargetsRevision() const noexcept { return m_uiInputRouter.GetTargetsRevision(); }
        SR_NODISCARD uint64_t GetUIInputRevision() const noexcept { return m_uiInputRouter.GetRevision(); }

    private:
        std::recursive_mutex m_mutex;
        Scene* m_scene = nullptr;

        SR_UTILS_NS::TimePointType m_lastBuildTimePoint;
        bool m_dirty = false;
        bool m_isPaused = false;
        SR_UTILS_NS::UIInputRouter m_uiInputRouter;

        uint32_t m_componentsPoolSize = 0;
        SR_UTILS_NS::Vector<SR_UTILS_NS::Component*> m_updatableComponents;
        SR_UTILS_NS::Vector<uint32_t> m_freeComponentIds;
        SR_UTILS_NS::Vector<SR_HTYPES_NS::SharedPtr<SR_UTILS_NS::SceneObject>> m_dirtyLayoutRoots;
        SR_UTILS_NS::Vector<SR_HTYPES_NS::SharedPtr<SR_UTILS_NS::SceneObject>> m_layoutRootsToRebuild;
        SR_HTYPES_NS::FlatHashSet<SR_UTILS_NS::SceneObject*> m_queuedLayoutRoots;
        SR_HTYPES_NS::FlatHashSet<SR_UTILS_NS::SceneObject*> m_layoutRootsInPass;
        SR_UTILS_NS::LayoutRebuilder::Context m_layoutContext;
        bool m_rebuildingLayout = false;
        uint64_t m_layoutRebuildCount = 0;

    };
}

#endif //SR_ENGINE_UTILS_SCENE_UPDATER_H
