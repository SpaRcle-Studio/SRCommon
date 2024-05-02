//
// Created by Monika on 16.11.2021.
//

#include <Utils/World/Chunk.h>
#include <Utils/World/Region.h>
#include <Utils/World/Scene.h>
#include <Utils/ECS/GameObject.h>
#include <Utils/ECS/Transform.h>
//#include <Utils/World/SceneChunkLogic.h>

namespace SR_WORLD_NS {
    Chunk::Allocator Chunk::g_allocator = Chunk::Allocator();

    Chunk::Chunk(ChunkCreateInfo createInfo)
    {
        m_observer = createInfo.observer;
        m_region = createInfo.region;
        m_position = createInfo.position;
        m_dimensionInfo = createInfo.dimensionInfo;

        SRAssert(m_observer);
        SRAssert(m_region);
        SRAssert(!m_position.HasZero());

        if (m_region) {
            m_regionPosition = m_region->GetPosition();
        }
    }

    Chunk::~Chunk() {
        SRAssert(m_preloaded.empty());
    }

    void Chunk::Update(float_t dt) {
        /*if (m_lifetime > 0) {
            m_lifetime -= dt;
        }*/
    }

    bool Chunk::Access(float_t dt) {
        //m_lifetime = dt + 10.f;
        SRAssert(m_loadState != LoadState::Unload);
        return true;
    }

    bool Chunk::Unload() {
        SR_TRACY_ZONE;

        m_loadState = LoadState::Unload;

        /*TODO: это потенциальное место для дедлоков, так как при уничтожении компоненты
         * блокируют другие потоки. Придумать как исправить */

        auto&& pLogic = m_observer->m_scene->GetLogicBase().DynamicCast<SceneChunkLogic>();
        auto&& gameObjects = pLogic->GetGameObjectsAtChunk(m_regionPosition, m_position);

        for (auto&& gameObject : gameObjects) {
            if (gameObject) {
                gameObject->Destroy();
            }
        }

        for (auto&& gameObject : m_preloaded) {
            if (gameObject) {
                gameObject->Destroy();
            }
        }
        m_preloaded.clear();

        return true;
    }

    void Chunk::OnExit() {
        m_region->OnExit();
    }

    void Chunk::OnEnter() {
        m_region->OnEnter();
    }

    void Chunk::SetAllocator(const Chunk::Allocator &allocator) {
        g_allocator = allocator;
    }

    Chunk* Chunk::Allocate(ChunkCreateInfo createInfo) {
        if (g_allocator) {
            return g_allocator(createInfo);
        }

        return new Chunk(createInfo);
    }

    bool Chunk::Belongs(const SR_MATH_NS::FVector3 &point) {
        return Belongs(m_dimensionInfo, m_position, point);
    }

    bool Chunk::Belongs(ChunkDimensionInfo dimensionInfo, const SR_MATH_NS::IVector3& position, const Math::FVector3& point) {
        switch (dimensionInfo.type) {
            case ChunkDimensionType::Cube: {
                const float_t xMax = position.x + dimensionInfo.dimension.cube.x;
                const float_t yMax = position.y + dimensionInfo.dimension.cube.y;
                const float_t zMax = position.z + dimensionInfo.dimension.cube.x;

                return point.x >= position.x && point.y >= position.y && point.z >= position.z &&
                       point.x <= xMax && point.y <= yMax && point.z <= zMax;
            }
            default:
                SR_PLATFORM_NS::Terminate();
                return false;
        }
    }

    bool Chunk::ApplyOffset() {
        return true;
    }

    bool Chunk::PreLoad(SR_HTYPES_NS::Marshal* pMarshal) {
        SR_TRACY_ZONE;

        /// TODO: add version and migration

        if (pMarshal && pMarshal->Valid()) {
            if (m_position != pMarshal->Read<SR_MATH_NS::IVector3>()) {
                SRAssert2(false, "Something went wrong...");
                return false;
            }

            const uint64_t count = pMarshal->Read<uint64_t>();
            for (uint64_t i = 0; i < count; ++i) {
                if (auto&& ptr = GameObject::Load(*pMarshal, nullptr)) {
                    auto&& pTransform = ptr->GetTransform();

                    if (pTransform->GetMeasurement() == SR_UTILS_NS::Measurement::Space3D) {
                        pTransform->GlobalTranslate(GetWorldPosition());
                    }

                    m_preloaded.emplace_back(ptr);
                }
            }
        }

        m_loadState = LoadState::PreLoaded;

        Access(0.f);

        return true;
    }

    bool Chunk::Load() {
        SR_TRACY_ZONE;

        //SRAssert(m_loadState == LoadState::PreLoaded);

        for (auto&& gameObject : m_preloaded) {
            m_observer->m_scene->RegisterGameObject(gameObject);
        }

        m_preloaded.clear();

        m_loadState = LoadState::Loaded;

        return true;
    }

    void Chunk::Reload() {

    }

