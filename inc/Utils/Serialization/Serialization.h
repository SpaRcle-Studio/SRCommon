//
// Created by Monika on 16.10.2024.
//

#ifndef SR_COMMON_TYPE_TRAITS_SERIALIZATION_H
#define SR_COMMON_TYPE_TRAITS_SERIALIZATION_H

#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>
#include <Utils/Serialization/ObjectDataAccessor.h>
#include <Utils/TypeTraits/Factory.h>
#include <Utils/TypeTraits/SRClassMeta.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Common/StringAtomLiterals.h>
#include <Utils/Common/AssertFwd.h>

namespace SR_MATH_NS {
    class FColor;
    class Quaternion;
    class AABB;
    class Matrix3x3;
    class Matrix4x4;
    template<typename T> struct Rect;
}

namespace SR_HTYPES_NS {
    class UnicodeString;
}

#include <Utils/Serialization/SerializationTraits.h>
#include <Utils/Serialization/SerializationSaveUtils.h>
#include <Utils/Serialization/SerializationLoadUtils.h>

namespace SR_UTILS_NS {
    template<typename T> class Optional;
	/// Data accessors for objects serialization

	#include <Utils/Serialization/ObjectDataAccessors.inl.h>
	#include <Utils/Serialization/ObjectContainerDataAccessors.inl.h>
}

#endif //SR_COMMON_TYPE_TRAITS_SERIALIZATION_H
