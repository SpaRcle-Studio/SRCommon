//
// Created by Monika on 03.09.2026.
//

#ifndef SR_ENGINE_FLUX_COMPILE_UTILS_H
#define SR_ENGINE_FLUX_COMPILE_UTILS_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS::Reflection {
    class TypeInfo;
    class Method;
    class Value;
}

namespace SR_FLUX_NS {
    struct FluxCallable;
    enum class FluxGraphNodeType : uint8_t;

    /// Первый пин, с которого начинаются аргументы вызова.
    /// У Invoke нулевой пин занят потоком исполнения, поэтому объект сдвинут на единицу
    SR_NODISCARD extern SR_COMMON_DLL_API uint32_t GetCallObjectPin(FluxGraphNodeType type);

    /// Объект синглтона среда исполнения разрешает по имени, поэтому в инструкцию он не
    /// передаётся - его пин занят первым аргументом вызова
    SR_NODISCARD extern SR_COMMON_DLL_API bool IsSingletonCallable(const FluxCallable& callable);

    /// Пин, с которого начинаются аргументы вызова
    SR_NODISCARD extern SR_COMMON_DLL_API uint32_t GetFirstArgumentPin(FluxGraphNodeType type, const FluxCallable& callable);

    /// Значение по умолчанию для типа параметра. Используется, когда входной пин выходного
    /// аргумента не подключен
    SR_NODISCARD extern SR_COMMON_DLL_API Reflection::Value MakeDefaultValue(Reflection::TypeInfo* pTypeInfo);

    /// Является ли выходной пин узла пином данных, а не потока исполнения
    SR_NODISCARD extern SR_COMMON_DLL_API bool IsDataOutputPin(FluxGraphNodeType type, uint32_t pinIndex);

    /// Индекс первого выходного пина, отведённого под выходные аргументы вызова.
    /// FluxInvalidPin, если узел не является вызовом
    SR_NODISCARD extern SR_COMMON_DLL_API uint32_t GetFluxOutArgumentBasePin(FluxGraphNodeType type, const Reflection::Method* pMethod) noexcept;

    /// Производит ли нулевой выходной пин узла Evaluate (или первый - узла Invoke) результат вызова
    SR_NODISCARD extern SR_COMMON_DLL_API bool HasFluxResultPin(FluxGraphNodeType type, const Reflection::Method* pMethod) noexcept;

    /// Выходной пин, на котором узел публикует изменённое значение аргумента paramIndex.
    /// FluxInvalidPin, если аргумент не является выходным
    SR_NODISCARD extern SR_COMMON_DLL_API uint32_t GetFluxOutArgumentPin(FluxGraphNodeType type, const Reflection::Method* pMethod, const uint32_t paramIndex) noexcept;
}

#endif //SR_ENGINE_FLUX_COMPILE_UTILS_H
