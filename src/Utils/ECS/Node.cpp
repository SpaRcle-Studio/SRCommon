//
// Created by Monika on 17.05.2025.
//

#include <Utils/ECS/Node.h>

#include <Codegen/Node.generated.hpp>

namespace SR_UTILS_NS {
    Node::Node() = default;

    Node::~Node() = default;

    SR_UTILS_NS::SceneObjectType Node::GetSceneObjectType() const noexcept {
        return SR_UTILS_NS::SceneObjectType::Node;
    }

    void Node::OnHierarchyChanged() {
        Super::OnHierarchyChanged();
    }

    void Node::OnAttached() {

    }
}