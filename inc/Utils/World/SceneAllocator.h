//
// Created by Monika on 23.08.2022.
//

#ifndef SR_ENGINE_SCENE_ALLOCATOR_H
#define SR_ENGINE_SCENE_ALLOCATOR_H

#include <Utils/Types/SharedPtr.h>

namespace SR_WORLD_NS {
    class Scene;

    class SR_DLL_EXPORT SceneAllocator : public SR_UTILS_NS::Singleton<SceneAllocator> {
        SR_REGISTER_SINGLETON(SceneAllocator)
        using ScenePtr = SR_HTYPES_NS::SharedPtr<Scene>;
        typedef std::function<ScenePtr(void)> Allocator;

    protected:
        ~SceneAllocator() override = default;

    public:
        bool Init(const Allocator& allocator);
        SR_NODISCARD ScenePtr Allocate() const;

    private:
        Allocator m_allocator;

    };
}

#endif //SR_ENGINE_SCENE_ALLOCATOR_H
