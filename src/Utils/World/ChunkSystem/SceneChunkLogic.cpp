//
// Created by innerviewer on 2024-04-29.
//

#include <Utils/World/ChunkSystem/SceneChunkLogic.h>
#include <Utils/World/ChunkSystem/Observer.h>
#include <Utils/World/ChunkSystem/Region.h>

#include <Utils/ECS/ComponentManager.h>

#include <Utils/Resources/ResourceManager.h>

#include <Utils/Types/Marshal.h>

namespace SR_WORLD_NS {
    SceneChunkLogic::SceneChunkLogic(const ScenePtr& scene)
            : Super(scene)
            , m_observer(new Observer(scene))
    { }

    SceneChunkLogic::~SceneChunkLogic() {
        SR_SAFE_DELETE_PTR(m_observer);
    }

    bool SceneChunkLogic::Init() {
        if (!LoadConfig()) {
            SR_ERROR("SceneChunkLogic::Init() : failed to load config!");
            return false;
        }


        /// Initial load/creation of all observable regions
/*        ChunkTicket ticket;
        ticket.type = ChunkTicketType::Start;
        ticket.level = 5;

        auto&& reg1 = GetOrLoadRegion({-1, -1, -1 });
        auto&& chunk1 = reg1->InitializeChunk({1, 1, 1}, ticket);
        m_chunks.emplace_back(chunk1);

        if (!InitializeRegions()) {
            SR_ERROR("SceneChunkLogic::Init() : failed to initialize regions!");
            return false;
        }*/

/*
        if (!InitializeChunks()) {
            SR_ERROR("SceneChunkLogic::Init() : failed to initialize chunks!");
            return false;
        }
*/

        return true;
    }

    bool SceneChunkLogic::Initialize() {
        if (!m_observer) {
            SR_ERROR("SceneChunkLogic::InitializeRegions() : m_observer is nullptr!");
            return false;
        }

        return true;
    }

    void SceneChunkLogic::Update(float_t dt) {
        /// TODO: change default dimensions.
        auto&& chunkSize = Math::IVector3(m_defaultDimensions.dimension.cube);
        auto&& regSize = Math::IVector3(m_regionWidth);
        auto&& regSize2 = Math::IVector3(m_regionWidth - 1);

        auto&& offset = m_observer->m_offset;

        if (m_observer->m_target) {
            auto&& pTransform = m_observer->m_target->GetRoot()->GetTransform();
            m_observer->m_targetPosition = pTransform->GetTranslation().Singular(chunkSize.Cast<Math::Unit>());
        }

        auto&& lastChunk = m_observer->m_lastChunk;
        auto&& lastRegion = m_observer->m_lastRegion;

        auto&& currentChunk = CalculateCurrentChunk();

        if (lastChunk != currentChunk) {
            if (auto&& pRegion = GetRegion(lastRegion)) {
                auto&& pChunk = pRegion->GetChunk(m_observer->m_chunk);

                SRAssert(pChunk == m_currentChunk);
                SRAssert(pChunk);

                if (pChunk) {
                    pChunk->OnExit();
                }

                m_currentChunk = nullptr;
            }

            m_observer->SetChunk(currentChunk);

            auto region = AddOffset(currentChunk.Singular(regSize2) / regSize, -offset.m_region);

            if (auto regionDelta = (region - lastRegion); !regionDelta.Empty()) {
                m_observer->MoveRegion(regionDelta);
                SRAssert(!m_observer->m_region.HasZero());
            }

            auto&& pRegion = GetRegion(region);
            if (!pRegion) {
                pRegion = Region::Allocate(m_observer, m_regionWidth, m_defaultDimensions, m_observer->m_region);
                pRegion->Load();
                m_regions.emplace_back(pRegion);
            }

            if (pRegion) {
                if (auto&& pChunk = pRegion->GetChunk(m_observer->m_chunk)) {
                    SRAssert(!m_currentChunk);

                    m_currentChunk = pChunk;

                    pChunk->OnEnter();
                    m_debugDirty = true;
                }
            }

            lastRegion = region;
            lastChunk = currentChunk;
        }

        UpdateContainers();

        UpdateScope(dt);

        CheckShift(m_observer->m_targetPosition.Cast<int>() / chunkSize);

        LoadObserverScope();

        for (auto&& pChunk : m_chunks) {
            auto&& ticket = pChunk->GetFirstTicket();
            if (ticket.type == ChunkTicketType::Start) {
                pChunk->Load();

                ChunkTicket observerTicket;
                observerTicket.type = ChunkTicketType::Observer;
                observerTicket.level = 1;
                pChunk->AddTicket(observerTicket);
            }
        }

        SceneLogic::Update(dt);
    }

