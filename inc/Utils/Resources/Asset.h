//
// Created by Monika on 11.07.2025.
//

#ifndef SR_ENGINE_COMMON_ASSET_H
#define SR_ENGINE_COMMON_ASSET_H

#include <Utils/Serialization/Serializable.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS {
    class Asset : public SR_UTILS_NS::IResource {
        SR_CLASS()
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<Asset>;
        using OriginType = Asset;

    public:
        SR_NODISCARD static Asset::Ptr Load(const SR_UTILS_NS::Path& path);

    };
}

#endif //SR_ENGINE_COMMON_ASSET_H
