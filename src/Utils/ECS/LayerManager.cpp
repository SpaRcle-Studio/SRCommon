//
// Created by innerviewer on 1/6/2024.
//

#include <Utils/ECS/LayerManager.h>
#include <Utils/Resources/IResource.h>

#include <Codegen/LayerManager.generated.hpp>

namespace SR_UTILS_NS {
    static SR_UTILS_NS::StringAtom LAYER_MANAGER_SETTINGS_PATH = "Engine/Configs/Layers.sras";

    uint16_t LayerManager::GetLayerIndex(StringAtom layer) const {
        for (SizeType i = 0; i < m_layers.size(); ++i) {
            if (layer == m_layers[i]) {
                return i;
            }
        }

        return SR_ID_INVALID;
    }

    bool LayerManager::HasLayer(StringAtom layer) const {
        for (SizeType i = 0; i < m_layers.size(); ++i) {
            if (layer == m_layers[i]) {
                return true;
            }
        }

        return false;
    }

    StringAtom LayerManager::GetDefaultLayer() {
        return LayerManager::Instance().m_defaultLayer;
    }

    void LayerManager::InitSingleton() {
        SR_LOG("LayerManager::InitSingleton() : initializing layer manager...");

        m_pSettings = CoreResLoader::Load<LayerManagerSettings>(LAYER_MANAGER_SETTINGS_PATH);
        if (!m_pSettings) {
            SR_ERROR("LayerManager::InitSingleton() : failed to load settings! Path: {}!", LAYER_MANAGER_SETTINGS_PATH);
            m_defaultLayer = "Default";
            return;
        }
        m_pSettings->AddUsePoint();

        LayerManager::Instance().ReloadSettings();

        m_onSettingsReloaded = m_pSettings->Subscribe(SR_UTILS_NS::IResource::RELOAD_DONE_EVENT, [](const SR_UTILS_NS::SubscriptionMessage&) {
            LayerManager::Instance().ReloadSettings();
        });
    }

    void LayerManager::ReloadSettings() {
        SR_LOCK_GUARD;

        SR_INFO("LayerManager::ReloadSettings() : reloading settings...");

        m_defaultLayer = StringAtom();
        m_layers.clear();
        m_layersInfo.clear();

        m_defaultLayer = m_pSettings->defaultLayer;
        SRAssert2(!m_defaultLayer.load().Empty(), "Default layer is not set");

        for (auto&& layerInfo : m_pSettings->layers) {
            if (layerInfo.name.Empty()) {
                SR_ERROR("LayerManager::ReloadSettings() : layer name is empty! Skipping layer...");
                continue;
            }

            m_layersInfo.emplace_back(layerInfo);
            if (layerInfo.isCustom) {
                continue;
            }

            m_layers.emplace_back(layerInfo.name);
        }

        m_hashState = m_defaultLayer.load().GetHash();

        for (auto&& layer : m_layers) {
            m_hashState = SR_COMBINE_HASHES(m_hashState, layer.GetHash());
        }

        SR_LOG("LayerManager::ReloadSettings() : settings reloaded! Default layer: {}, Layers count: {}.", m_defaultLayer.load(), m_layers.size());
    }

    void LayerManager::OnSingletonDestroy() {
        m_onSettingsReloaded.Reset();
        m_pSettings->RemoveUsePoint();
        m_pSettings = nullptr;
    }
}