    bool SceneChunkLogic::LoadObserverScope() {
        if (!m_observer) {
            SR_ERROR("SceneChunkLogic::LoadScope() : m_observer is nullptr!");
            return false;
        }

        ChunkTicket ticket;
        ticket.type = ChunkTicketType::Observer;
        ticket.level = 2;

        auto&& scope = m_observer->GetScope();
        for (int32_t x = -scope; x <= scope; ++x) {
            for (int32_t y = -scope; y <= scope; ++y) {
                for (int32_t z = -scope; z <= scope; ++z) {
                    if (!ScopeCheckFunction({ x, y, z })) {
                        continue;
                    }

                    UpdateChunk(SR_MATH_NS::IVector3(x, y, z), ticket);
                }
            }
        }

        /*auto&& observerRegion = m_observer->GetRegion();
        uint16_t regionScope = std::ceil(static_cast<float_t>(m_observer->GetScope()) / static_cast<float_t>(m_regionWidth));

        for (int32_t x = observerRegion.x - regionScope; x <= observerRegion.x + regionScope; ++x) {
            for (int32_t y = observerRegion.y - regionScope; y <= observerRegion.y + regionScope; ++y) {
                for (int32_t z = observerRegion.z -regionScope; z <= observerRegion.z + regionScope; ++z) {
                    SR_MATH_NS::IVector3 regionPosition = { x, y, z };
                    if (regionPosition.HasZero()) {
                        continue;
                    }


                }
            }
        }*/

        return true;
    }

    bool SceneChunkLogic::InitializeRegions() {
        if (!m_observer) {
            SR_ERROR("SceneChunkLogic::InitializeRegions() : m_observer is nullptr!");
            return false;
        }

        auto&& observerRegion = m_observer->GetRegion();

        for (int32_t x = -1; x <= 1; ++x) {
            for (int32_t y = -1; y <= 1; ++y) {
                for (int32_t z = -1; z <= 1; ++z) {
                    SR_MATH_NS::IVector3 regionPosition = { x, y, z };
                    if (regionPosition.HasZero()) {
                        continue;
                    }

                    auto&& pRegion = GetOrLoadRegion(regionPosition);
                    if (!pRegion) {
                        SR_ERROR("SceneChunkLogic::InitializeRegions() : failed to get or load region!");
                        return false;
                    }

                    if (!InitializeChunks(pRegion)) {
                        SR_ERROR("SceneChunkLogic::InitializeRegions() : failed to initialize chunks!");
                        return false;
                    }
                }
            }
        }

        return true;
    }

    bool SceneChunkLogic::InitializeChunks(Region* pRegion) {
        if (!m_observer) {
            SR_ERROR("SceneChunkLogic::InitializeChunks() : m_observer is nullptr!");
            return false;
        }

        ChunkTicket ticket;
        ticket.type = ChunkTicketType::Start;
        ticket.level = 5;

//        SR_MATH_NS::IVector3 defaultRegion = { -1, -1, -1 };
//        SR_MATH_NS::IVector3 defaultChunk = { 1, 1, 1 };

        auto&& scope = m_observer->GetScope();

        for (int32_t x = -scope; x <= scope; ++x) {
            for (int32_t y = -scope; y <= scope; ++y) {
                for (int32_t z = -scope; z <= scope; ++z) {
                    if (!ScopeCheckFunction({ x, y, z })) {
                        continue;
                    }

                    UpdateChunk(SR_MATH_NS::IVector3(x, y, z), ticket);
                }
            }
        }

        return true;
    }

    void SceneChunkLogic::UpdateChunk(const SR_MATH_NS::IVector3& chunkPosition, ChunkTicket ticket) {
        auto&& neighbour = m_observer->MathNeighbour(chunkPosition);
        auto&& pRegion = GetOrLoadRegion(neighbour.GetRegion());

        if (auto&& pChunk = pRegion->GetChunk(neighbour.GetChunk())) {
            pChunk->AddTicket(ticket);
        }
    }

