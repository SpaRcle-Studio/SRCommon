//
// Created by Monika on 11.07.2025.
//

#ifndef SR_ENGINE_COMMON_ASSET_H
#define SR_ENGINE_COMMON_ASSET_H

#include <Utils/Resources/IResource.h>
#include <Utils/Resources/ResourceManager.h>

namespace SR_UTILS_NS {
    /// @extension(sras)
    class Asset : public IResource {
        SR_CLASS()
        using Super = IResource;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<Asset>;
        using OriginType = Asset;

    public:
        SR_NODISCARD bool SaveAsset(const Path& path) const;
        SR_NODISCARD bool SaveAsset() const;

        SR_NODISCARD static SR_HTYPES_NS::SharedPtr<Asset> CreateNew(const Path& path, SR_UTILS_NS::StringAtom assetType);

        template<class AssetT = Asset> SR_NODISCARD static SR_HTYPES_NS::SharedPtr<AssetT> Load(const Path& path);
        template<class AssetT> SR_NODISCARD static SR_HTYPES_NS::SharedPtr<AssetT> CreateNew(const Path& path);
        template<class AssetT> SR_NODISCARD static SR_HTYPES_NS::SharedPtr<AssetT> LoadOrCreate(const Path& path);

    protected:
        virtual void OnAssetLoaded() { }

    private:
        SR_NODISCARD static Asset::Ptr LoadImpl(const Path& path);

    protected:
        bool Load() override;
        bool Unload() override;

    };

    template<class AssetT> SR_HTYPES_NS::SharedPtr<AssetT> Asset::Load(const Path& path) {
        if constexpr (!std::is_base_of_v<Asset, AssetT>) {
            static_assert(AlwaysFalseV<AssetT>, "AssetT must be derived from Asset!");
        }
        else {
            auto&& pAsset = Asset::LoadImpl(path);
            if (!pAsset) {
                SR_ERROR("Asset::Load() : failed to load asset from path: {}", path);
                return nullptr;
            }

            if constexpr (std::is_same_v<AssetT, Asset>) {
                return pAsset;
            }

            auto&& pImpl = DynamicPointerCast<AssetT>(pAsset);
            if (!pImpl) {
                SRHalt("Asset::Load() : failed to cast asset to type: {}\n\tPath:", AssetT::GetClassStaticName(), path);
                pImpl->CheckResourceUsage();
                return nullptr;
            }

            return pImpl;
        }
    }

    template<class AssetT> SR_HTYPES_NS::SharedPtr<AssetT> Asset::LoadOrCreate(const Path& rawPath) {
        auto&& resourceManager = ResourceManager::Instance();
        SR_UTILS_NS::Path&& path = rawPath.RemoveSubPath(resourceManager.GetResPath());

        if (resourceManager.GetResPath().Concat(path).Exists(Path::Type::File)) {
            return Asset::template Load<AssetT>(path);
        }
        else {
            return Asset::template CreateNew<AssetT>(path);
        }
    }

    template<class AssetT> SR_HTYPES_NS::SharedPtr<AssetT> Asset::CreateNew(const Path& rawPath) {
        if constexpr (!std::is_base_of_v<Asset, AssetT>) {
            static_assert(AlwaysFalseV<AssetT>, "AssetT must be derived from Asset!");
        }
        else {
            return StaticPointerCast<AssetT>(CreateNew(rawPath, AssetT::GetClassStaticName()));
        }
    }
}

#endif //SR_ENGINE_COMMON_ASSET_H
