//
// Created by Nikita on 27.11.2020.
//

#include <Utils/ECS/Transform.h>
#include <Utils/ECS/Transform3D.h>
#include <Utils/ECS/TransformZero.h>
#include <Utils/Profile/TracyContext.h>

#include <Codegen/Transform.generated.hpp>

namespace SR_UTILS_NS {
    Transform::~Transform() {
        m_gameObject = nullptr;
    }

    void Transform::SetGameObject(GameObject* gameObject) {
        if ((m_gameObject = gameObject)) {
            OnHierarchyChanged();
        }
    }

    Transform* Transform::GetParentTransform() const {
        if (!m_gameObject) {
            return nullptr;
        }

        if (auto&& pParent = m_gameObject->GetParent()) {
            if (auto&& pGameObject = pParent.DynamicCast<GameObject>()) {
                return pGameObject->GetTransform();
            }
        }

        return nullptr;
    }

    SR_NODISCARD GameObject::Ptr Transform::GetGameObject() const {
        return m_gameObject->GetThis().DynamicCast<GameObject>();
    }

    void Transform::GlobalTranslate(const Math::FVector3& translation) {
        SetTranslation(GetTranslation() + translation);
    }

    void Transform::GlobalRotate(const Math::FVector3& eulers) {
        SetRotation(GetRotation() + eulers);
    }

    void Transform::GlobalScale(const Math::FVector3 &scale) {
        SetScale(GetScale() * scale);
    }

    void Transform::GlobalSkew(const Math::FVector3 &skew) {
        SetSkew(GetSkew() * skew);
    }

    void Transform::Rotate(Math::Unit x, Math::Unit y, Math::Unit z) {
        Rotate(Math::FVector3(x, y, z));
    }

    void Transform::GlobalRotate(Math::Unit x, Math::Unit y, Math::Unit z) {
        GlobalRotate(Math::FVector3(x, y, z));
    }

    void Transform::SetTranslation(Math::Unit x, Math::Unit y, Math::Unit z) {
        SetTranslation(Math::FVector3(x, y, z));
    }

    void Transform::SetRotation(Math::Unit yaw, Math::Unit pitch, Math::Unit roll) {
        SetRotation(Math::FVector3(yaw, pitch, roll));
    }

    void Transform::SetScale(Math::Unit x, Math::Unit y, Math::Unit z) {
        SetScale(Math::FVector3(x, y, z));
    }

    void Transform::SetSkew(Math::Unit x, Math::Unit y, Math::Unit z) {
        SetSkew(Math::FVector3(x, y, z));
    }

    void Transform::Translate(Math::Unit x, Math::Unit y, Math::Unit z) {
        Translate(Math::FVector3(x, y, z));
    }

    void Transform::Scale(Math::Unit x, Math::Unit y, Math::Unit z) {
        Scale(Math::FVector3(x, y, z));
    }

    SR_HTYPES_NS::Marshal::Ptr Transform::SaveLegacy(SavableContext data) const {
        auto&& pMarshal = Super::SaveLegacy(data);
        pMarshal->Write<uint16_t>(VERSION);
        pMarshal->Write(static_cast<uint8_t>(GetMeasurement()));

        switch (GetMeasurement()) {
            case Measurement::SpaceZero:
                break;
            case Measurement::Space2D: {
                auto&& pTransform2D = dynamic_cast<const SR_UTILS_NS::Transform2D*>(this);
                pMarshal->Write<uint8_t>(static_cast<uint8_t>(pTransform2D->GetStretch()));
                pMarshal->Write<uint8_t>(static_cast<uint8_t>(pTransform2D->GetAnchor()));
                pMarshal->Write<uint8_t>(static_cast<uint8_t>(pTransform2D->GetPositionMode()));
                pMarshal->Write<bool>(static_cast<bool>(pTransform2D->IsRelativePriority()));
                pMarshal->Write<int32_t>(static_cast<int32_t>(pTransform2D->GetLocalPriority()));
                pMarshal->Write(GetTranslation(), SR_MATH_NS::FVector3(0.f));
                pMarshal->Write(GetRotation(), SR_MATH_NS::FVector3(0.f));
                pMarshal->Write(GetScale(), SR_MATH_NS::FVector3(1.f));
                pMarshal->Write(GetSkew(), SR_MATH_NS::FVector3(1.f));
                break;
            }
            case Measurement::Space3D: {
                if (!m_gameObject || m_gameObject->GetParent()) {
                    pMarshal->Write(GetTranslation(), SR_MATH_NS::FVector3(0.f));
                }
                else {
                    auto &&offset = SR_THIS_THREAD->GetContext()->GetValueDef<SR_MATH_NS::FVector3>(SR_MATH_NS::FVector3());
                    pMarshal->Write(offset + GetTranslation(), SR_MATH_NS::FVector3(0.f));
                }

                pMarshal->Write(GetRotation(), SR_MATH_NS::FVector3(0.f));
                pMarshal->Write(GetScale(), SR_MATH_NS::FVector3(1.f));
                pMarshal->Write(GetSkew(), SR_MATH_NS::FVector3(1.f));
                break;
            }
            case Measurement::Space4D:
                break;
            case Measurement::Space1D:
                break;
            case Measurement::Holder:
                break;
            case Measurement::Unknown:
                break;
            default:
                SRHalt0();
                break;
        }

        return pMarshal;
    }

