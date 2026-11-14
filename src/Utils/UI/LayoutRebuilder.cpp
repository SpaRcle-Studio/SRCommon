//
// Created by Codex on 15.11.2026.
//

#include <Utils/UI/LayoutRebuilder.h>
#include <Utils/UI/ILayoutElement.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/ECS/Component.h>
#include <Utils/World/Scene.h>
#include <Utils/World/SceneUpdater.h>

namespace SR_UTILS_NS::LayoutRebuilder {
    void MarkDirty(SceneObject& object, const bool fromTransform) {
        if (auto&& pScene = object.GetScenePtr()) {
            pScene->GetSceneUpdater()->MarkLayoutDirty(object, fromTransform);
        }
    }

    void Collect(SceneObject& object, Context& context) {
        if (!object.IsActive()) {
            return;
        }
        if (object.GetMeta() == GameObject::GetMetaStatic()) {
            auto&& gameObject = static_cast<GameObject&>(object);
            Node node;
            node.elementBegin = context.elements.size();
            node.selfBegin = context.selfControllers.size();
            node.controllerBegin = context.controllers.size();
            for (auto&& pComponent : gameObject.GetComponents()) {
                if (!pComponent->IsActive()) {
                    continue;
                }
                auto&& pMutableComponent = const_cast<Component*>(pComponent.Get());
                if (auto&& pElement = pMutableComponent->GetLayoutElement()) {
                    context.elements.emplace_back(pElement);
                }
                if (auto&& pController = pMutableComponent->GetLayoutController()) {
                    if (pController->IsSelfController()) {
                        context.selfControllers.emplace_back(pController);
                    }
                    else {
                        context.controllers.emplace_back(pController);
                    }
                }
            }
            node.elementCount = context.elements.size() - node.elementBegin;
            node.selfCount = context.selfControllers.size() - node.selfBegin;
            node.controllerCount = context.controllers.size() - node.controllerBegin;
            if ((node.selfCount > 1 && context.selfControllers[node.selfBegin]->MarkConflictReported())
                || (node.controllerCount > 1 && context.controllers[node.controllerBegin]->MarkConflictReported())) {
                SR_WARN("LayoutRebuilder::Collect() : multiple layout controllers; component order resolves the conflict.");
            }
            if (node.elementCount > 0 || node.selfCount > 0 || node.controllerCount > 0) {
                context.nodes.emplace_back(node);
            }
        }
        for (auto&& pChild : object.GetChildrenRef()) {
            Collect(*pChild, context);
        }
    }

    void Measure(Context& context, const bool horizontal) {
        for (uint64_t n = context.nodes.size(); n > 0; --n) {
            const auto node = context.nodes[n - 1];
            for (uint64_t i = 0; i < node.elementCount; ++i) {
                context.elements[node.elementBegin + i]->CalculateLayoutInput(horizontal);
            }
        }
    }

    void Arrange(Context& context, const bool horizontal) {
        for (auto&& node : context.nodes) {
            for (uint64_t i = 0; i < node.selfCount; ++i) {
                context.selfControllers[node.selfBegin + i]->SetLayout(horizontal);
            }
            for (uint64_t i = 0; i < node.controllerCount; ++i) {
                context.controllers[node.controllerBegin + i]->SetLayout(horizontal);
            }
        }
    }

    void Rebuild(SceneObject& root, Context& context) {
        SR_TRACY_ZONE;
        context.nodes.clear();
        context.elements.clear();
        context.selfControllers.clear();
        context.controllers.clear();
        Collect(root, context);
        Measure(context, true);
        Arrange(context, true);
        Measure(context, false);
        Arrange(context, false);
    }

    void Rebuild(SceneObject& root) {
        Context context;
        Rebuild(root, context);
    }
}
