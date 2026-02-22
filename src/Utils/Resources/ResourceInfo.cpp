//
// Created by Monika on 02.05.2022.
//

#include <Utils/Resources/ResourceInfo.h>
#include <Utils/Resources/IResourceReloader.h>
#include <Utils/Resources/ResourceManager.h>

namespace SR_UTILS_NS {
    IResource::Ptr ResourceType::Find(ResourceId id, const IResourceVariant* pVariant) const {
        if (auto&& pIt = m_storage.find(id); pIt != m_storage.end()) {
            return pIt->second->Find(pVariant);
        }

        return nullptr;
    }

    bool ResourceType::IsLast(ResourceId id, const IResourceVariant* pVariant) {
        if (auto&& pIt = m_storage.find(id); pIt == m_storage.end()) {
            return true;
        }
        else {
            return pIt->second->IsLast();
        }
    }

    void ResourceType::Remove(const IResource::Ptr& pResource) {
        if (auto&& pIt = m_storage.find(pResource->GetResourceId()); pIt != m_storage.end()) {
            pIt->second->Remove(pResource);
            if (m_count > 0) {
                m_count--;
            }
            else {
                SRHalt("ResourceType::Remove() : resource count is already zero!");
            }
        }
        else {
            SRHalt("ResourceType::Remove() : resource id not found in storage!\n\tType: {}\n\tId: {}",
                   pResource->GetResourceType(), pResource->GetResourceId());
        }
    }

    void ResourceType::Add(const IResource::Ptr& pResource) {
        if (pResource->GetResourcePath().empty()) {
            SRHalt("ResourceType::Add() : resource has empty path!");
            return;
        }

        auto&& pStorageRef = m_storage[pResource->GetResourceId()];
        if (!pStorageRef) {
            pStorageRef = new ResourcesStorage();
            pStorageRef->path = StringAtom(pResource->GetResourcePath().ToStringView());
            pStorageRef->id = pResource->GetResourceId();
        }
        else if (pStorageRef->path != pResource->GetResourcePath()) {
            SRHalt("ResourceType::Add() : resource path mismatch!\n\tExpected: {}\n\tActual: {}", pStorageRef->path, pResource->GetResourcePath());
            return;
        }

        pStorageRef->Add(pResource);
        pResource->OnResourceRegistered();

        m_count++;
    }

    void ResourceType::CollectUnused() {
        SR_TRACY_ZONE;
        for (auto&& [resourceId, pStorage] : m_storage) {
            pStorage->CollectUnused();
        }
    }

    void ResourceType::SetReloader(IResourceReloader* pReloader) {
        m_reloader = pReloader;
    }

    void ResourceType::ReloadAll() {
        SR_TRACY_ZONE;
        for (auto&& [resourceId, pStorage] : m_storage) {
            m_dirtyResources.insert(resourceId);
        }
    }

    void ResourceType::Reload(const IResource::Ptr& pResource) {
        SR_TRACY_ZONE;
        if (m_dirtyResources.contains(pResource->GetResourceId())) {
            return;
        }
        SR_LOG("ResourceType::Reload() : resource \"{}\" marked as dirty!", pResource->GetResourceId());
        m_dirtyResources.insert(pResource->GetResourceId());
    }

    void ResourceType::ReloadDirtyResources() {
        SR_TRACY_ZONE;
        IResourceReloader* pReloader = m_reloader.Get() ? m_reloader.Get() : ResourceManager::Instance().GetDefaultReloader();
        if (!pReloader) {
            SRHalt("ResourceType::ReloadDirtyResources() : no reloader set for resource type \"{}\"!", GetName());
            return;
        }
        for (auto&& resourceId : m_dirtyResources) {
            if (auto&& pIt = m_storage.find(resourceId); pIt != m_storage.end()) {
                if (!pReloader->Reload(pIt->second->path, pIt->second)) {
                    SR_ERROR("ResourceType::ReloadDirtyResources() : failed to reload resource!\n\tPath: {}", pIt->second->path);
                }
            }
        }
        m_dirtyResources.clear();
    }

    void ResourceType::ForEach(const SR_HTYPES_NS::Function<void(const IResource&)>& fun) const {
        for (auto&& [resourceId, pStorage] : m_storage) {
            pStorage->ForEach(fun);
        }
    }

    void ResourceType::ForEach(const SR_HTYPES_NS::Function<void(IResource&)>& fun) {
        for (auto&& [resourceId, pStorage] : m_storage) {
            pStorage->ForEach(fun);
        }
    }

    void ResourcesStorage::ForEach(const SR_HTYPES_NS::Function<void(IResource&)> &fun) {
        SR_TRACY_ZONE;

        for (auto&& [hash, container] : variants) {
            for (auto&& pResource : container) {
                if (!pResource) {
                    continue;
                }
                fun(*pResource);
            }
        }
    }

    void ResourcesStorage::Add(const IResource::Ptr& pResource) {
        SR_TRACY_ZONE;

        const IResourceVariant* pVariant = pResource->GetVariant();
        uint64_t variantHash = pVariant ? pVariant->GetHash() : 0;

        auto&& pIt = variants.find(variantHash);
        if (pIt != variants.end()) {
            pIt->second.emplace_back(pResource);
        }
        else {
            variants[variantHash] = { pResource };
        }
    }

    void ResourcesStorage::Remove(const IResource::Ptr& pResource) {
        SR_TRACY_ZONE;

        const IResourceVariant* pVariant = pResource->GetVariant();
        uint64_t variantHash = pVariant ? pVariant->GetHash() : 0;

        if (auto&& pVariantIt = variants.find(variantHash); pVariantIt != variants.end()) {
            auto&& container = pVariantIt->second;
            if (auto&& pResIt = std::ranges::find(container, pResource); pResIt != container.end()) {
                container.erase(pResIt);
                if (container.empty()) {
                    variants.erase(pVariantIt);
                }
            }
            else {
                SRHalt("Resource not found in storage!");
            }
        }
        else {
            SRHalt("Resource not found in storage!");
        }
    }

    void ResourcesStorage::CollectUnused() {
        SR_TRACY_ZONE;

        for (auto&& [variantId, container] : variants) {
            for (auto&& pResource : container) {
                pResource->Execute([pResource]() -> bool {
                    if (pResource->GetCountUses() > 0) {
                        return false;
                    }

                    if (pResource->IsDestroyed()) {
                        return false;
                    }

                    pResource->Destroy();

                    return true;
                });
            }
        }
    }

    bool ResourcesStorage::IsLast() const {
        bool hasOne = false;
        for (auto&& [variantId, container] : variants) {
            if (!container.empty()) {
                if (hasOne) {
                    return false;
                }
                hasOne = true;
            }
        }
        return true;
    }

    IResource::Ptr ResourcesStorage::Find(const IResourceVariant* pVariant) const {
        uint64_t variantHash = pVariant ? pVariant->GetHash() : 0;
        if (auto&& pVariantIt = variants.find(variantHash); pVariantIt != variants.end()) {
            auto&& container = pVariantIt->second;
            for (auto&& pResource : container) {
                if (SR_UTILS_NS::ResourceManager::Instance().ReviveResource(pResource)) {
                    return pResource;
                }
            }
        }
        return nullptr;
    }
}