    Transform* Transform::Load(SR_HTYPES_NS::Marshal& marshal) {
        SR_TRACY_ZONE;

        Transform* pTransform = nullptr;

        SR_MAYBE_UNUSED auto&& version = marshal.Read<uint16_t>();
        if (version != VERSION) {
            SR_INFO("Transform::Load() : transform has different version! Trying to migrate from " +
                    SR_UTILS_NS::ToString(version) + " to " + SR_UTILS_NS::ToString(VERSION) + "..."
            );

            static const auto TRANSFORM_HASH_NAME = SR_HASH_STR("Transform");

            if (!Migration::Instance().Migrate(TRANSFORM_HASH_NAME, marshal, version, VERSION)) {
                SR_ERROR("Transform::Load() : failed to migrate transform!");
                return nullptr;
            }
        }

        auto&& measurement = static_cast<Measurement>(marshal.Read<uint8_t>());

        switch (measurement) {
            case Measurement::Holder:
                pTransform = new TransformHolder();
                break;
            case Measurement::SpaceZero:
                pTransform = new TransformZero();
                break;
            case Measurement::Space2D:
                pTransform = new Transform2D();
                break;
            case Measurement::Space3D:
                pTransform = new Transform3D();
                break;
            case Measurement::Space4D:
            default:
                SRHalt("Unknown measurement \"{}\"!", static_cast<int32_t>(measurement));
                return nullptr;
        }

        switch (measurement) {
            case Measurement::SpaceZero:
                break;
            case Measurement::Space2D: {
                auto&& pTransform2D = dynamic_cast<Transform2D*>(pTransform);
                pTransform2D->SetStretch(static_cast<Stretch>(marshal.Read<uint8_t>()));
                pTransform2D->SetAnchor(static_cast<Anchor>(marshal.Read<uint8_t>()));
                pTransform2D->SetPositionMode(static_cast<PositionMode>(marshal.Read<uint8_t>()));
                pTransform2D->SetRelativePriority(marshal.Read<bool>());
                pTransform2D->SetLocalPriority(marshal.Read<int32_t>());
                SR_FALLTHROUGH;
            }
            case Measurement::Space3D:
                pTransform->SetTranslation(marshal.Read<SR_MATH_NS::FVector3>(SR_MATH_NS::FVector3(0.f)));
                pTransform->SetRotation(marshal.Read<SR_MATH_NS::FVector3>(SR_MATH_NS::FVector3(0.f)));
                pTransform->SetScale(marshal.Read<SR_MATH_NS::FVector3>(SR_MATH_NS::FVector3(1.f)));
                pTransform->SetSkew(marshal.Read<SR_MATH_NS::FVector3>(SR_MATH_NS::FVector3(1.f)));
                break;
            case Measurement::Space4D:
            default:
                SRHalt0();
                return nullptr;
        }

        return pTransform;
    }

    SR_MATH_NS::FVector2 Transform::GetTranslation2D() const {
        return GetTranslation().XY();
    }

    SR_MATH_NS::FVector2 Transform::GetScale2D() const {
        return GetScale().XY();
    }

    const SR_MATH_NS::Matrix4x4& Transform::GetMatrix() const {
        static SR_MATH_NS::Matrix4x4 matrix4X4 = SR_MATH_NS::Matrix4x4::Identity();
        return matrix4X4;
    }

    void Transform::UpdateTree() {
        if (!m_gameObject) SR_UNLIKELY_ATTRIBUTE {
            return;
        }

        if (m_dirtyMatrix) {
            return;
        }
        m_dirtyMatrix = true;

        m_gameObject->OnMatrixDirty();

        for (auto&& pChild : m_gameObject->GetChildrenRef()) {
            if (auto&& pGameObject = pChild.DynamicCast<GameObject>()) {
                pGameObject->GetTransform()->UpdateTree();
            }
        }
    }

    bool Transform::IsDirty() const noexcept {
        return m_dirtyMatrix;
    }

    Transform *Transform::Copy() const {
        SRHalt("Not implemented!");
        return nullptr;
    }

    void Transform::OnHierarchyChanged() {
        UpdateTree();
    }

    SR_MATH_NS::FVector3 Transform::Right() const {
        return GetMatrix().GetQuat() * SR_MATH_NS::FVector3::Right();
    }

    SR_MATH_NS::FVector3 Transform::Up() const {
        return GetMatrix().GetQuat() * SR_MATH_NS::FVector3::Up();
    }

    SR_MATH_NS::FVector3 Transform::Forward() const {
        return GetMatrix().GetQuat() * SR_MATH_NS::FVector3::Forward();
    }

    void Transform::GlobalRotate(const SR_MATH_NS::Quaternion& quaternion) {
        SetRotation(GetQuaternion() * quaternion);
    }
}