//
// Created by Monika on 03.09.2026.
//

#include <Utils/Flux/Graph/FluxCompileUtils.h>
#include <Utils/Flux/Graph/FluxGraph.h>

namespace SR_FLUX_NS {
    uint32_t GetCallObjectPin(const FluxGraphNodeType type) {
        return type == FluxGraphNodeType::Invoke ? 1 : 0;
    }

    bool IsSingletonCallable(const FluxCallable& callable) {
        return GetSingletonManager()->GetSingletonMeta(callable.object.GetHash()) != nullptr;
    }

    uint32_t GetFirstArgumentPin(const FluxGraphNodeType type, const FluxCallable& callable) {
        const uint32_t objectPin = GetCallObjectPin(type);
        return IsSingletonCallable(callable) ? objectPin : objectPin + 1;
    }

    Reflection::Value MakeDefaultValue(Reflection::TypeInfo* pTypeInfo) {
        if (!pTypeInfo || (!pTypeInfo->vtable.pConstructor && !Reflection::FindVTable(*pTypeInfo))) {
            return {};
        }
        return Reflection::Value::CreateDefault(pTypeInfo);
    }

    bool IsDataOutputPin(const FluxGraphNodeType type, const uint32_t pinIndex) {
        switch (type) {
            case FluxGraphNodeType::Event:
                return pinIndex >= 1;
            case FluxGraphNodeType::Invoke:
                /// первый пин - возвращаемое значение, за ним идут выходные аргументы
                return pinIndex >= 1;
            case FluxGraphNodeType::For:
            case FluxGraphNodeType::Cast:
                return pinIndex == 2;
            case FluxGraphNodeType::Evaluate:
                /// нулевой пин - результат вызова, за ним идут выходные аргументы
                return true;
            case FluxGraphNodeType::Constant:
            case FluxGraphNodeType::ReadVariable:
                return pinIndex == 0;
            default:
                return false;
        }
    }

    uint32_t GetFluxOutArgumentBasePin(const FluxGraphNodeType type, const Reflection::Method* pMethod) noexcept {
        /// нулевой выходной пин у Invoke занят потоком исполнения, а у Evaluate - результатом.
        /// Пин результата существует только у методов, которые что-то возвращают. Сигнатура
        /// неизвестного метода считается возвращающей значение - так раскладка совпадает с той,
        /// что строил редактор до появления метаданных
        const bool hasResult = !pMethod || pMethod->HasReturn();
        switch (type) {
            case FluxGraphNodeType::Invoke:
                return hasResult ? 2 : 1;
            case FluxGraphNodeType::Evaluate:
                return hasResult ? 1 : 0;
            default:
                return FluxInvalidPin;
        }
    }

    bool HasFluxResultPin(const FluxGraphNodeType type, const Reflection::Method* pMethod) noexcept {
        return (type == FluxGraphNodeType::Invoke || type == FluxGraphNodeType::Evaluate) && (!pMethod || pMethod->HasReturn());
    }

    uint32_t GetFluxOutArgumentPin(const FluxGraphNodeType type, const Reflection::Method* pMethod, const uint32_t paramIndex) noexcept {
        const uint32_t basePin = GetFluxOutArgumentBasePin(type, pMethod);
        if (!pMethod || basePin == FluxInvalidPin || !pMethod->IsOutputParam(paramIndex)) {
            return FluxInvalidPin;
        }
        uint32_t offset = 0;
        for (uint32_t i = 0; i < paramIndex; ++i) {
            offset += pMethod->IsOutputParam(i) ? 1 : 0;
        }
        return basePin + offset;
    }
}