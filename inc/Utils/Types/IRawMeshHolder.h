//
// Created by Monika on 20.03.2023.
//

#ifndef SR_ENGINE_UTILS_I_RAW_MESH_HOLDER_H
#define SR_ENGINE_UTILS_I_RAW_MESH_HOLDER_H

#include <Utils/Types/SharedPtr.h>
#include <Utils/Common/Vertices.h>
#include <Utils/Common/Subscription.h>
#include <Utils/Serialization/Serializable.h>

namespace SR_HTYPES_NS {
    class RawMesh;

    /// Этот класс предоставляет удобный интерфейс для работы с RwMesh
    class IRawMeshHolder {
    public:
        using RawMeshPtr = SR_HTYPES_NS::SharedPtr<SR_HTYPES_NS::RawMesh>;
        using MeshIndex = int32_t;

        IRawMeshHolder() = default;
        virtual ~IRawMeshHolder();

        IRawMeshHolder(const IRawMeshHolder& other);
        IRawMeshHolder& operator=(const IRawMeshHolder& other);
        IRawMeshHolder(IRawMeshHolder&& other) noexcept;
        IRawMeshHolder& operator=(IRawMeshHolder&& other) noexcept;

    public:
        SR_NODISCARD MeshIndex GetMeshId() const noexcept { return m_meshId; }
        SR_NODISCARD const RawMeshPtr& GetRawMesh() const noexcept { return m_rawMesh; }
        SR_NODISCARD const SR_UTILS_NS::Path& GetMeshPath() const noexcept;
        SR_NODISCARD std::string GetMeshStringPath() const noexcept;
        SR_NODISCARD bool IsValidMeshId() const noexcept;
        SR_NODISCARD StringView GetGeometryName() const noexcept;
        SR_NODISCARD const SR_UTILS_NS::VertexDataBuffer& GetVertexBuffer(const SR_UTILS_NS::VertexLayoutDescription& layout) const;

        virtual void OnRawMeshChanged() { }

        void SetRawMesh(const SR_UTILS_NS::Path& path);
        void SetRawMesh(const RawMeshPtr& pRawMesh);
        void SetMeshId(MeshIndex meshIndex, bool forceReload = false);

    private:
        SR_UTILS_NS::Subscription m_reloadSubscription;
        RawMeshPtr m_rawMesh;
        /// определяет порядок меша в файле, если их там несколько
        /// TODO: переделать в int16_t, но нужно написать миграторы.
        MeshIndex m_meshId = SR_ID_INVALID;

    };

    class RawMeshHolder : public SR_UTILS_NS::Serializable, public IRawMeshHolder {
        SR_CLASS()
        using Super = SR_UTILS_NS::Serializable;
    public:
        /// @virtualProperty(meshPath) @getter(GetMeshPath) @setter(SetRawMesh)
        /// @customArgs(pick: enabled, filter name: Meshes, relative: resources)
        /// @customArg(filter value: fbx,blend,obj,pmx,stl,dae)
        SR_VIRTUAL_PROPERTY
        /// @virtualProperty(meshId) @getter(GetMeshId) @setter(SetMeshId)
        /// @condition(!This.GetMeshPath().empty())
        SR_VIRTUAL_PROPERTY

    };
}

#endif //SR_ENGINE_UTILS_I_RAW_MESH_HOLDER_H
