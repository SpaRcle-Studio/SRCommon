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

        SR_NODISCARD virtual SRClassMeta* GetType(std::string_view name) const = 0;

        SR_NODISCARD bool IsRegistered(const SRClassMeta* pMeta) const {
            return pMeta && GetType(pMeta->GetFactoryName()) == pMeta;
        }
    };

    template<typename T> class Factory : public SR_UTILS_NS::Singleton<Factory<T>>, public BaseFactory {
        SR_REGISTER_TEMPLATE_SINGLETON(Factory, T);
        using ClassPtrT = SR_HTYPES_NS::SharedPtr<T>;
        using AllocatorT = std::function<ClassPtrT()>;
        using MetaGetterT = SRClassMeta*(*)();
        struct TypeInfo {
            AllocatorT allocator;
            MetaGetterT metaGetter;
        };
    public:
        SR_NODISCARD std::string_view GetName(const SRClassMeta* pMeta, bool isMustExists = true) const;

        bool Register(std::string_view name, AllocatorT allocator, MetaGetterT metaGetter) {
            auto&& pIt = m_types.find(name);
            if (pIt != m_types.end()) {
                return false;
            }
            m_types[name] = {allocator, metaGetter};
            return true;
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

        SR_NODISCARD ClassPtrT Create(std::string_view name) const noexcept {
            auto&& pIt = m_types.find(name);
            if (pIt != m_types.end()) {
                return pIt->second.allocator();
            }
            return nullptr;
        }

        SR_NODISCARD SRClassMeta* GetType(std::string_view name) const noexcept override {
            auto&& pIt = m_types.find(name);
            if (pIt != m_types.end()) {
                return pIt->second.metaGetter();
            }
            return nullptr;
        }

    private:
        ska::flat_hash_map<std::string_view, TypeInfo> m_types;

    };

    /// Implementation

    template<typename T> std::string_view Factory<T>::GetName(const SRClassMeta* pMeta, const bool isMustExists) const {
        if (IsRegistered(pMeta)) {
			return pMeta->GetFactoryName(); /// NOLINT
		}

		if (isMustExists) {
			if (!pMeta) {
				SRHalt("Factory::GetName() : meta is nullptr!");
			}
			else {
				SRHalt("Factory::GetName() : meta is not registered!");
			}
		}
		return {};
    }
}

#endif //SR_COMMON_TYPE_TRAITS_FACTORY_H
