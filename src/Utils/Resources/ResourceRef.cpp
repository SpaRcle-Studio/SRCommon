//
// Created by Monika on 19.02.2026.
//

#include <Utils/Resources/ResourceRef.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/Resources/Asset.h>

#include <Codegen/ResourceRef.generated.hpp>

namespace SR_UTILS_NS {
    ResourceRefBase::ResourceRefBase(const ResourceRefBase& other)
        : Super(other)
    {
        m_id = other.m_id;
        if (m_resource) {
            m_resource->RemoveUsePoint();
        }
        m_resource = other.m_resource;
        if (m_resource) {
            m_resource->AddUsePoint();
        }
    }

    ResourceRefBase::ResourceRefBase(ResourceRefBase&& other) noexcept {
        m_id = SR_EXCHANGE(other.m_id, StringAtom());
        if (m_resource) {
            m_resource->RemoveUsePoint();
        }
        m_resource = SR_EXCHANGE(other.m_resource, {});
        if (m_resource) {
            m_resource->AddUsePoint();
        }
    }

    ResourceRefBase& ResourceRefBase::operator=(const ResourceRefBase& other) {
        if (this != &other) {
            m_id = other.m_id;
            if (m_resource) {
                m_resource->RemoveUsePoint();
            }
            m_resource = other.m_resource;
            if (m_resource) {
                m_resource->AddUsePoint();
            }
        }
        return *this;
    }

    ResourceRefBase& ResourceRefBase::operator=(ResourceRefBase&& other) noexcept {
        if (this != &other) {
            m_id = SR_EXCHANGE(other.m_id, StringAtom());
            if (m_resource) {
                m_resource->RemoveUsePoint();
            }
            m_resource = SR_EXCHANGE(other.m_resource, {});
            if (m_resource) {
                m_resource->AddUsePoint();
            }
        }
        return *this;
    }

    ResourceRefBase::~ResourceRefBase() {
        if (m_resource) {
            m_resource->RemoveUsePoint();
        }
    }

    IResource::Ptr ResourceRefBase::LoadResource(StringAtom type, StringAtom id) {
        SR_TRACY_ZONE;

        if (SR_UTILS_NS::Factory::Instance().HasBaseClass(type, Asset::GetClassStaticName())) {
            type = Asset::GetClassStaticName();
        }

        auto&& pResource = ResourceManager::Instance().LoadResource(id, type, nullptr);
        if (!pResource) {
            SR_ERROR("ResourceRefBase::LoadResource() : failed to load resource!\n\tType: {}\n\tId: {}", type, id);
            return {};
        }
        pResource->AddUsePoint();
        return pResource;
    }

    void ResourceRefBase::SetResource(StringAtom id) {
        if (m_id == id) {
            return;
        }

        m_id = id;

        if (m_resource) {
            m_resource->RemoveUsePoint();
        }

        m_resource = LoadResource(GetResourceType(), id);
    }

    StringAtom ResourceRefBase::GetExtension() const noexcept {
        if (auto&& pMeta = Factory::Instance().GetType(GetResourceType())) {
            return pMeta->GetExtension();
        }
        SRHalt("ResourceRefBase::GetExtension() : resource type not found!\n\tType: {}", GetResourceType());
        return {};
    }

    void ResourceRefBase::OnPostLoad() {
        if (m_resource) {
            m_resource->RemoveUsePoint();
        }
        m_resource = LoadResource(GetResourceType(), m_id);
        Super::OnPostLoad();
    }

    void ResourceRefBase::CloneTo(SRClass& clone) const {
        Super::CloneTo(clone);
        if (m_resource) {
            m_resource->AddUsePoint();
        }
        if (auto&& pResource = static_cast<ResourceRefBase&>(clone).m_resource) {
            pResource->RemoveUsePoint();
        }
        static_cast<ResourceRefBase&>(clone).m_resource = m_resource;
    }
}