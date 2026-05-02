//
// Created by Monika on 14.02.2026.
//

#ifndef SR_ENGINE_UTILS_PCH_H
#define SR_ENGINE_UTILS_PCH_H

#include <Utils/pchBase.h>

#define SR_ENGINE_COMMON_PCH_FOR_BASE_CODE

#include <Utils/Math/Vector2.h>
#include <Utils/Math/Vector3.h>
#include <Utils/Math/Vector4.h>
#include <Utils/Math/Rect.h>
#include <Utils/Math/Matrix3x3.h>
#include <Utils/Math/Matrix4x4.h>

#include <Utils/Common/AssertFwd.h>
#include <Utils/Common/Enumerations.h>
#include <Utils/Common/Hashes.h>

#include <Utils/Types/StringAtom.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/SafePointer.h>
#include <Utils/Types/Function.h>
#include <Utils/Types/String.h>

#include <Utils/Reflection/Value.h>
#include <Utils/Reflection/Property.h>

#include <Utils/Serialization/Serializable.h>
#include <Utils/TypeTraits/SRClassMeta.h>

#undef SR_ENGINE_COMMON_PCH_FOR_BASE_CODE

#endif //SR_ENGINE_UTILS_PCH_H
