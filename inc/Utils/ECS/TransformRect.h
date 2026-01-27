//
// Created by Monika on 01.08.2022.
//

#ifndef SR_ENGINE_UTILS_TRANSFORM_RECT_H
#define SR_ENGINE_UTILS_TRANSFORM_RECT_H

#include <Utils/ECS/Transform.h>
#include <Utils/UI/MaskInfo.h>

namespace SR_UTILS_NS {
    class GameObject;

    struct SR_COMMON_DLL_API RectAnchors : public SR_UTILS_NS::Serializable {
        SR_STRUCT()
    public:
        RectAnchors() = default;
        RectAnchors(const SR_MATH_NS::FVector2& min, const SR_MATH_NS::FVector2& max)
            : min(min)
            , max(max)
        { }

        /// @property @resetValue(SR_MATH_NS::FVector2(0.5f, 0.5f))
        SR_MATH_NS::FVector2 min = SR_MATH_NS::FVector2(0.5f, 0.5f);
        /// @property @resetValue(SR_MATH_NS::FVector2(0.5f, 0.5f))
        SR_MATH_NS::FVector2 max = SR_MATH_NS::FVector2(0.5f, 0.5f);

        bool operator==(const RectAnchors& other) const {
            return min == other.min && max == other.max;
        }

        bool operator!=(const RectAnchors& other) const {
            return !(*this == other);
        }

    };

    /// @inspector(TransformRectPropertyDrawer)
    class SR_COMMON_DLL_API TransformRect : public Transform {
        SR_CLASS()
        friend class GameObject;
        using Super = Transform;
    public:
        TransformRect();

    public:
        void SetTranslation(const SR_MATH_NS::FVector3& translation) override;
        void SetTranslationAndRotation(const SR_MATH_NS::FVector3& translation, const SR_MATH_NS::FVector3& euler) override;
        void SetRotation(const SR_MATH_NS::FVector3& euler) override;
        void SetRotation(const SR_MATH_NS::Quaternion& quaternion) override;
        void SetScale(const SR_MATH_NS::FVector3& scale) override;
        void SetSkew(const SR_MATH_NS::FVector3& skew) override;
        void SetSize(const SR_MATH_NS::FVector2& size);
        void SetPivot(const SR_MATH_NS::FVector2& pivot);
        void SetAnchors(const RectAnchors& anchors);
        void SetCanvasSize(const SR_MATH_NS::FVector2& size);
        void SetOffsetMin(const SR_MATH_NS::FVector2& offset);
        void SetOffsetMax(const SR_MATH_NS::FVector2& offset);

        void Translate(const SR_MATH_NS::FVector3& translation) override;
        void Rotate(const SR_MATH_NS::Quaternion& quaternion) override;
        void Scale(const SR_MATH_NS::FVector3& scale) override;

        void SetGlobalTranslation(const SR_MATH_NS::FVector3& translation) override;
        void SetGlobalRotation(const SR_MATH_NS::Quaternion& quaternion) override;

        void SetLocalPriority(int32_t priority);
        void SetRelativePriority(bool relative);

        SR_NODISCARD SR_MATH_NS::Quaternion GetGlobalRotation() const override;
        SR_NODISCARD SR_MATH_NS::Quaternion GetQuaternion() const override { return m_quaternion; }
        SR_NODISCARD SR_MATH_NS::FVector3 GetTranslation() const override { return m_translation; }
        SR_NODISCARD SR_MATH_NS::FVector3 GetRotation() const override { return m_rotation; }
        SR_NODISCARD SR_MATH_NS::FVector3 GetScale() const override { return m_scale; }
        SR_NODISCARD SR_MATH_NS::FVector3 GetSkew() const override { return m_skew; }
        SR_NODISCARD SR_MATH_NS::FVector2 GetSize() const { return m_size; }
        SR_NODISCARD SR_MATH_NS::FVector2 GetPivot() const { return m_pivot; }
        SR_NODISCARD const RectAnchors& GetAnchors() const { return m_anchors; }
        SR_NODISCARD SR_MATH_NS::FVector2 GetOffsetMin() const { return m_offsetMin; }
        SR_NODISCARD SR_MATH_NS::FVector2 GetOffsetMax() const { return m_offsetMax; }
        SR_NODISCARD SR_MATH_NS::FVector2 GetAnchoredPosition() const;
        SR_NODISCARD SR_MATH_NS::FVector2 GetSizeDelta() const;
        SR_NODISCARD bool AreAnchorsTogetherX() const;
        SR_NODISCARD bool AreAnchorsTogetherY() const;

