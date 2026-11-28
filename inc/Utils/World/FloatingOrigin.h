//
// Created by Monika on 28.11.2026.
//

#ifndef SR_ENGINE_UTILS_FLOATING_ORIGIN_H
#define SR_ENGINE_UTILS_FLOATING_ORIGIN_H

#include <Utils/ECS/Component.h>

namespace SR_WORLD_NS {
    class Scene;
}

namespace SR_UTILS_NS {
    /// Плавающее начало координат для больших миров. Когда наблюдатель уходит от начала дальше порога,
    /// весь мир сдвигается (Scene::ShiftOrigin), чтобы наблюдатель снова оказался у нуля: Transform и физика
    /// остаются во float с полной точностью вблизи камеры. Абсолютная позиция = origin (double) + локальная.
    /// Наблюдателя каждый кадр передаёт движок после физики и LateUpdate (EngineScene): в режиме редактора -
    /// редакторская камера, иначе игровая. Без компонента начало не сдвигается, абсолютные координаты = локальные.
    /// @category(World)
    class SR_COMMON_DLL_API FloatingOrigin final : public Component {
        SR_CLASS()
        using Super = Component;
    public:
        FloatingOrigin();

    public:
        void OnEnable() override;
        void OnDisable() override;
        void OnDestroy() override;

        /// Регистрируется и в редакторе, чтобы абсолютное начало было известно всегда. Сдвиг вне игры - shiftInEditMode
        SR_NODISCARD bool ExecuteInEditMode() const override { return true; }

        /// Сдвигает мир, если наблюдатель дальше порога. isRunning - сцена симулируется (игра запущена и не на паузе),
        /// иначе сдвиг только при shiftInEditMode. true - сдвиг был
        bool UpdateObserver(const SR_MATH_NS::FVector3& observerPosition, bool isRunning);

        /// Сдвинуть начало на delta: всё в сцене переезжает на -delta, абсолютное начало растёт на delta
        /// @method
        void ShiftOrigin(const SR_MATH_NS::FVector3& delta);

        SR_NODISCARD SR_MATH_NS::Vector3<double_t> GetOrigin() const noexcept;
        SR_NODISCARD SR_MATH_NS::Vector3<double_t> ToAbsolute(const SR_MATH_NS::FVector3& local) const noexcept;
        SR_NODISCARD SR_MATH_NS::FVector3 ToLocal(const SR_MATH_NS::Vector3<double_t>& absolute) const noexcept;

        /// Начало координат приблизительно во float - для шейдерной процедурики (шум, текстуры)
        /// @method @evaluate
        SR_NODISCARD SR_MATH_NS::FVector3 GetOriginApprox() const noexcept;

        /// Активный FloatingOrigin сцены, nullptr - начало не плавает
        SR_NODISCARD static FloatingOrigin* Find(const SR_WORLD_NS::Scene* pScene);
        /// Абсолютное начало сцены (без компонента - ноль)
        SR_NODISCARD static SR_MATH_NS::Vector3<double_t> FindOrigin(const SR_WORLD_NS::Scene* pScene);
        SR_NODISCARD static SR_MATH_NS::Vector3<double_t> FindAbsolute(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::FVector3& local);
        SR_NODISCARD static SR_MATH_NS::FVector3 FindLocal(const SR_WORLD_NS::Scene* pScene, const SR_MATH_NS::Vector3<double_t>& absolute);

    private:
        void RegisterOrigin();
        void UnregisterOrigin();

    private:
        SR_WORLD_NS::Scene* m_pRegisteredScene = nullptr;

        /// @property @tooltip(Shift the world when the observer is farther from the origin than this distance, meters)
        float_t m_threshold = 2048.f;
        /// @property @tooltip(Shift is rounded to a multiple of this step, meters. 0 - shift exactly to the observer)
        float_t m_step = 1024.f;
        /// @property @tooltip(Shift the world outside of the play mode too (editor). Undo history keeps old positions)
        bool m_shiftInEditMode = false;

        /// @property @readOnly @tooltip(Absolute position of the local origin)
        double_t m_originX = 0.0;
        /// @property @readOnly
        double_t m_originY = 0.0;
        /// @property @readOnly
        double_t m_originZ = 0.0;

    };
}

#endif //SR_ENGINE_UTILS_FLOATING_ORIGIN_H
