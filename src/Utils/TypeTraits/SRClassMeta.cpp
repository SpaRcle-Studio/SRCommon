//
// Created by Monika on 16.10.2024.
//

#include <Utils/TypeTraits/SRClassMeta.h>

namespace SR_UTILS_NS {
    SRClassMeta::~SRClassMeta() = default;

    void SRClassMeta::Save(SR_UTILS_NS::ISerializer& serializer, const SR_UTILS_NS::Serializable& obj) const {
		for (auto&& pMeta : GetBaseMetas()) {
			pMeta->Save(serializer, obj);
		}
    }

    bool SRClassMeta::Load(SR_UTILS_NS::IDeserializer& deserializer, SR_UTILS_NS::Serializable& obj) const {
    	for (auto&& pMeta : GetBaseMetas()) {
    		if (!pMeta->Load(deserializer, obj)) {
    			return false;
    		}
    	}
        return true;
    }

    bool SRClassMeta::IsInherited(SR_UTILS_NS::StringAtom baseClass) const noexcept {
		if (GetFactoryName() == baseClass) {
			return false;
		}

    	for (auto&& pMeta : GetBaseMetas()) {
			if (pMeta->GetFactoryName() == baseClass) {
				return true;
			}

			if (pMeta->IsInherited(baseClass)) {
				return true;
			}
		}

    	return false;
    }

    void SRClassMeta::ForEachProperty(const std::function<void(const SR_UTILS_NS::Reflection::Property& property, uint64_t index)>& func, uint64_t* pIndex) const {
    	uint64_t index = 0;
    	if (!pIndex) {
    		pIndex = &index;
    	}

    	for (auto&& pBase : GetBaseMetas()) {
    		pBase->ForEachProperty(func, pIndex);
    	}

    	for (auto&& property : GetProperties()) {
    		func(property, *pIndex);
    		++(*pIndex);
    	}
    }

    std::span<const SR_UTILS_NS::StringAtom> SRClassMeta::GetCategory() const noexcept {
    	for (auto&& pBase : GetBaseMetas()) {
			if (!pBase->GetCategory().empty()) {
				return pBase->GetCategory();
			}
		}
    	return {};
    }

    uint64_t SRClassMeta::GetVersion() const noexcept {
        if (m_versionCached != SR_UINT64_MAX) {
            return m_versionCached;
        }

        m_versionCached = GetVersionImpl();

        for (auto&& pBase : GetBaseMetas()) {
            m_versionCached = std::max(m_versionCached, pBase->GetVersion());
        }

        return m_versionCached;
    }

    SR_UTILS_NS::StringAtom SRClassMeta::GetInspectorName() const noexcept {
        static const SR_UTILS_NS::StringAtom def = "ObjectPropertyDrawer";
        return def;
    }

    uint64_t SRClassMeta::GetVersionImpl() const noexcept {
        return 0;
    }

    bool SRClassMeta::IsAbstract() const noexcept {
        return false;
    }

    bool SRClassMeta::IsHidden() const noexcept {
        return false;
    }

    bool SRClassMeta::IsEditorOnly() const noexcept {
        return false;
    }

    std::span<const SRClassMeta*> SRClassMeta::GetBaseMetas() const noexcept {
        return std::span<const SRClassMeta*>();
    }

    std::span<const SR_UTILS_NS::Reflection::Property> SRClassMeta::GetProperties() const noexcept {
        return std::span<const SR_UTILS_NS::Reflection::Property>();
    }

    SR_UTILS_NS::StringAtom SRClassMeta::GetFactoryName() const noexcept {
        return SR_UTILS_NS::StringAtom();
    }

    SRClass* SRClassMeta::Allocate() const noexcept {
        return nullptr;
    }

    void SRClassMeta::ForEachSRClass(SRClass& srClass, const SR_HTYPES_NS::Function<void(SRClass&)>& function) const noexcept {
        for (auto&& pBase : GetBaseMetas()) {
            pBase->ForEachSRClass(srClass, function);
        }
    }

    bool SRClassMeta::IsSameOrInherited(SR_UTILS_NS::StringAtom name) const {
        if (GetFactoryName() == name) {
            return true;
        }
        for (auto&& pBase : GetBaseMetas()) {
            if (pBase->IsSameOrInherited(name)) {
                return true;
            }
        }
        return false;
    }
}

