//
// Created by Monika on 16.06.2026.
//

#ifndef SR_ENGINE_COMMON_ASSIMP_TOOLS_H
#define SR_ENGINE_COMMON_ASSIMP_TOOLS_H

#include <Utils/stdInclude.h>

#ifdef SR_UTILS_ASSIMP

struct aiMesh;

namespace SR_UTILS_NS::AssimpTools {
    SR_COMMON_DLL_API extern uint32_t NormalizeWeights(const aiMesh* pMesh);
}

#endif

#endif //SR_ENGINE_COMMON_ASSIMP_TOOLS_H