    SR_HTYPES_NS::Marshal::Ptr Chunk::Save(SR_HTYPES_NS::DataStorage* pContext) const {
        SR_TRACY_ZONE;

        /// scene is locked

        auto&& pLogic = m_observer->m_scene->GetLogicBase().DynamicCast<SceneChunkLogic>();
        auto&& gameObjects = pLogic->GetGameObjectsAtChunk(m_regionPosition, m_position);

        if (gameObjects.empty() && m_preloaded.empty()) {
            return nullptr;
        }

        std::list<SR_HTYPES_NS::Marshal::Ptr> marshaled;

        /// сохраняем объекты относительно начала координат чанка
        pContext->SetValue<SR_MATH_NS::FVector3>(-GetWorldPosition());

        const auto gameObjectSaveData = SR_UTILS_NS::SavableContext(nullptr, SAVABLE_FLAG_ECS_NO_ID);

        for (auto&& gameObject : gameObjects) {
            if (gameObject.RecursiveLockIfValid()) {
                if (auto&& gameObjectMarshal = gameObject->Save(gameObjectSaveData); gameObjectMarshal) {
                    if (gameObjectMarshal->Valid()) {
                        marshaled.emplace_back(gameObjectMarshal);
                    }
                    else {
                        SR_SAFE_DELETE_PTR(gameObjectMarshal);
                    }
                }

                gameObject.Unlock();
            }
        }

        for (auto&& gameObject : m_preloaded) {
            if (gameObject.RecursiveLockIfValid()) {
                if (auto &&gameObjectMarshal = gameObject->Save(gameObjectSaveData); gameObjectMarshal) {
                    if (gameObjectMarshal->Valid()) {
                        marshaled.emplace_back(gameObjectMarshal);
                    }
                    else {
                        SR_SAFE_DELETE_PTR(gameObjectMarshal);
                    }
                }

                gameObject.Unlock();
            }
        }

        pContext->RemoveValue<SR_MATH_NS::FVector3>();

        auto&& pMarshal = new SR_HTYPES_NS::Marshal();

        if (marshaled.empty()) {
            return pMarshal;
        }

        pMarshal->Write(m_position);
        pMarshal->Write(static_cast<uint64_t>(marshaled.size()));

        for (auto&& gameObject : marshaled)
            pMarshal->Append(gameObject);

        return pMarshal;
    }

    SR_MATH_NS::FVector3 Chunk::GetWorldPosition(SR_MATH_NS::AxisFlag center) const {
        auto fPos = SR_UTILS_NS::World::AddOffset(
                ((m_region->GetWorldPosition()) + (m_position - SR_MATH_NS::FVector3(1, 1, 1))).Cast<SR_MATH_NS::Unit>(),
                m_observer->m_offset.m_chunk.Cast<SR_MATH_NS::Unit>()
        );

        fPos = SR_MATH_NS::FVector3(
                fPos.x * m_dimensionInfo.dimension.cube.x + (center & SR_MATH_NS::Axis::X ? (SR_MATH_NS::Unit) m_dimensionInfo.dimension.cube.x / 2 : 0),
                fPos.y * m_dimensionInfo.dimension.cube.y + (center & SR_MATH_NS::Axis::Y ? (SR_MATH_NS::Unit) m_dimensionInfo.dimension.cube.y / 2 : 0),
                fPos.z * m_dimensionInfo.dimension.cube.x + (center & SR_MATH_NS::Axis::Z ? (SR_MATH_NS::Unit) m_dimensionInfo.dimension.cube.x / 2 : 0)
        );

        fPos = fPos.DeSingular(SR_MATH_NS::FVector3(m_dimensionInfo.dimension.cube));

        return fPos;
    }

    Chunk::ScenePtr Chunk::GetScene() const {
        return m_observer->m_scene;
    }

    void Chunk::AddTicket(ChunkTicket newTicket) {
        if (m_tickets.empty()) {
            m_tickets.emplace_back(newTicket);
            return;
        }

        ///TODO: Use sorted vector instead.
        auto&& pIt = std::upper_bound(m_tickets.begin(), m_tickets.end(), newTicket, &ChunkTicket::Compare);
        m_tickets.insert(pIt, newTicket);
    }

    /*void Chunk::SetDebugActive(BoolExt enabled) {
        if (!Features::Instance().Enabled("DebugChunks", false)) {
            enabled = BoolExt::False;
        }

        if (enabled == BoolExt::True || (enabled == BoolExt::None && m_debugActiveId != SR_ID_INVALID)) {
            m_debugActiveId = SR_UTILS_NS::DebugDraw::Instance().DrawCube(
                    GetWorldPosition(SR_MATH_NS::AXIS_XYZ),
                    SR_MATH_NS::Quaternion::Identity(),
                    SR_MATH_NS::FVector3(m_size.x, m_size.y, m_size.x) / 2,
                    SR_MATH_NS::FColor(0, 255, 0, 255),
                    SR_FLOAT_MAX
            );
        }
        else if (m_debugActiveId != SR_ID_INVALID) {
            SR_UTILS_NS::DebugDraw::Instance().DrawPlane(m_debugActiveId);
            m_debugActiveId = SR_ID_INVALID;
        }
    }

    void Chunk::SetDebugLoaded(BoolExt enabled) {
        if (m_position.y != 1 || m_regionPosition.y != 1) {
            return;
        }

        if (!Features::Instance().Enabled("DebugChunks", false)) {
            enabled = BoolExt::False;
        }

        if (enabled == BoolExt::True || (enabled == BoolExt::None && m_debugLoadedId != SR_ID_INVALID)) {
            m_debugLoadedId = SR_UTILS_NS::DebugDraw::Instance().DrawPlane(
                    GetWorldPosition(SR_MATH_NS::AXIS_XZ),
                    SR_MATH_NS::Quaternion::Identity(),
                    SR_MATH_NS::FVector3(m_size.x, m_size.y, m_size.x) / 2,
                    SR_MATH_NS::FColor(255, 255, 0, 255),
                    SR_FLOAT_MAX
            );
        }
        else if (m_debugLoadedId != SR_ID_INVALID) {
            SR_UTILS_NS::DebugDraw::Instance().DrawPlane(m_debugLoadedId);
            m_debugLoadedId = SR_ID_INVALID;
        }
    }*/
}