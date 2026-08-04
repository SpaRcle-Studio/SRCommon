//
// Created by Monika on 11.07.2025.
//

#include <Utils/Resources/Asset.h>
#include <Utils/Serialization/SRASerialization.h>
#include <Utils/Serialization/Serializable.h>

#include <Codegen/Asset.generated.hpp>

namespace SR_UTILS_NS {
    Asset::Ptr Asset::LoadImpl(const Path& rawPath) {
        SR_TRACY_ZONE;
        SR_TRACY_ZONE_TEXT_C(rawPath.c_str());
        SR_GLOBAL_RECURSIVE_LOCK;

        auto&& resourceManager = ResourceManager::Instance();
        SR_UTILS_NS::Path path = rawPath.RemoveSubPath(resourceManager.GetResPath());

        Asset::Ptr pAsset = DynamicPointerCast<Asset>(resourceManager.FindAnyType(SR_UTILS_NS::StringAtom(path.View()), nullptr));
        if (pAsset) {
            return pAsset;
        }

        SR_LOG("Asset::LoadImpl() : loading asset \"{}\"", path);

        SR_UTILS_NS::SRADeserializer deserializer;
        if (!deserializer.LoadFromFile(resourceManager.GetResPath().Concat(path))) {
            SR_ERROR("Asset::LoadImpl() : failed to deserialize asset from file!\n\tPath: {}", path);
            return nullptr;
        }

        if (!Serialization::Load(deserializer, pAsset, SerializationId::Create("asset"))) {
            SR_ERROR("Asset::LoadImpl() : failed to load asset from deserializer!\n\tPath: {}", path);
            return nullptr;
        }

        if (!pAsset) {
            SR_ERROR("Asset::LoadImpl() : asset is null after deserialization!\n\tPath: {}", path);
            return nullptr;
        }

        pAsset->m_loadState = IResource::LoadState::Loaded;
        pAsset->SetId(SR_UTILS_NS::StringAtom(path.View()), true);
        pAsset->OnAssetLoaded();

        return pAsset;
    }

    bool Asset::SaveAsset(const Path& rawPath) const {
        auto&& resourceManager = ResourceManager::Instance();
        SR_UTILS_NS::Path&& path = rawPath.RemoveSubPath(resourceManager.GetResPath());

        SR_UTILS_NS::SRASerializer serializer;
        serializer.SetUseTabs(true);

        Asset::Ptr pThis = StaticPointerCast<Asset>(GetThis());
        Serialization::Save(serializer, pThis, SerializationId::Create("asset"));

        if (!serializer.SaveToFile(resourceManager.GetResPath().Concat(path))) {
            SR_ERROR("Asset::SaveAsset() : failed to save asset to file!\n\tPath: {}", path);
            return false;
        }

        return true;
    }

    bool Asset::Load() {
        SR_UTILS_NS::SRADeserializer deserializer;
        if (!deserializer.LoadFromFile(ResourceManager::Instance().GetResPath().Concat(GetResourcePath()))) {
            SR_ERROR("Asset::Load() : failed to deserialize asset from file!\n\tPath: {}", GetResourcePath());
            return false;
        }

        if (deserializer.BeginObject(SerializationId::Create("asset"))) {
            const bool success = Serialization::Load(deserializer, *this, SerializationId::Create("ptr"));
            deserializer.EndObject();
            if (!success) {
                SR_ERROR("Asset::Load() : failed to load asset from deserializer!\n\tPath: {}", GetResourcePath());
                return false;
            }
        }

        OnAssetLoaded();

        return Super::Load();
    }

    bool Asset::Unload() {
        GetMeta()->ForEachProperty([&](auto&& property, uint64_t index) {
            auto&& defaultValue = property.GetDefaultValue().Ref();
            property.Set(this, defaultValue);
        });

        OnAssetUnloaded();

        return Super::Unload();
    }

    bool Asset::SaveAsset() const {
        return SaveAsset(SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat(GetResourcePath()));
    }

    SR_HTYPES_NS::SharedPtr<Asset> Asset::CreateNew(const Path& rawPath, StringAtom assetType) {
        auto&& resourceManager = ResourceManager::Instance();
        SR_UTILS_NS::Path&& path = rawPath.RemoveSubPath(resourceManager.GetResPath());

        if (resourceManager.GetResPath().Concat(path).Exists()) {
            SR_ERROR("Asset::CreateNew() : asset already exists at path: {}", path);
            return nullptr;
        }

        SR_LOG("Asset::CreateNew() : creating new asset {} at path: {}", assetType, path);

        SR_HTYPES_NS::SharedPtr<Asset> pAsset = Factory::Instance().Create<Asset>(assetType);
        pAsset->m_loadState = IResource::LoadState::Loaded;

        if (!pAsset->SaveAsset(resourceManager.GetResPath().Concat(path))) {
            SR_ERROR("Asset::CreateNew() : failed to save asset to path: {}", path);
            pAsset->DeleteResource();
            return nullptr;
        }

        pAsset->SetId(StringAtom(path.View()), true);
        return pAsset;
    }
}