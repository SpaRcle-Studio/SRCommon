//
// Created by Monika on 16.10.2024.
//

#ifndef SR_COMMON_TYPE_TRAITS_SERIALIZATION_H
#define SR_COMMON_TYPE_TRAITS_SERIALIZATION_H

#include <Utils/Serialization/Serializer.h>
#include <Utils/Serialization/Deserializer.h>
#include <Utils/TypeTraits/Factory.h>
#include <Utils/Types/SharedPtr.h>

namespace SR_UTILS_NS {
	class Serializable;

	template<class T>
	using HasMethodEmptyT = decltype(std::declval<const T&>().empty());

	template<typename T>
	struct SerializationTraits
	{
		static constexpr bool HasEmpty = IsDetectedV<HasMethodEmptyT, T>;
		static constexpr bool IsSerializable = std::is_base_of_v<SR_UTILS_NS::Serializable, T>;
	};

	template<typename T, typename Enable = void> struct DefaultChecker {
		using IsAlwaysFalse = std::true_type;

		static bool IsDefault(const T&) { return false; }
	};

	namespace Details {
		template<typename T>
		using IsDefaultCheckerAlwaysFalseT = typename DefaultChecker<T>::IsAlwaysFalse;

		template<typename T>
		using ReserveMethodT = decltype(std::declval<T>().reserve(size_t()));
	}

	template<typename T> SR_INLINE bool IsDefault(const T& value) {
		return DefaultChecker<T>::IsDefault(value);
	}

	template<typename T>
	SR_INLINE constexpr bool IsDefaultCheckerAlwaysFalse = DetectedOrT<std::false_type, Details::IsDefaultCheckerAlwaysFalseT, T>::value;

	/// Default object makers for serialization

	#include <Utils/Serialization/DefaultObjectMakers.inl.h>

	/// Specialization for serializable types

	template<typename T, typename Enable = void> struct ObjectDataAccessor {
		static void Save(ISerializer&, const T&, const SerializationId&) {
			static_assert(AlwaysFalseV<T>, "Unable to save! Need to inherit from Serializable!");
		}

		static void Load(IDeserializer&, T&, const SerializationId&) {
			static_assert(AlwaysFalseV<T>, "Unable to load! Need to inherit from Serializable!");
		}
	};

	namespace Serialization {
		template<typename T> bool IsValidValue(const T& value) {
			if constexpr (CheckOperatorUsableV<CheckerEqualityComparable, T, std::nullptr_t>) {
				return value != nullptr;
			}
			else {
				SR_IGNORE_UNUSED(value);
				return true;
			}
		}

		template<typename T> void Save(ISerializer& serializer, const T& value, const SerializationId& key) {
			SR_UTILS_NS::ObjectDataAccessor<T>::Save(serializer, value, key);
		}

		template<typename T> void SaveCheckDefault(ISerializer& serializer, const T& value, const SerializationId& key) {
			if (!serializer.IsWriteDefaults() && IsDefault(value)) {
				return;
			}
			SR_UTILS_NS::ObjectDataAccessor<T>::Save(serializer, value, key);
		}

		template<typename T> bool Load(IDeserializer& deserializer, T& value, const SerializationId& key) {
			if (!deserializer.IsDefault(key)) {
				ObjectDataAccessor<T>::Load(deserializer, value, key);
				return true;
			}

			if constexpr (DefaultObjectMaker<T>::value) {
				if (deserializer.ShouldSetDefaults(key)) {
					DefaultObjectMaker<T>::MakeDefault(value);
				}
			}
			return false;
		}
	}

	/// Default checkers for serialization

	#include <Utils/Serialization/DefaultCheckers.inl.h>

	/// Data accessors for objects serialization

	#include <Utils/Serialization/ObjectDataAccessors.inl.h>
}

#endif //SR_COMMON_TYPE_TRAITS_SERIALIZATION_H
