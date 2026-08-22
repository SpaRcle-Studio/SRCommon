//
// Created by Monika on 21.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_GRAPH_ASSET_H
#define SR_ENGINE_COMMON_FLUX_GRAPH_ASSET_H

#include <Utils/Resources/Asset.h>
#include <Utils/Flux/Graph/FluxGraph.h>
#include <Utils/Flux/IR/FluxProgram.h>

namespace SR_FLUX_NS {
    /// @extension(flux)
    class FluxGraphAsset : public Asset {
        using Super = Asset;
        SR_CLASS()
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<FluxGraphAsset>;

    public:
        SR_NODISCARD const FluxProgram* Compile() const;

        SR_NODISCARD const FluxGraph& GetGraph() const { return m_graph; }

        /// граф помечается как изменённый, следующий Compile() пересоберёт программу
        SR_NODISCARD FluxGraph& GetGraphMutable() { m_compiled = false; return m_graph; }

        void Invalidate() { m_compiled = false; }

    private:
        /// @property
        FluxGraph m_graph;

    private:
        mutable FluxProgram m_program;
        mutable bool m_compiled = false;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_GRAPH_ASSET_H
