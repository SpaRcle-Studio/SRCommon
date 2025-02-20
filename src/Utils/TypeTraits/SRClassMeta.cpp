//
// Created by Monika on 16.10.2024.
//

#include <Utils/TypeTraits/SRClassMeta.h>

namespace SR_UTILS_NS {
    void SRClassMeta::Save(SR_UTILS_NS::ISerializer& serializer, const SR_UTILS_NS::Serializable& obj) const {
		for (auto&& pMeta : GetBaseMetas()) {
			pMeta->Save(serializer, obj);
		}
    }

    void SRClassMeta::Load(SR_UTILS_NS::IDeserializer& deserializer, SR_UTILS_NS::Serializable& obj) const {
    	for (auto&& pMeta : GetBaseMetas()) {
    		pMeta->Load(deserializer, obj);
    	}
    }

    bool SRClassMeta::IsInherited(std::string_view baseClass) const noexcept {
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

    void SRClassMeta::ForEachProperty(const std::function<void(const SR_UTILS_NS::Reflection::Property& property, uint64_t index)>& func, uint64_t* pIndex) const  {
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
}

