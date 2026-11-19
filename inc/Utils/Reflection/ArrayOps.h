//
// Created by Codex on 19.11.2026.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_ARRAY_OPS_H
#define SR_ENGINE_COMMON_REFLECTION_ARRAY_OPS_H

#include <Utils/Reflection/VTable.h>

namespace SR_UTILS_NS::Reflection {
    struct TypeInfoKey;
    SR_COMMON_DLL_API bool FillVectorVTable(const TypeInfoKey& key, TypeInfoVTable& vtable);
}

#endif //SR_ENGINE_COMMON_REFLECTION_ARRAY_OPS_H