        SR_NODISCARD Measurement GetMeasurement() const override { return Measurement::Space2D; }

        SR_NODISCARD const SR_MATH_NS::Matrix4x4& GetMatrix() const override;

        SR_NODISCARD const SR_MATH_NS::FRect& GetLayoutRect() const;

        SR_NODISCARD int32_t GetPriority() const;
        SR_NODISCARD int32_t GetLocalPriority() const noexcept { return m_localPriority; }

        SR_NODISCARD bool IsRelativePriority() const noexcept { return m_relativePriority; }

        void OnHierarchyChanged() override;

        void SetMaskInfo(const UI::MaskInfo& maskInfo);

        SR_NODISCARD const UI::MaskInfo& GetMaskInfo() const;

    public:
        SR_INLINE static constexpr SR_MATH_NS::FVector2 RIGHT = Math::FVector2(1, 0);
        SR_INLINE static constexpr SR_MATH_NS::FVector2 UP    = Math::FVector2(0, 1);

    private:
        void UpdatePriorityTree();
        void UpdateMaskInfoTree();

    protected:
        mutable SR_MATH_NS::Matrix4x4 m_localMatrix = SR_MATH_NS::Matrix4x4::Identity();
        mutable SR_MATH_NS::Matrix4x4 m_matrix = SR_MATH_NS::Matrix4x4::Identity();
        mutable SR_MATH_NS::Quaternion m_globalRotation = SR_MATH_NS::Quaternion::Identity();

        UI::MaskInfo m_localMaskInfo;
        mutable UI::MaskInfo m_globalMaskInfo;
        mutable bool m_maskInfoDirty = true;

        SR_MATH_NS::FVector2 m_canvasSize;

        /// @property @readOnly @dontSave
        mutable SR_MATH_NS::FRect m_layoutRect;
        /// @property @readOnly @dontSave
        mutable SR_MATH_NS::FRect m_localRect;

        mutable bool m_isDirtyPriority = true;
        mutable int32_t m_priority = 0;

        /// @property @setter(SetTranslation)
        SR_MATH_NS::FVector3 m_translation = SR_MATH_NS::FVector3::Zero();
        /// @property @setter(SetSize) @resetValue(SR_MATH_NS::FVector2(100.0f, 100.0f))
        SR_MATH_NS::FVector2 m_size = SR_MATH_NS::FVector2(100.0f, 100.0f);

        /// @property @setter(SetOffsetMin) @onChanged(UpdateTree)
        SR_MATH_NS::FVector2 m_offsetMin; // Left, Bottom
        /// @property @setter(SetOffsetMax) @onChanged(UpdateTree)
        SR_MATH_NS::FVector2 m_offsetMax; // Right, Top

        /// @property @onChanged(UpdateTree) @setter(SetAnchors)
        RectAnchors m_anchors;
        /// @property @resetValue(SR_MATH_NS::FVector2(0.5f, 0.5f)) @setter(SetPivot)
        SR_MATH_NS::FVector2 m_pivot = SR_MATH_NS::FVector2(0.5f, 0.5f);

        /// @property @setter(SetRotation) @dontSave
        SR_MATH_NS::FVector3 m_rotation = SR_MATH_NS::FVector3::Zero();
        /// @property @setter(SetRotation) @hidden
        SR_MATH_NS::Quaternion m_quaternion = SR_MATH_NS::Quaternion::Identity();

        /// @property @setter(SetScale) @resetValue(SR_MATH_NS::FVector3::One())
        SR_MATH_NS::FVector3 m_scale = SR_MATH_NS::FVector3::One();
        /// @property @setter(SetSkew) @resetValue(SR_MATH_NS::FVector3::One()) @hidden
        SR_MATH_NS::FVector3 m_skew = SR_MATH_NS::FVector3::One();

        /// @property @setter(SetLocalPriority)
        int32_t m_localPriority = 0;
        /// @property @setter(SetRelativePriority)
        bool m_relativePriority = true;

    };
}

#endif //SR_ENGINE_UTILS_TRANSFORM_RECT_H
