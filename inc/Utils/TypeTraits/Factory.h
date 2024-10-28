//
// Created by Monika on 17.10.2024.
//

#ifndef SR_COMMON_TYPE_TRAITS_FACTORY_H
#define SR_COMMON_TYPE_TRAITS_FACTORY_H

#include <Utils/Common/Singleton.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/TypeTraits/SRClassMeta.h>

namespace SR_UTILS_NS {
    class SRClassMeta;

    class BaseFactory {
    public:
        virtual ~BaseFactory() = default;

        SR_NODISCARD virtual const SRClassMeta* GetType(std::string_view name) const = 0;

        SR_NODISCARD bool IsRegistered(const SRClassMeta* pMeta) const {
            return pMeta && GetType(pMeta->GetFactoryName()) == pMeta;
        }
    };

    class Factory : public BaseFactory {
        using ClassPtrT = SR_HTYPES_NS::SharedPtr<SRClass>;
        using AllocatorT = std::function<ClassPtrT()>;
        using MetaGetterT = const SRClassMeta*(*)();
        struct TypeInfo {
            AllocatorT allocator;
            MetaGetterT metaGetter = nullptr;
        };
    public:
        SR_NODISCARD static Factory& Instance() noexcept;

        SR_NODISCARD std::string_view GetName(const SRClassMeta* pMeta, bool isMustExists = true) const;

        template<class T> bool Register() {
            if constexpr (std::is_abstract_v<T>) {
                return false;
            }
            else if constexpr (std::is_same_v<T, void>) {
                static_assert(AlwaysFalseV<T>, "Type must be specified!");
            }
            else if constexpr (!std::is_default_constructible_v<T>) {
                static_assert(AlwaysFalseV<T>, "Type must be default constructible!");
            }
            else if (auto&& pMeta = T::GetMetaStatic()) {
                auto&& name = pMeta->GetFactoryName();
                TypeInfo& info = m_types[name];
                info.allocator = []() { return SRNew<T>(); };
                info.metaGetter = T::GetMetaStatic;
                return true;
            }
            return false;
        }

        template<class Y> SR_NODISCARD std::string_view GetName(Y* pObject, const bool isMustExists = true) const {
            if (SRVerify(pObject)) {
                return GetName(pObject->GetMeta(), isMustExists);
            }
            return {};
        }

        template<class Y> SR_NODISCARD std::string_view GetName() const {
            return GetName(Y::GetMetaStatic(), true);
        }

        template<typename T = SRClass> SR_NODISCARD SR_HTYPES_NS::SharedPtr<T> Create(std::string_view name) const noexcept {
            auto&& pClass = CreateBase(name);
            if constexpr (std::is_same_v<T, SRClass>) {
                return pClass;
            }
            return pClass.DynamicCast<T>();
        }

        SR_NODISCARD ClassPtrT CreateBase(std::string_view name) const noexcept {
            auto&& pIt = m_types.find(name);
            if (pIt != m_types.end()) {
                return pIt->second.allocator();
            }
            return nullptr;
        }

        SR_NODISCARD const SRClassMeta* GetType(std::string_view name) const noexcept override {
            auto&& pIt = m_types.find(name);
            if (pIt != m_types.end()) {
                return pIt->second.metaGetter();
            }
            return nullptr;
        }

    private:
        std::unordered_map<std::string_view, TypeInfo> m_types;

    };
}

#endif //SR_COMMON_TYPE_TRAITS_FACTORY_H
