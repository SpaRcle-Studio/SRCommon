//
// Created by Monika on 27.05.2022.
//

#ifndef SR_ENGINE_COMPONENT_MANAGER_H
#define SR_ENGINE_COMPONENT_MANAGER_H

#include <Utils/ECS/EntityController.h>
#include <Utils/ECS/Component.h>
#include <Utils/Types/Thread.h>
#include <Utils/Types/Function.h>
#include <Utils/Types/DataStorage.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Math/Vector3.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Serialization/SRASerialization.h>

namespace SR_UTILS_NS {
    class SR_DLL_EXPORT ComponentManager : public Singleton<ComponentManager> {
        SR_REGISTER_SINGLETON(ComponentManager)
    public:
        SR_NODISCARD bool IsSingletonCanBeDestroyed() const override {
            return false;
        }
    };
}

#endif //SR_ENGINE_COMPONENTMANAGER_H
