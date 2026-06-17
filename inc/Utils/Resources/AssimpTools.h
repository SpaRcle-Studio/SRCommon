//
// Created by Monika on 16.06.2026.
//

#ifndef SR_ENGINE_COMMON_ASSIMP_TOOLS_H
#define SR_ENGINE_COMMON_ASSIMP_TOOLS_H

#include <Utils/Math/Matrix4x4.h>

#ifdef SR_UTILS_ASSIMP

struct aiMesh;
template<typename T> struct aiMatrix4x4t;

namespace SR_UTILS_NS::AssimpTools {
    SR_COMMON_DLL_API extern uint32_t NormalizeWeights(const aiMesh* pMesh);
    SR_COMMON_DLL_API extern SR_MATH_NS::Matrix4x4 ConvertAssimpMatrix(const aiMatrix4x4t<float_t>& matrix);
}

#endif

#endif //SR_ENGINE_COMMON_ASSIMP_TOOLS_H
