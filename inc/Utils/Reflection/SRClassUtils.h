//
// Created by Monika on 16.07.2025.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_SR_CLASS_UTILS_H
#define SR_ENGINE_COMMON_REFLECTION_SR_CLASS_UTILS_H

#include <Utils/Serialization/Serializable.h>
#include <Utils/Reflection/Property.h>
#include <Utils/TypeTraits/SRClass.h>

namespace SR_UTILS_NS::Reflection {
    /*static void ForEachSRClass(const SR_UTILS_NS::Reflection::Value& value, const SR_HTYPES_NS::Function<void(SRClass&)>& callback) {
        if (value.IsClass()) {
            if (auto&& pSRClass = value.GetSRClass()) {
                callback(*pSRClass);
                pSRClass->GetMeta()->ForEachProperty([&callback, pSRClass](const SR_UTILS_NS::Reflection::Property& property, uint64_t index) {
                    ForEachSRClass(property.Get(pSRClass), callback);
                });
                return;
            }
        }

        if (value.IsSequenceContainer()) {
            auto&& container = value.AsSequenceContainer();
            for (auto&& pIt = container.begin(); pIt != container.end(); ++pIt) {
                ForEachSRClass(*pIt, callback);
            }
        }
    }*/

    template<typename T>
    constexpr bool ContainsSRClassV = IsSRClassV<InnerTypeT<T>>;

    template<typename T> SR_INLINE void ForEachSRClass(T&& object, const auto& func) {
        if constexpr (IsSRClassV<T>) {
            func(object);
        }
        else if constexpr ((IsStdVectorV<T> || IsStdSetV<T>)) {
            if constexpr (ContainsSRClassV<T>) {
                for (auto&& item : object) {
                    ForEachSRClass(item, func);
                }
            }
        }
        //else if constexpr (IsStdMapV<T>) {
        //    for (auto&& [key, value] : object) {
        //        ForEachSRClass(value, func);
        //    }
        //}
        else if constexpr (IsSharedPointerV<T>) {
            if constexpr (IsSRClassV<typename RemoveQualifiersT<T>::SharedPointerType>) {
                if (object) {
                    func(*object);
                }
            }
        }
    }
}

#endif //SR_ENGINE_COMMON_REFLECTION_SERIALIZABLE_UTILS_H
