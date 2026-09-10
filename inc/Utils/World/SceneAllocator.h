//
// Created by Monika on 23.08.2022.
//

#ifndef SR_ENGINE_SCENE_ALLOCATOR_H
#define SR_ENGINE_SCENE_ALLOCATOR_H

#include <Utils/Types/SharedPtr.h>

namespace SR_WORLD_NS {
    class Scene;

    class SR_COMMON_DLL_API SceneAllocator : public SR_UTILS_NS::Singleton<SceneAllocator> {
        SR_REGISTER_SINGLETON(SceneAllocator)
        using ScenePtr = SR_HTYPES_NS::SharedPtr<Scene>;
        using AllocatorFn = SR_HTYPES_NS::Function<ScenePtr(void)>;
        using AddSceneToQueueFn = SR_HTYPES_NS::Function<void(const ScenePtr&)>;
    protected:
        ~SceneAllocator() override = default;

    public:
        void AddSceneToQueue(const ScenePtr& pScene);
        bool Init(const AllocatorFn& allocator, const AddSceneToQueueFn& addSceneToQueue);
        SR_NODISCARD ScenePtr Allocate() const;

    private:
        AllocatorFn m_allocator;
        AddSceneToQueueFn m_addSceneToQueue;

    };
}

#endif //SR_ENGINE_SCENE_ALLOCATOR_H
