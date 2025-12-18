//
// Created by Monika on 19.01.2025.
//

//#include <Utils/UI/UIModifier.h>
//
//#include <Utils/Common/Measurement.h>
//#include <Utils/ECS/Transform2D.h>
//
//#include <Codegen/UIModifier.generated.hpp>
//
//namespace SR_UTILS_NS::UI {
//    UIModifierComponent::~UIModifierComponent() {
//        SRAssert2(!m_connected, "UIModifierComponent is still connected!");
//    }
//
//    void UIModifierComponent::OnAttached() {
//        SRAssert2(!m_connected, "UIModifierComponent is already connected!");
//        ConnectModifier();
//        Super::OnAttached();
//    }
//
//    void UIModifierComponent::OnDetached() {
//        if (auto&& pParentTransform = GetTransform()) {
//            if (pParentTransform->GetMeasurement() == SR_UTILS_NS::Measurement::Space2D) {
//                static_cast<Transform2D*>(pParentTransform)->RemoveModifier(this);
//            }
//        }
//
//        m_connected = false;
//
//        Super::OnDetached();
//    }
//
//    void UIModifierComponent::OnDestroy() {
//        Super::OnDestroy();
//    }
//
//    void UIModifierComponent::SetUITreeDirty() {
//        SR_TRACY_ZONE;
//
//        if (auto&& pParentTransform = GetTransform()) {
//            if (pParentTransform->GetMeasurement() == SR_UTILS_NS::Measurement::Space2D) {
//                static_cast<Transform2D*>(pParentTransform)->OnUITreeChanged();
//            }
//        }
//    }
//
//    void UIModifierComponent::ConnectModifier() {
//        if (auto&& pParentTransform = GetTransform()) {
//            if (pParentTransform->GetMeasurement() == SR_UTILS_NS::Measurement::Space2D) {
//                static_cast<Transform2D*>(pParentTransform)->AddModifier(this);
//                m_connected = true;
//            }
//        }
//    }
//}
//