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
}

