//
// Created by Monika on 11.03.2023.
//

#ifndef SR_ENGINE_IRESOURCERELOADER_H
#define SR_ENGINE_IRESOURCERELOADER_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS {
    class ResourceInfo;
    class IResource;

    class SR_COMMON_DLL_API IResourceReloader : public SR_UTILS_NS::NonCopyable {
    public:
        SR_NODISCARD virtual bool IsResourceSuitableForReload(const IResource::Ptr& pResource) const;
        SR_NODISCARD virtual bool Reload(const SR_UTILS_NS::Path& path, ResourceInfo* pResourceInfo) = 0;

    protected:
        std::recursive_mutex m_mutex;

    };

    /** Обычная перезагрузка, перебором каждого ресурса и вызова ему Reload */
    class SR_COMMON_DLL_API DefaultResourceReloader final : public IResourceReloader {
    public:
        SR_NODISCARD bool Reload(const SR_UTILS_NS::Path& path, ResourceInfo* pResourceInfo) override;

    };
}

#endif //SR_ENGINE_IRESOURCERELOADER_H
