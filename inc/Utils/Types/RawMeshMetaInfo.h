//
// Created by Monika on 14.10.2026.
//

#ifndef SR_ENGINE_COMMON_RAW_MESH_META_INFO_H
#define SR_ENGINE_COMMON_RAW_MESH_META_INFO_H

#include <Utils/Animations/SkeletonRig.h>

namespace SR_HTYPES_NS {
    struct RawMeshMetaInfo : public SR_UTILS_NS::Serializable {
        SR_STRUCT()
    public:
        bool operator==(const RawMeshMetaInfo& other) const {
            return rig.GetId() == other.rig.GetId();
        }

        bool operator!=(const RawMeshMetaInfo& other) const {
            return !(*this == other);
        }

    public:
        /// @property @tooltip(Риг с полным набором костей. Если задан, индексы костей меша сопоставляются по именам с индексами костей скелета рига)
        SR_UTILS_NS::ResourceRef<SR_ANIMATIONS_NS::SkeletonRig> rig;

    };
}

#endif //SR_ENGINE_COMMON_RAW_MESH_META_INFO_H