    void SceneChunkLogic::UpdateContainers() {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        auto&& chunkSize = m_defaultDimensions.dimension.cube;

        const uint64_t reserved = m_tensor.size();
        m_tensor.clear();
        m_tensor.reserve(reserved);

        auto&& root = m_scene->GetRootGameObjects();

        for (GameObject::Ptr gameObject : root) {
            const SR_MATH_NS::FVector3 gmPosition = gameObject->GetTransform()->GetTranslation();

            if (!gmPosition.IsFinite() || gmPosition.ContainsNaN()) {
                continue;
            }

            auto chunk = AddOffset(SR_MATH_NS::IVector3(gmPosition.Singular(chunkSize.Cast<SR_MATH_NS::Unit>()) / chunkSize), -m_observer->m_offset.m_chunk);
            auto region = AddOffset(chunk.Singular(SR_MATH_NS::IVector3(m_regionWidth - 1)) / SR_MATH_NS::IVector3(m_regionWidth), -m_observer->m_offset.m_region);

            const TensorKey key = TensorKey(region, MakeCubeChunk(chunk, m_regionWidth));

            if (auto&& pIt = m_tensor.find(key); pIt != m_tensor.end()) {
                pIt->second.emplace_back(std::move(gameObject));
            }
            else {
                m_tensor[key].emplace_back(std::move(gameObject));

                if (GetOrLoadRegion(key.region)->GetChunk(key.chunk)) {
                    /// подгружаем чанк, чтобы объект не остался висеть в пустоте
                }
            }
        }
    }

    Chunk* SceneChunkLogic::GetChunk(const SR_MATH_NS::IVector3 &region, const SR_MATH_NS::IVector3 &chunk) const {
        return GetRegion(region)->GetChunk(chunk);

        /*if (auto&& pRegion = GetRegion(region)) {
            return pRegion->IsChunkLoaded(chunk);
        }

        return false;*/
    }

    bool SceneChunkLogic::Reload() {
        SR_TRACY_ZONE;

        if (!LoadConfig()) {
            SR_ERROR("SceneChunkLogic::Reload() : failed to reload config!");
            return false;
        }

/*        if (!ReloadChunks()) {
            SR_ERROR("SceneChunkLogic::Reload() : failed to reload chunks!");
            return false;
        }*/

        return true;
    }

    bool SceneChunkLogic::LoadConfig() {
        /// TODO: возможно лучше сохранять в саму сцену?
        const std::string path = ResourceManager::Instance().GetResPath().Concat("Engine/Configs/World.xml");
        if (auto xml = SR_XML_NS::Document::Load(path); xml.Valid()) {
            const auto& configs = xml.Root().GetNode("Configs");

            ///TODO: move this chunk type logic somewhere else.
            auto&& chunkType = configs.TryGetNode("ChunkType").TryGetAttribute("Value").ToString("Cube");
            if (chunkType == "Cube") {
                m_defaultDimensions.type = ChunkDimensionType::Cube;
                m_defaultDimensions.dimension.cube = SR_MATH_NS::IVector3(
                        configs.TryGetNode("DefaultCubeChunkWidth").TryGetAttribute("Value").ToInt(10),
                        configs.TryGetNode("DefaultCubeChunkHeight").TryGetAttribute("Value").ToInt(10),
                        configs.TryGetNode("DefaultCubeChunkDepth").TryGetAttribute("Value").ToInt(10)
                );
            }
            else {
                SR_ERROR("SceneChunkLogics::ReloadConfig() : unknown chunk type.");
                return false;
            }

            m_regionWidth = configs.TryGetNode("DefaultRegionWidth").TryGetAttribute("Value").ToInt(6);

            m_observer->SetWorldMetrics(m_defaultDimensions, m_regionWidth);
            m_observer->SetShiftDist(configs.TryGetNode("DefaultShiftDistance").TryGetAttribute("Value").ToInt(10));
            m_observer->SetScope(configs.TryGetNode("DefaultScope").TryGetAttribute("Value").ToInt(10));

            return true;
        }
        else {
            SR_ERROR("SceneChunkLogic::Scene() : file not found! Path: " + path);
            return false;
        }
    }

