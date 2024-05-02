//
// Created by Monika on 30.09.2021.
//

#ifndef GAMEENGINE_CHUNK_H
#define GAMEENGINE_CHUNK_H

#include <Utils/Math/Vector2.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/DataStorage.h>
#include <Utils/Types/Marshal.h>
#include <Utils/World/Observer.h>
#include <Utils/World/ChunkTicket.h>

/*
namespace SR_UTILS_NS {
    class GameObject;
}

namespace SR_WORLD_NS {
    class Scene;
    class Region;
    class Chunk;
    class Observer;

    class SR_DLL_EXPORT Chunk : public NonCopyable {
        using ScenePtr = SR_HTYPES_NS::SafePtr<Scene>;
    protected:
        explicit Chunk(ChunkCreateInfo createInfo);

    public:
        ~Chunk() override;

        enum class LoadState {
            PreLoaded, Loaded, Unload, Preload
        };
    public:
        typedef std::function<Chunk*(ChunkCreateInfo)> Allocator;
    public:
        static void SetAllocator(const Allocator& allocator);
        static Chunk* Allocate(ChunkCreateInfo createInfo);
        static bool Belongs(ChunkDimensionInfo dimensionInfo, const SR_MATH_NS::IVector3& position, const Math::FVector3& point);

    public:
        SR_NODISCARD LoadState GetState() const { return m_loadState; }
        SR_NODISCARD bool IsPreLoaded() const { return m_loadState == LoadState::PreLoaded; }
        SR_NODISCARD SR_MATH_NS::FVector3 GetWorldPosition(SR_MATH_NS::AxisFlag center = SR_MATH_NS::Axis::None) const;
        SR_NODISCARD SR_MATH_NS::IVector3 GetPosition() const { return m_position; }
        SR_NODISCARD ScenePtr GetScene() const;
        SR_NODISCARD const std::vector<ChunkTicket>& GetTickets() const { return m_tickets; }
        SR_NODISCARD ChunkTicket GetFirstTicket() const { return m_tickets.front(); }

        SR_NODISCARD ChunkDimensionType GetType() const { return m_dimensionInfo.type; }
        SR_NODISCARD ChunkDimensionInfo GetDimensionInfo() const { return m_dimensionInfo; }
        SR_NODISCARD SR_MATH_NS::IVector2 GetSquareDimensions() const { return m_dimensionInfo.dimension.square; }
        SR_NODISCARD SR_MATH_NS::IVector3 GetCubeDimensions() const { return m_dimensionInfo.dimension.cube; }
        SR_NODISCARD float_t GetHexagonRadius() const { return m_dimensionInfo.dimension.hexagon; }

        SR_NODISCARD SR_HTYPES_NS::Marshal::Ptr Save(SR_HTYPES_NS::DataStorage* pContext) const;

    public:
        void AddTicket(ChunkTicket ticket);

    public:
        virtual void OnEnter();
        virtual void OnExit();
        virtual void Reload();
        virtual void Update(float_t dt);
        virtual bool Access(float_t dt);
        virtual bool Belongs(const Math::FVector3& point);
        virtual bool Unload();
        virtual bool PreLoad(SR_HTYPES_NS::Marshal* pMarshal);
        virtual bool Load();

        virtual bool ApplyOffset();

    private:
        static Allocator g_allocator;

    protected:
        LoadState m_loadState;
        std::list<SR_HTYPES_NS::SharedPtr<GameObject>> m_preloaded;

        std::vector<ChunkTicket> m_tickets;

        SR_MATH_NS::IVector3 m_position;
        SR_WORLD_NS::Observer* m_observer = nullptr;
        SR_WORLD_NS::Region* m_region = nullptr;

        //ChunkCreateInfo m_info;
        ChunkDimensionInfo m_dimensionInfo;
        SR_MATH_NS::IVector3 m_regionPosition;
    };
}
*/

#endif //GAMEENGINE_CHUNK_H
