//
// Created by innerviewer on 1/6/2024.
//

#ifndef SR_ENGINE_LAYER_MANAGER_H
#define SR_ENGINE_LAYER_MANAGER_H

#include <Utils/Resources/Asset.h>

namespace SR_UTILS_NS {
    struct RenderLayerInfo : public Serializable {
        SR_STRUCT()

        /// @property
        StringAtom name;

        /// @property
        bool isCustom = false;
        /// @property
        bool noPostProcess = false;
        /// @property
        bool mainRenderer = false;
        /// @property
        bool editorOnly = false;
        /// @property
        bool castShadows = false;
        /// @property
        bool applyShadows = false;
        /// @property
        bool colorBuffer = false;
        /// @property
        bool frustumCulling = false;
        /// @property
        bool clearDepth = false;

        SR_NODISCARD bool CompareParams(const RenderLayerInfo& other) const noexcept {
            return isCustom == other.isCustom &&
                noPostProcess == other.noPostProcess &&
                mainRenderer == other.mainRenderer &&
                editorOnly == other.editorOnly &&
                castShadows == other.castShadows &&
                applyShadows == other.applyShadows &&
                colorBuffer == other.colorBuffer &&
                frustumCulling == other.frustumCulling &&
                clearDepth == other.clearDepth;
        }
    };

    class LayerManagerSettings final : public SR_UTILS_NS::Asset {
        SR_CLASS()
        using Super = SR_UTILS_NS::Asset;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<LayerManagerSettings>;

        /// @property
        SR_UTILS_NS::Vector<RenderLayerInfo> layers;
        /// @property
        StringAtom defaultLayer = "Default";
    };

    class LayerManager : public Singleton<LayerManager> {
        SR_REGISTER_SINGLETON(LayerManager);
    public:
        SR_NODISCARD bool HasLayer(StringAtom layer) const;
        SR_NODISCARD uint16_t GetLayerIndex(StringAtom layer) const;
        SR_NODISCARD SR_UTILS_NS::Vector<StringAtom> GetLayers() const { return m_layers; }
        SR_NODISCARD SR_UTILS_NS::Vector<RenderLayerInfo> GetLayersInfo() const { return m_layersInfo; }

        SR_NODISCARD static StringAtom GetDefaultLayer();
        SR_NODISCARD uint64_t GetHashState() const { return m_hashState; }

    private:
        void InitSingleton() override;
        void OnSingletonDestroy() override;
        void ReloadSettings();

    private:
        LayerManagerSettings::Ptr m_pSettings;
        SR_UTILS_NS::Subscription m_onSettingsReloaded;
        SR_UTILS_NS::Vector<RenderLayerInfo> m_layersInfo;
        SR_UTILS_NS::Vector<StringAtom> m_layers;
        std::atomic<StringAtom> m_defaultLayer;
        uint64_t m_hashState = 0;

    };
}
#endif //SR_ENGINE_LAYERMANAGER_H
