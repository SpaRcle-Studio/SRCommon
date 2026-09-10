//
// Created by Monika on 23.08.2022.
//

#include <Utils/World/SceneAllocator.h>

namespace SR_WORLD_NS {
    bool SceneAllocator::Init(const SceneAllocator::AllocatorFn& allocator, const SceneAllocator::AddSceneToQueueFn& addSceneToQueue) {
        if (m_allocator || m_addSceneToQueue) {
            return false;
        }

        m_allocator = allocator;
        m_addSceneToQueue = addSceneToQueue;

        return true;
    }

    SceneAllocator::ScenePtr SceneAllocator::Allocate() const {
        if (!m_allocator) {
            SRHaltTerminate("SceneAllocator::Allocate() : allocator isn't initialized!");
        }
        return m_allocator();
    }

    void SceneAllocator::AddSceneToQueue(const SceneAllocator::ScenePtr &pScene) {
        if (!m_addSceneToQueue) {
            SRHaltTerminate("SceneAllocator::AddSceneToQueue() : add scene to queue function isn't initialized!");
        }
        m_addSceneToQueue(pScene);
    }
}