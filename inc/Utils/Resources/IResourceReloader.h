//
// Created by Monika on 11.03.2023.
//

#ifndef SR_ENGINE_IRESOURCERELOADER_H
#define SR_ENGINE_IRESOURCERELOADER_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/Types/SharedPtr.h>

namespace SR_UTILS_NS {
    struct ResourcesStorage;
    class Path;
    class IResource;

    class SR_COMMON_DLL_API IResourceReloader : public SR_UTILS_NS::NonCopyable {
    public:
        SR_NODISCARD virtual bool IsResourceSuitableForReload(const IResource& resource) const;
        SR_NODISCARD virtual bool Reload(const SR_UTILS_NS::Path& path, ResourcesStorage* pStorage) = 0;

    };

    /** Обычная перезагрузка, перебором каждого ресурса и вызова ему Reload */
    class SR_COMMON_DLL_API DefaultResourceReloader final : public IResourceReloader {
    public:
        SR_NODISCARD bool Reload(const SR_UTILS_NS::Path& path, ResourcesStorage* pStorage) override;

    };
}

#endif //SR_ENGINE_IRESOURCERELOADER_H
