//
// Created by Monika on 16.04.2023.
//

#include <Utils/ECS/EntityRefUtils.h>
#include <Utils/World/Scene.h>
#include <Utils/ECS/Entity.h>
#include <Utils/ECS/Component.h>
#include <Utils/ECS/GameObject.h>

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
                    SceneObject::Ptr pObject = pEntity.DynamicCast<SceneObject>();

                    if (!pObject && owner.pEntity) {
                        if (auto&& pComponent = owner.pEntity.DynamicCast<Component>()) {
                            if (!pComponent->IsComponentValid()) {
                                return nullptr;
                            }
                            pObject = pComponent->GetSceneObject();
                        }
                        else if (auto&& pOwnerObject = owner.pEntity.DynamicCast<SceneObject>()) {
                            pObject = pOwnerObject->GetParent();
                        }
                    }

                    if (pObject) {
                        pEntity = pObject->GetParent().DynamicCast<Entity>();
                    }

                    break;
                }
                case Action::Action_Child: {
                    const std::vector<SR_UTILS_NS::SceneObject::Ptr>* tree = nullptr;

                    if (!pEntity) {
                        if (owner.pEntity) {
                            if (auto&& pSceneObject = owner.pEntity.DynamicCast<SceneObject>()) {
                                tree = &pSceneObject->GetChildrenRef();
                            }
                            //else if (auto&& pComponent = owner.pEntity.DynamicCast<Component>()) {
                            //    tree = &pComponent->GetScene()->GetRootSceneObjects();
                            //}
                            else if (auto&& pComponent = owner.pEntity.DynamicCast<Component>()) {
                                if (auto&& pObject = pComponent->GetSceneObject()) {
                                    tree = &pObject->GetChildrenRef();
                                }
                            }
                        }
                        else if (owner.pScene) {
                            tree = &owner.pScene->GetRootSceneObjects();
                        }
                    }
                    else if (auto&& pObject = pEntity.DynamicCast<SceneObject>()) {
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
                        pEntity = pChild.DynamicCast<Entity>();
                        found = true;
                    }

                    if (found) {
                        break;
                    }

                    return nullptr;
                }
                case Action::Action_Component: {
                    const std::vector<Component*>& components = pEntity ? pEntity.DynamicCast<SceneObject>()->GetComponents() : GetSceneFromOwner(owner)->GetComponents();

                    if (components.empty()) {
                        return nullptr;
                    }

                    uint16_t index = item.index;
                    for (auto&& pComponent : components) {
                        SRAssert1Once(pComponent->Valid());

                        if (pComponent->GetComponentName() != item.name) {
                            continue;
                        }
                        if (index > 0) {
                            --index;
                            continue;
                        }
                        pEntity = pComponent->DynamicCast<Entity>();
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
            if (auto&& pComponent = pFromEntity.DynamicCast<Component>()) {
                if (auto&& pParent = pComponent->GetParent()) {
                    uint16_t componentIndex = 0;

                    for (auto&& pComponentIteration : pParent->GetComponents()) {
                        if (pComponent == pComponentIteration) {
                            break;
                        }
                        if (pComponentIteration->GetComponentName() == pComponent->GetComponentName()) {
                            ++componentIndex;
                        }
                    }

                    PathItem item = {
                        .name = pComponent->GetComponentName(),
                        .index = componentIndex,
                        .action = Action::Action_Component
                    };

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
            else if (auto&& pObject = pFromEntity.DynamicCast<SceneObject>()) {
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

                PathItem item = {
                    .name = pObject->GetName(),
                    .index = objectIndex,
                    .action = Action::Action_Child
                };

                refPath.emplace_back(item);
                pFromEntity = pParent.DynamicCast<Entity>();
            }
        }

        std::reverse(refPath.begin(), refPath.end());

        return std::move(refPath);
    }

    RefPath CalculateRelativePath(const OwnerRef& from, const OwnerRef& target) {
        RefPath refPath;

        auto&& fromPath = CalculatePath(from);
        auto&& targetPath = CalculatePath(target);

        /**
         * File-System example:
         *      from: C:/A/B/C/file.txt
         *      to:   C:/A/G/C/file.txt
         *      path: ../../../G/C/file.txt
        */

        const int32_t minSize = SR_MIN(fromPath.size(), targetPath.size());
        int32_t offset = 0;

        for (int32_t i = 0; i < minSize; ++i) {
            if (fromPath[i] != targetPath[i]) {
                break;
            }
            ++offset;
        }

        /// Вычитаем 2, так как один это конец массива, а второй это компонент
        for (int32_t i = fromPath.size() - 2; i >= offset; --i) {
            PathItem item;
            item.action = Action::Action_Parent;
            refPath.emplace_back(item);
        }

        for (int32_t i = offset; i < targetPath.size(); ++i) {
            refPath.emplace_back(targetPath[i]);
        }

        return refPath;
    }

    bool IsOwnerValid(const OwnerRef& owner) {
        return owner.pScene || owner.pEntity;
    }

    bool IsTargetInitialized(const OwnerRef& owner) {
        if (owner.pScene) {
            return true;
        }

        if (auto&& pComponent = owner.pEntity.DynamicCast<Component>()) {
            return pComponent->HasParent();
        }

        if (auto&& pObject = owner.pEntity.DynamicCast<SceneObject>()) {
            return pObject->GetScene();
        }

        return false;
    }

    SR_WORLD_NS::Scene::Ptr GetSceneFromOwner(const OwnerRef& owner) {
        if (owner.pEntity) {
            SR_UTILS_NS::Entity::Ptr pEntity = owner.pEntity;

            if (auto&& pComponent = pEntity.DynamicCast<Component>()) {
                if (!pComponent->IsComponentValid()) {
                    return SR_WORLD_NS::Scene::Ptr();
                }
                return pComponent->GetSceneObject()->GetScene();
            }

            if (auto&& pObject = pEntity.DynamicCast<SceneObject>()) {
                return pObject->GetScene();
            }
        }

        return owner.pScene;
    }
}
