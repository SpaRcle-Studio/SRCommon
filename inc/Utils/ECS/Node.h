//
// Created by Monika on 17.05.2025.
//

#ifndef SR_ENGINE_ECS_NODE_H
#define SR_ENGINE_ECS_NODE_H

#include <Utils/ECS/SceneObject.h>
#include <Utils/Math/Matrix4x4.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(ECSNodeType, uint32_t,
        UINode,
        UIControlNode,
        UIContainerNode
    )

    class SR_COMMON_DLL_API Node : public SceneObject {
        SR_CLASS()
        using Super = SceneObject;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<Node>;

    public:
        Node();
        ~Node() override;

    public:
        SR_NODISCARD SR_UTILS_NS::SceneObjectType GetSceneObjectType() const noexcept override;

        SR_NODISCARD virtual ECSNodeType GetNodeType() const noexcept {
            SRHalt("Abstract method called!");
            return ECSNodeType::ECSNodeTypeMAX;
        }

        SR_NODISCARD virtual const SR_MATH_NS::Matrix4x4& GetMatrix() const noexcept {
            SRHalt("Abstract method called!");
            static const SR_MATH_NS::Matrix4x4 identity = SR_MATH_NS::Matrix4x4::Identity();
            return identity;
        }

        SR_NODISCARD virtual uint64_t GetNodePriority() const noexcept { return 0; }

    protected:
        void OnHierarchyChanged() override;

    };
}

#endif //SR_ENGINE_ECS_NODE_H
