//
// Created by Monika on 11.12.2025.
//

#ifndef SR_ENGINE_UTILS_SERIALIZATION_SAVE_UTILS_H
#define SR_ENGINE_UTILS_SERIALIZATION_SAVE_UTILS_H

namespace SR_UTILS_NS {
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

	#include <Utils/Serialization/DefaultCheckers.inl.h>
	#include <Utils/Serialization/SaveCheckers.inl.h>

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

		template<typename T> bool CanBeSaved(const T& value) {
			return SR_UTILS_NS::SaveChecker<T>::CanBeSaved(value);
		}

		template<typename T> void Save(ISerializer& serializer, const T& value, const SerializationId& key) {
			if (!CanBeSaved(value)) {
				return;
			}
			SR_UTILS_NS::ObjectDataAccessor<T>::Save(serializer, value, key);
		}

		template<typename T> void SaveCheckDefault(ISerializer& serializer, const T& value, const SerializationId& key) {
			if (!serializer.IsWriteDefaults() && IsDefault(value)) {
				return;
			}
			Save(serializer, value, key);
		}
	}
}


#endif //SR_ENGINE_UTILS_SERIALIZATION_SAVE_UTILS_H
