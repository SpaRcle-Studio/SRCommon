//
// Created by Monika on 16.04.2023.
//

#include <Utils/ECS/EntityRefUtils.h>
#include <Utils/World/Scene.h>
#include <Utils/ECS/Entity.h>
#include <Utils/ECS/Component.h>
#include <Utils/ECS/GameObject.h>

#include <Enum/Action.hpp>

#include <Codegen/EntityRefUtils.generated.hpp>

namespace SR_UTILS_NS::EntityRefUtils {
    EntityRefUtils::OwnerRef::OwnerRef(const SR_HTYPES_NS::SharedPtr<Entity>& ptr)
        : pEntity(ptr)
    { }

    EntityRefUtils::OwnerRef::OwnerRef(const SR_HTYPES_NS::SharedPtr<SR_WORLD_NS::Scene>& ptr)
        : pScene(ptr)
    { }

    Entity::Ptr GetEntity(const OwnerRef& owner, const RefPath& path) {
        Entity::Ptr pEntity;

        for (const PathItem& item : path) {
            switch (item.action) {
                case Action::Action_Parent: {
                    SceneObject::Ptr pObject = DynamicPointerCast<SceneObject>(pEntity);

                    if (!pObject && owner.pEntity) {
                        if (auto&& pComponent = DynamicPointerCast<Component>(owner.pEntity)) {
                            if (!pComponent->IsComponentValid()) {
                                return nullptr;
                            }
                            pObject = pComponent->GetSceneObject();
                        }
                        else if (auto&& pOwnerObject = DynamicPointerCast<SceneObject>(owner.pEntity)) {
                            pObject = pOwnerObject->GetParent();
                        }
                    }

                    if (pObject) {
                        pEntity = DynamicPointerCast<Entity>(pObject->GetParent());
                    }

                    break;
                }
                case Action::Action_Child: {
                    const Vector<SR_UTILS_NS::SceneObject::Ptr>* tree = nullptr;

                    if (!pEntity) {
                        if (owner.pEntity) {
                            if (auto&& pSceneObject = DynamicPointerCast<SceneObject>(owner.pEntity)) {
                                tree = &pSceneObject->GetChildrenRef();
                            }
                            //else if (auto&& pComponent = owner.pEntity.DynamicCast<Component>()) {
                            //    tree = &pComponent->GetScene()->GetRootSceneObjects();
                            //}
                            else if (auto&& pComponent = DynamicPointerCast<Component>(owner.pEntity)) {
                                if (auto&& pObject = pComponent->GetSceneObject()) {
                                    tree = &pObject->GetChildrenRef();
                                }
                            }
                        }
                        else if (owner.pScene) {
                            tree = &owner.pScene->GetRootSceneObjects();
                        }
                    }
                    else if (auto&& pObject = DynamicPointerCast<SceneObject>(pEntity)) {
                        tree = &pObject->GetChildrenRef();
                    }

                    if (!tree) {
                        SRHaltOnce0();
                        return nullptr;
                    }

                    uint16_t index = item.index;
                    bool found = false;
                    for (auto&& pChild : *tree) {
                        if (pChild->GetName() != item.name) {
                            continue;
                        }
                        if (index > 0) {
                            --index;
                            continue;
                        }
                        pEntity = DynamicPointerCast<Entity>(pChild);
                        found = true;
                    }

                    if (found) {
                        break;
                    }

                    return nullptr;
                }
                case Action::Action_Component: {
                    const Vector<Component::Ptr>& components = pEntity ? DynamicPointerCast<SceneObject>(pEntity)->GetComponents() : GetSceneFromOwner(owner)->GetComponents();

                    if (components.empty()) {
                        return nullptr;
                    }

                    uint16_t index = item.index;
                    for (auto&& pComponent : components) {
                        SRAssert1Once(pComponent->Valid());

                        if (pComponent->GetMeta()->GetFactoryName() != item.name) {
                            continue;
                        }
                        if (index > 0) {
                            --index;
                            continue;
                        }
                        pEntity = DynamicPointerCast<Entity>(pComponent);
                        break;
                    }

                    break;
                }
                default:
                    SRHaltOnce0();
                    break;
            }
        }

        return pEntity;
    }

    RefPath CalculatePath(const OwnerRef& from) {
        RefPath refPath;

        if (from.pScene) {
            return std::move(refPath);
        }

        Entity::Ptr pFromEntity = from.pEntity;

        while (pFromEntity) {
            /// ---------------------- [ Component ] ----------------------
            if (auto&& pComponent = DynamicPointerCast<Component>(pFromEntity)) {
                if (auto&& pParent = pComponent->GetParent()) {
                    uint16_t componentIndex = 0;

                    for (auto&& pComponentIteration : pParent->GetComponents()) {
                        if (pComponent == pComponentIteration) {
                            break;
                        }
                        if (pComponentIteration->GetMeta()->GetFactoryName() == pComponent->GetMeta()->GetFactoryName()) {
                            ++componentIndex;
                        }
                    }

                    PathItem item(pComponent->GetMeta()->GetFactoryName(), componentIndex, Action::Action_Component);

                    refPath.emplace_back(item);

                    if (auto&& pObject = dynamic_cast<SceneObject*>(pParent)) {
                        pFromEntity = pObject;
                    }
                    else {
                        pFromEntity = nullptr;
                    }
                }
                else {
                    SRHalt0();
                }
            }
                /// --------------------- [ Game Object ] ---------------------
            else if (auto&& pObject = DynamicPointerCast<SceneObject>(pFromEntity)) {
                auto&& pParent = pObject->GetParent();

                auto&& tree = pParent ? pParent->GetChildrenRef() : pObject->GetScene()->GetRootSceneObjects();

                uint16_t objectIndex = 0;

                for (auto&& pChild : tree) {
                    if (pChild == pObject) {
                        break;
                    }
                    if (pChild->GetName() == pObject->GetName()) {
                        ++objectIndex;
                    }
                }

                PathItem item(pObject->GetName(), objectIndex, Action::Action_Child);

                refPath.emplace_back(item);
                pFromEntity = DynamicPointerCast<Entity>(pParent);
            }
        }

        std::reverse(refPath.begin(), refPath.end());

        return std::move(refPath);
    }

    bool IsOwnerValid(const OwnerRef& owner) {
        return owner.pScene || owner.pEntity;
    }

    bool IsTargetInitialized(const OwnerRef& owner) {
        if (owner.pScene) {
            return true;
        }

        if (auto&& pComponent = DynamicPointerCast<Component>(owner.pEntity)) {
            return pComponent->HasParent();
        }

        if (auto&& pObject = DynamicPointerCast<SceneObject>(owner.pEntity)) {
            return pObject->GetScene();
        }

        return false;
    }

    SR_WORLD_NS::Scene::Ptr GetSceneFromOwner(const OwnerRef& owner) {
        if (owner.pEntity) {
            SR_UTILS_NS::Entity::Ptr pEntity = owner.pEntity;

            if (auto&& pComponent = DynamicPointerCast<Component>(pEntity)) {
                if (!pComponent->IsComponentValid()) {
                    return SR_WORLD_NS::Scene::Ptr();
                }
                return pComponent->GetSceneObject()->GetScene();
            }

            if (auto&& pObject = DynamicPointerCast<SceneObject>(pEntity)) {
                return pObject->GetScene();
            }
        }

        return owner.pScene;
    }
}
