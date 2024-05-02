//
// Created by innerviewer on 2024-05-02.
//
#ifndef SR_COMMON_CHUNK_H
#define SR_COMMON_CHUNK_H

#include <Utils/Common/NonCopyable.h>

#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/Marshal.h>
#include <Utils/Types/DataStorage.h>

#include <Utils/World/ChunkSystem/ChunkInfo.h>
#include <Utils/World/ChunkSystem/ChunkTicket.h>

namespace SR_UTILS_NS {
    class GameObject;
}

namespace SR_WORLD_NS {
    class Observer;

    class SR_DLL_EXPORT Chunk : public NonCopyable {
    protected:
        explicit Chunk(ChunkCreateInfo createInfo);

        enum class LoadState {
            PreLoaded, Loaded, Unload, Preload
        };

    public:
        static bool Belongs(ChunkDimensionInfo dimensionInfo, const SR_MATH_NS::IVector3& position, const SR_MATH_NS::FVector3& point);
        bool Belongs(const SR_MATH_NS::FVector3& point);

    public:
        bool PreLoad(SR_HTYPES_NS::Marshal* pMarshal);
        bool Load();
        bool Unload();

        SR_NODISCARD SR_HTYPES_NS::Marshal::Ptr Save(SR_HTYPES_NS::DataStorage* pContext) const;

    public:
        SR_NODISCARD LoadState GetState() const { return m_loadState; }
        SR_NODISCARD bool IsPreLoaded() const { return m_loadState == LoadState::PreLoaded; }

        SR_NODISCARD SR_MATH_NS::FVector3 GetWorldPosition(SR_MATH_NS::AxisFlag center = SR_MATH_NS::Axis::None) const;
        SR_NODISCARD SR_MATH_NS::IVector3 GetRegionPosition() const { return m_regionPosition; }
        SR_NODISCARD SR_MATH_NS::IVector3 GetPosition() const { return m_position; }

        SR_NODISCARD const std::vector<ChunkTicket>& GetTickets() const { return m_tickets; }
        SR_NODISCARD ChunkTicket GetFirstTicket() const { return m_tickets.empty() ? ChunkTicket{} : m_tickets.front(); }

        SR_NODISCARD ChunkDimensionType GetType() const { return m_dimensionInfo.type; }
        SR_NODISCARD ChunkDimensionInfo GetDimensionInfo() const { return m_dimensionInfo; }
        SR_NODISCARD SR_MATH_NS::IVector2 GetSquareDimensions() const { return m_dimensionInfo.dimension.square; }
        SR_NODISCARD SR_MATH_NS::IVector3 GetCubeDimensions() const { return m_dimensionInfo.dimension.cube; }
        SR_NODISCARD float_t GetHexagonRadius() const { return m_dimensionInfo.dimension.hexagon; }

    private:
        LoadState m_loadState;
        std::list<SR_HTYPES_NS::SharedPtr<GameObject>> m_preloaded;

        ChunkDimensionInfo m_dimensionInfo;
        std::vector<ChunkTicket> m_tickets;

        SR_MATH_NS::IVector3 m_position;
        SR_MATH_NS::IVector3 m_RegionPosition;

        Region* m_region = nullptr;
        Observer* m_region = nullptr;
    };
}

#endif //SR_COMMON_CHUNK_H
