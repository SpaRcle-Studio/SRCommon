//
// Created by innerviewer on 2024-04-29.
//
#ifndef SR_COMMON_SCENE_CHUNK_LOGIC_H
#define SR_COMMON_SCENE_CHUNK_LOGIC_H

#include <Utils/World/SceneLogic.h>
#include <Utils/World/ChunkTicket.h>
#include <Utils/World/ChunkInfo.h>

namespace SR_WORLD_NS {
    class SceneChunkLogic : public SceneLogic {
        using Super = SceneLogic;
        using RegionChunkPositions = std::pair<SR_MATH_NS::IVector3, SR_MATH_NS::IVector3>;
    public:
        explicit SceneChunkLogic(const ScenePtr& scene);
        ~SceneChunkLogic() override;

    public:
        bool Init() override;
        bool Initialize();
        bool InitializeRegions();
        bool InitializeChunks(Region* pRegion);

        bool LoadObserverScope();

        void UpdateChunk(const SR_MATH_NS::IVector3& chunkPosition, ChunkTicket ticket);

        SR_NODISCARD bool ScopeCheckFunction(const SR_MATH_NS::IVector3& position) const;

        void PostLoad() override;

        void Update(float_t dt) override;
        void UpdateContainers();

        bool Reload() override;

        bool Load(const Path& path) override;


        SR_NODISCARD SR_MATH_NS::IVector3 CalculateCurrentChunk() const;
        //bool ReloadChunks();

    public:
        SR_NODISCARD Path GetRegionsPath() const;
        SR_NODISCARD Region* GetRegion(const SR_MATH_NS::IVector3& region) const;
        SR_NODISCARD Region* GetOrLoadRegion(const SR_MATH_NS::IVector3& region);

        SR_NODISCARD Observer* GetObserver() const { return m_observer; }

        SR_NODISCARD Chunk* GetChunk(const SR_MATH_NS::IVector3 &region, const SR_MATH_NS::IVector3 &chunk) const;

        SR_NODISCARD const GameObjects& GetGameObjectsAtChunk(const SR_MATH_NS::IVector3& region, const SR_MATH_NS::IVector3& chunk) const;
        std::pair<SR_MATH_NS::IVector3, SR_MATH_NS::IVector3> GetRegionAndChunk(const SR_MATH_NS::FVector3& pos) const;

    public:
        void SetObserver(const GameObject::Ptr& target);

    private:
        bool LoadConfig();

    private:
        Observer* m_observer = nullptr;
        Chunk* m_currentChunk = nullptr;

        std::vector<Region*> m_regions;
        std::vector<Chunk*> m_chunks;

        Tensor m_tensor;

        ChunkDimensionInfo m_defaultDimensions;
        uint32_t m_regionWidth = 0;
    };
}

#endif //SR_COMMON_SCENE_CHUNK_LOGIC_H
