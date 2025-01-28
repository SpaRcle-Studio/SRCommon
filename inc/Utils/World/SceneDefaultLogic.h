//
// Created by Monika on 23.12.2022.
//

#ifndef SR_ENGINE_SCENEDEFAULTLOGIC_H
#define SR_ENGINE_SCENEDEFAULTLOGIC_H

#include <Utils/World/SceneLogic.h>

namespace SR_WORLD_NS {
    class SceneDefaultLogic : public SceneLogic {
        SR_CLASS()
        using Super = SceneLogic;
    public:
        SceneDefaultLogic() = default;
        explicit SceneDefaultLogic(const ScenePtr& scene);
        ~SceneDefaultLogic() override = default;

    public:
        SR_NODISCARD bool IsDefault() const noexcept override { return true; }

        bool Reload() override;
        void Destroy() override;
        void Update(float_t dt) override;

        bool Load(const Path& path) override;
        bool Save(const Path& path) override;

    };
}

#endif //SR_ENGINE_SCENEDEFAULTLOGIC_H
