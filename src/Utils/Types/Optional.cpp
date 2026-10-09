//
// Created by Monika on 08.10.2026.
//

#include <Utils/Types/Optional.h>

#include <Codegen/Optional.generated.hpp>

namespace SR_UTILS_NS {
    SR_MAYBE_UNUSED_VAR OptionalUtils::Instance();

    Reflection::Value OptionalUtils::GetValue(const Reflection::Value& value) const {
        return value.GetOptionalBase()->GetReflectionValue();
    }

    bool OptionalUtils::HasValue(const Reflection::Value& value) const {
        return value.GetOptionalBase()->HasValue();
    }
}