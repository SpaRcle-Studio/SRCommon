//
// Created by Monika on 16.10.2024.
//

#include <Utils/Reflection/Property.h>
#include <Utils/Reflection/Method.h>
#include <Utils/TypeTraits/SRClass.h>
#include <Utils/TypeTraits/SRClassMeta.h>
#include <Utils/Types/StringAtom.h>

namespace SR_UTILS_NS {
    SRClassMeta::~SRClassMeta() = default;

    void SRClassMeta::Save(ISerializer& serializer, const Serializable& obj) const {
		for (auto&& pMeta : GetBaseMetas()) {
			pMeta->Save(serializer, obj);
		}
    }

    bool SRClassMeta::Load(IDeserializer& deserializer, Serializable& obj) const {
    	for (auto&& pMeta : GetBaseMetas()) {
    		if (!pMeta->Load(deserializer, obj)) {
    			return false;
    		}
    	}
        return true;
    }

    bool SRClassMeta::IsInherited(StringAtom baseClass) const noexcept {
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

    void SRClassMeta::ForEachProperty(const SR_HTYPES_NS::Function<void(const Reflection::Property& property, uint64_t index)>& func, uint64_t* pIndex) const {
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

    void SRClassMeta::ForEachMethod(const SR_HTYPES_NS::Function<void(const Reflection::Method& method, uint64_t index)>& func, uint64_t* pIndex) const {
    	uint64_t index = 0;
    	if (!pIndex) {
    		pIndex = &index;
    	}

    	for (auto&& pBase : GetBaseMetas()) {
    		pBase->ForEachMethod(func, pIndex);
    	}

    	for (auto&& method : GetMethods()) {
    		func(method, *pIndex);
    		++(*pIndex);
    	}
    }

    std::span<const StringAtom> SRClassMeta::GetCategory() const noexcept {
    	for (auto&& pBase : GetBaseMetas()) {
			if (!pBase->GetCategory().empty()) {
				return pBase->GetCategory();
			}
		}
    	return {};
    }

    std::span<const StringAtom> SRClassMeta::GetExtensions() const noexcept {
    	for (auto&& pBase : GetBaseMetas()) {
			if (!pBase->GetExtensions().empty()) {
				return pBase->GetExtensions();
			}
		}
    	return {};
    }

    bool SRClassMeta::HasExtension(StringAtom extension) const noexcept {
        auto&& extensions = GetExtensions();
        return std::ranges::any_of(extensions, [extension](auto&& ext) { return ext == extension; });
    }

    bool SRClassMeta::HasExtension(StringView extension) const noexcept {
        auto&& extensions = GetExtensions();
        return std::ranges::any_of(extensions, [extension](auto&& ext) { return ext == extension; });
    }

    bool SRClassMeta::HasExtension(const String& extension) const noexcept {
        auto&& extensions = GetExtensions();
        return std::ranges::any_of(extensions, [extension](auto&& ext) { return ext == extension; });
    }

    StringAtom SRClassMeta::GetExtension() const noexcept {
        static const StringAtom empty = {};
        auto&& extensions = GetExtensions();
        return extensions.empty() ? empty : extensions.front();
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

    StringAtom SRClassMeta::GetInspectorName() const noexcept {
        static const StringAtom def = "ObjectPropertyDrawer";
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

    std::span<const Reflection::Property> SRClassMeta::GetProperties() const noexcept {
        return std::span<const Reflection::Property>();
    }

    std::span<const Reflection::Method> SRClassMeta::GetMethods() const noexcept {
        return std::span<const Reflection::Method>();
    }

    StringAtom SRClassMeta::GetFactoryName() const noexcept {
        return StringAtom();
    }

    StringAtom SRClassMeta::GetDisplayName() const noexcept {
        return GetFactoryName();
    }

    SRClass* SRClassMeta::Allocate() const noexcept {
        return nullptr;
    }

    void SRClassMeta::ForEachSRClass(SRClass& srClass, const SR_HTYPES_NS::Function<void(SRClass&)>& function) const noexcept {
        for (auto&& pBase : GetBaseMetas()) {
            pBase->ForEachSRClass(srClass, function);
        }
    }

    bool SRClassMeta::IsSameOrInherited(StringAtom name) const {
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

    void SRClassMeta::CloneTo(const SRClass& src, SRClass& dest) const noexcept {
        if (src.GetMeta() != dest.GetMeta()) SR_UNLIKELY_ATTRIBUTE {
            SRHalt("Cannot clone from {} to {}: meta mismatch!", src.GetMeta()->GetFactoryName(), dest.GetMeta()->GetFactoryName());
        }

        for (auto&& pBase : GetBaseMetas()) {
            pBase->CloneTo(src, dest);
        }
    }

    const Reflection::Method* SRClassMeta::FindMethod(StringAtom name) const noexcept {
        for (auto&& method : GetMethods()) {
            if (method.GetName() == name) {
                return &method;
            }
        }
        for (auto&& pBase : GetBaseMetas()) {
            if (auto&& pMethod = pBase->FindMethod(name)) {
                return pMethod;
            }
        }
        return nullptr;
    }

    Reflection::TypeInfoVTable SRClassMeta::GetVTable() const noexcept {
        return Reflection::TypeInfoVTable();
    }

    Reflection::Value SRClassMeta::Cast(const Reflection::Value& value) const noexcept {
        return Reflection::Value();
    }
}
