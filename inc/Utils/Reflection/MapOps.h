//
// Created by Codex on 21.11.2026.
//

#ifndef SR_ENGINE_UTILS_REFLECTION_MAP_OPS_H
#define SR_ENGINE_UTILS_REFLECTION_MAP_OPS_H

#include <Utils/Reflection/VTable.h>

namespace SR_UTILS_NS::Reflection {
    extern SR_COMMON_DLL_API bool FillTreeMapVTable(const TypeInfoKey& key, TypeInfoVTable& vtable);
}

#endif // SR_ENGINE_UTILS_REFLECTION_MAP_OPS_H
