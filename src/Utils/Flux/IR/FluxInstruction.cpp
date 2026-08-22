//
// Created by Monika on 23.08.2026.
//

#include <Utils/Flux/IR/FluxInstruction.h>
#include <Utils/Reflection/Method.h>
#include <Utils/TypeTraits/Factory.h>

namespace SR_FLUX_NS {
    const Reflection::Method* FluxCallable::FindMethodMeta() const {
        SR_TRACY_ZONE;
        if (auto&& pMeta = Factory::Instance().GetType(object)) {
            return pMeta->FindMethod(function);
        }
        return nullptr;
    }
}