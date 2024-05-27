//
// Created by Monika on 06.11.2023.
//

#include <Utils/TypeTraits/Properties.h>

namespace SR_UTILS_NS {
    PropertyContainer::PropertyContainer() {
        m_properties.reserve(16);
    }

    PropertyContainer::~PropertyContainer() {
        SRAssert2(m_properties.empty(), "PropertyContainer::~PropertyContainer() : properties are not empty!");
    }

    PropertyContainer& PropertyContainer::AddContainer(const char* name) {
        if (auto&& pProperty = Find(name)) {
            SRHalt("Properties::AddContainer() : property \"" + std::string(name) + "\" already exists!");
            return *dynamic_cast<PropertyContainer*>(pProperty);
        }

        auto&& pProperty = new PropertyContainer();

        pProperty->SetName(name);

        PropertyInfo propertyInfo;
        propertyInfo.pProperty = pProperty;

        m_properties.emplace_back(propertyInfo);
        OnPropertyAdded(pProperty);

        return *pProperty;
    }

    void PropertyContainer::ClearContainer() { /// NOLINT
        SR_TRACY_ZONE;

        for (auto&& propertyInfo : m_properties) {
            if (propertyInfo.isExternal) {
                continue; /// внешние свойства не удаляем
            }

            if (auto&& pContainer = dynamic_cast<PropertyContainer*>(propertyInfo.pProperty)) {
                pContainer->ClearContainer();
            }
            delete propertyInfo.pProperty;
        }

        m_properties.clear();
    }

    void PropertyContainer::SaveProperty(MarshalRef marshal) const noexcept {
        SR_TRACY_ZONE;

        if (auto&& pBlock = AllocatePropertyBlock()) {
            SR_HTYPES_NS::Marshal propertiesMarshal;
            uint16_t count = 0;

            for (auto&& propertyInfo : GetProperties()) {
                if (propertyInfo.pProperty->IsDontSave()) {
                    continue;
                }

                if (!propertyInfo.pProperty->IsActive()) {
                    continue;
                }

                ++count;

                SR_HTYPES_NS::Marshal propertyMarshal;
                propertyInfo.pProperty->SaveProperty(propertyMarshal);

                propertiesMarshal.Write<StringAtom>(propertyInfo.pProperty->GetName());
                propertiesMarshal.Write<uint32_t>(propertyMarshal.Size());
                propertiesMarshal.Append(std::move(propertyMarshal));
            }

            pBlock->Write<uint16_t>(count);
            pBlock->Append(std::move(propertiesMarshal));

            SavePropertyBase(marshal, std::move(pBlock));
        }
    }

    EntityRefProperty& PropertyContainer::AddEntityRefProperty(SR_UTILS_NS::StringAtom name, const EntityRefUtils::OwnerRef& owner) {
        if (auto&& pProperty = Find(name)) {
            SRHalt("Properties::AddEntityRefProperty() : property \"" + name.ToStringRef() + "\" already exists!");
            return *dynamic_cast<EntityRefProperty*>(pProperty);
        }

        auto&& pProperty = new EntityRefProperty();

        pProperty->SetName(name);
        pProperty->GetEntityRef().SetOwner(owner);

        PropertyInfo propertyInfo;
        propertyInfo.pProperty = pProperty;
        m_properties.emplace_back(propertyInfo);

        OnPropertyAdded(pProperty);

        return *pProperty;
    }

    void PropertyContainer::LoadProperty(MarshalRef marshal) noexcept {
        if (auto&& pBlock = LoadPropertyBase(marshal)) {
            auto&& count = pBlock->Read<uint16_t>();

            for (uint16_t i = 0; i < count; ++i) {
                auto&& name = pBlock->Read<StringAtom>();
                auto&& size = pBlock->Read<uint32_t>();

                SR_TRACY_ZONE;
                SR_TRACY_ZONE_TEXT(name.ToStringRef());

                auto&& propertyMarshal = pBlock->ReadBytes(size);

                if (auto&& pProperty = Find(name)) {
                    pProperty->LoadProperty(propertyMarshal);
                }
                else {
                    SR_WARN("PropertyContainer::LoadProperty() : property not found!\n\tContainer: {}\n\tProperty name: {}",
                        GetName().ToCStr(), name.ToCStr()
                    );
                }
            }
        }
    }

    ArrayReferenceProperty& PropertyContainer::AddArrayReferenceProperty(const char* name) {
        return AddCustomProperty<ArrayReferenceProperty>(name);
    }

    void PropertyContainer::AddExternalProperty(Property* pProperty) {
        PropertyInfo propertyInfo;
        propertyInfo.pProperty = pProperty;
        propertyInfo.isExternal = true;
        m_properties.emplace_back(propertyInfo);
    }
}
