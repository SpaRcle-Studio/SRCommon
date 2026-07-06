//
// Created by Monika on 16.07.2025.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_SR_CLASS_UTILS_H
#define SR_ENGINE_COMMON_REFLECTION_SR_CLASS_UTILS_H

#include <Utils/Reflection/Property.h>
#include <Utils/TypeTraits/SRClass.h>
#include <Utils/TypeTraits/Factory.h>

namespace SR_UTILS_NS::Reflection {
    template<typename T> SR_INLINE void CloneTo(const T& from, T& to) {
        if constexpr (IsSRClassV<T>) {
            from.CloneTo(to);
        }
        else if constexpr (IsSharedPointerV<T>) {
            if (from) {
                auto&& pClone = SR_UTILS_NS::Factory::Instance().Create<InnerTypeT<T>>(from->GetMeta()->GetFactoryName());
                from->CloneTo(*pClone);
                to = std::move(pClone);
            }
        }
        else if constexpr (IsStdVectorV<T> && ContainsSRClassV<T>) {
            to.resize(from.size());
            for (size_t i = 0; i < from.size(); ++i) {
                CloneTo(from[i], to[i]);
            }
        }
        else if constexpr (IsStdSetV<T> && ContainsSRClassV<T>) {
            to.clear();
            for (auto&& item : from) {
                InnerTypeT<T> cloneValue;
                CloneTo(item, cloneValue);
                to.insert(std::move(cloneValue));
            }
        }
        else if constexpr (IsStdMapV<T> && ContainsSRClassV<T>) {
            to.clear();
            for (auto&& [key, value] : from) {
                InnerTypeT<T> cloneValue;
                CloneTo(value, cloneValue);
                to.emplace(key, std::move(cloneValue));
            }
        }
        else {
            to = from;
        }
    }

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
