//
// Created by Monika on 23.08.2022.
//

#include <Utils/World/SceneAllocator.h>

namespace SR_WORLD_NS {
    bool SceneAllocator::Init(const SceneAllocator::Allocator& allocator) {
        if (m_allocator) {
            return false;
        }

        m_allocator = allocator;

        return true;
    }

    SceneAllocator::ScenePtr SceneAllocator::Allocate() const {
        if (!m_allocator) {
            SRHaltTerminate("SceneAllocator::Allocate() : allocator isn't initialized!");
        }

        return m_allocator();
    }
}