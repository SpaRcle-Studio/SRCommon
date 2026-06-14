//
// Created by Monika on 29.01.2026.
//

#ifndef SR_ENGINE_COMMON_GAME_INSTANTIATE_COMPONENT_H
#define SR_ENGINE_COMMON_GAME_INSTANTIATE_COMPONENT_H

#include <Utils/ECS/Component.h>
#include <Utils/ECS/EntityRef.h>
#include <Utils/ECS/SceneObject.h>
#include <Utils/Types/Optional.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(InstantiateComponentInstanceMode, uint8_t,
        Clone,
        Prefab
    );

    SR_ENUM_NS_CLASS_T(InstantiateComponentActivateMode, uint8_t,
        OnStart,
        OnEnable,
        Manual
    );

    /// @displayName(Instantiate) @category(Game)
    class InstantiateComponent : public Component {
        SR_CLASS()
        using Super = Component;
    public:
        void DoInstantiate();

        void Start() override;
        void OnEnable() override;

    private:
        /// @property
        InstantiateComponentInstanceMode m_instanceMode = InstantiateComponentInstanceMode::Clone;
        /// @property
        InstantiateComponentActivateMode m_activateMode = InstantiateComponentActivateMode::OnStart;

        /// @property
        uint32_t m_count = 1;
        /// @property
        SR_HTYPES_NS::Optional<bool> m_activityOnInstantiate;
        /// @property
        SR_HTYPES_NS::Optional<SR_UTILS_NS::StringAtom> m_instanceName;
        /// @property
        SR_UTILS_NS::EntityRef<SR_UTILS_NS::SceneObject> m_target;

        /// @property @condition(This.m_instanceMode == SR_UTILS_NS::InstantiateComponentInstanceMode::Clone)
        SR_UTILS_NS::EntityRef<SR_UTILS_NS::SceneObject> m_source;
        /// @property @condition(This.m_instanceMode == SR_UTILS_NS::InstantiateComponentInstanceMode::Prefab)
        SR_UTILS_NS::StringAtom m_prefabId;

    };
}

#endif //SR_ENGINE_COMMON_GAME_INSTANTIATE_COMPONENT_H
