//
// Created by Monika on 28.11.2026.
//

#ifndef SR_ENGINE_UTILS_WORLD_SHAPE_H
#define SR_ENGINE_UTILS_WORLD_SHAPE_H

#include <Utils/ECS/Component.h>
#include <Utils/Math/Vector4.h>

namespace SR_WORLD_NS {
    class Scene;
}

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(WorldShapeType, uint8_t,
        Flat,
        Sphere
    );

    /// Форма мира: где "верх" и какая высота над поверхностью. Нужна системам, которые по умолчанию
    /// считают верх осью Y (небо, атмосфера, облака, туман, ambient, высота солнца).
    /// Без компонента в сцене мир плоский, верх - ось Y, земля на высоте 0.
    /// Сфера - центр в позиции объекта, radius - радиус поверхности (уровень моря).
    /// @category(World)
    class SR_COMMON_DLL_API WorldShape final : public Component {
        SR_CLASS()
        using Super = Component;
    public:
        WorldShape();

    public:
        void OnEnable() override;
        void OnDisable() override;
        void OnDestroy() override;

        SR_NODISCARD bool ExecuteInEditMode() const override { return true; }

        /// @method @evaluate
        SR_NODISCARD WorldShapeType GetShape() const noexcept { return m_shape; }
        /// @method @evaluate
        SR_NODISCARD float_t GetRadius() const noexcept { return m_radius; }
        /// @method @evaluate
        SR_NODISCARD float_t GetGroundHeight() const noexcept { return m_groundHeight; }
        /// @method @evaluate
        SR_NODISCARD SR_MATH_NS::FVector3 GetCenter() const;
        /// @method @evaluate
        SR_NODISCARD bool IsSphere() const noexcept { return m_shape == WorldShapeType::Sphere && m_radius > 0.f; }

        /// Верх в точке мира
        /// @method @evaluate
        SR_NODISCARD SR_MATH_NS::FVector3 GetUp(const SR_MATH_NS::FVector3& position) const;
        /// Высота точки над поверхностью (плоский мир - над groundHeight, сфера - над радиусом)
        /// @method @evaluate
        SR_NODISCARD float_t GetHeight(const SR_MATH_NS::FVector3& position) const;
        /// Расстояние от точки до поверхности формы, по нему выбирается ближайшая форма
        SR_NODISCARD float_t GetDistanceToSurface(const SR_MATH_NS::FVector3& position) const;

        /// Параметры для шейдеров (WORLD_SHAPE, Common/world-shape.srsl) относительно точки наблюдения:
        /// сфера - (центр - viewPosition, radius), плоский мир - (0, groundHeight, 0, 0)
        SR_NODISCARD SR_MATH_NS::FVector4 GetShaderParams(const SR_MATH_NS::FVector3& viewPosition) const;

        void SetShape(WorldShapeType shape) { m_shape = shape; }
        void SetRadius(float_t radius) { m_radius = SR_MAX(radius, 0.f); }
        void SetGroundHeight(float_t height) { m_groundHeight = height; }

        /// Ближайшая к точке форма мира сцены, nullptr - нет ни одной (плоский мир по Y)
        SR_NODISCARD static const WorldShape* Find(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::FVector3& position);
        /// Верх в точке с учетом формы мира сцены (без формы - ось Y)
        SR_NODISCARD static SR_MATH_NS::FVector3 FindUp(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::FVector3& position);
        /// Высота над поверхностью с учетом формы мира сцены (без формы - position.y)
        SR_NODISCARD static float_t FindHeight(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::FVector3& position);
        /// Параметры для шейдеров с учетом формы мира сцены (без формы - плоский мир, земля на 0)
        SR_NODISCARD static SR_MATH_NS::FVector4 FindShaderParams(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::FVector3& viewPosition);

    private:
        void RegisterShape();
        void UnregisterShape();

    private:
        SR_WORLD_NS::Scene* m_pRegisteredScene = nullptr;

        /// @property @getter(GetShape) @setter(SetShape)
        WorldShapeType m_shape = WorldShapeType::Flat;
        /// @property @getter(GetRadius) @setter(SetRadius) @tooltip(Sphere surface (sea level) radius)
        float_t m_radius = 6360000.f;
        /// @property @getter(GetGroundHeight) @setter(SetGroundHeight) @tooltip(Flat world ground height)
        float_t m_groundHeight = 0.f;

    };
}

#endif //SR_ENGINE_UTILS_WORLD_SHAPE_H