    /*bool SceneChunkLogic::ReloadChunks() {
        SRHalt("Not yet implemented!");
        return false;
    }*/

    bool SceneChunkLogic::Load(const Path &path) {
        auto&& componentsPath = m_scene->GetAbsPath().Concat("data/components.bin");

        if (auto&& rootComponentsMarshal = SR_HTYPES_NS::Marshal::LoadPtr(componentsPath)) {
            auto&& components = SR_UTILS_NS::ComponentManager::Instance().LoadComponents(*rootComponentsMarshal);
            delete rootComponentsMarshal;
            for (auto&& pComponent : components) {
                m_scene->AddComponent(pComponent);
            }
        }
        else {
            SR_ERROR("SceneChunkLogic::Load() : file not found!\n\tPath: " + componentsPath.ToString());
            return false;
        }

        return true;
    }

    void SceneChunkLogic::PostLoad() {
        for (auto&& pRegion : m_regions) {
            pRegion->PostLoad();
        }

        Super::PostLoad();
    }

    Path SceneChunkLogic::GetRegionsPath() const {
        /// TODO: cache path
        return m_scene->GetAbsPath().Concat("regions");
    }

    SR_NODISCARD Region* SceneChunkLogic::GetRegion(const SR_MATH_NS::IVector3& region) const {
        auto&& pIt = std::find_if(m_regions.begin(), m_regions.end(), [&region](auto&& pRegion) {
            return pRegion->GetPosition() == region;
        });

        return pIt == m_regions.end() ? nullptr : *pIt;
    }

    SR_NODISCARD Region* SceneChunkLogic::GetOrLoadRegion(const SR_MATH_NS::IVector3& region) {
        auto&& pRegion = GetRegion(region);

        if (!pRegion) {
            pRegion = Region::Allocate(m_observer, m_regionWidth, m_defaultDimensions, region);
            pRegion->Load();
            m_regions.emplace_back(pRegion);
        }

        return pRegion;
    }

    void SceneChunkLogic::SetObserver(const GameObject::Ptr& target) {
        if (target != m_observer->m_target) {
            m_observer->SetTarget(target);
        }
    }

    const SceneLogic::GameObjects& SceneChunkLogic::GetGameObjectsAtChunk(const Math::IVector3 &region, const Math::IVector3 &chunk) const {
        const auto key = TensorKey(region, chunk);
        if (m_tensor.count(key) == 0) {
            static GameObjects _default = GameObjects();
            return _default;
        }

        return m_tensor.at(key);
    }

    bool SceneChunkLogic::ScopeCheckFunction(const SR_MATH_NS::IVector3& position) const {
        SR_TRACY_ZONE;

        if (position.HasZero()) {
            return false;
        }

        if (!m_observer) {
            return false;
        }

        if (position.y > 1 || position.y < -1) {
            return false;
        }

        constexpr float_t alpha = 3.f;

        /// TODO: it works only for cube chunks?
        /// TODO: есть предположение, что криво вычисляется попадание в радиус, надо проверить
        return ((SR_SQUARE(position.x) / alpha) + (SR_SQUARE(position.y) / alpha) + SR_SQUARE(position.z) <= SR_SQUARE(m_observer->m_scope));
    }

    std::pair<SR_MATH_NS::IVector3, SR_MATH_NS::IVector3> SceneChunkLogic::GetRegionAndChunk(const SR_MATH_NS::FVector3& pos) const {
        const auto chunkSize = Math::IVector3(m_defaultDimensions.dimension.cube);
        const World::Offset& offset = m_observer->m_offset;

        auto&& currentRegion = SR_MATH_NS::IVector3(pos / chunkSize);

        if (currentRegion.x == 0) { currentRegion.x = -1; }
        if (currentRegion.y == 0) { currentRegion.y = -1; }
        if (currentRegion.z == 0) { currentRegion.z = -1; }

        auto&& currentChunk = AddOffset(
                currentRegion,
                -offset.m_chunk
        );

        SRAssertOnce(!currentChunk.HasZero());

        return std::make_pair(currentRegion, currentChunk);
    }

    SR_MATH_NS::IVector3 SceneChunkLogic::CalculateCurrentChunk() const {
        SR_TRACY_ZONE;
        auto&& [region, chunk] = GetRegionAndChunk(m_observer->m_targetPosition);
        return chunk;
    }
}