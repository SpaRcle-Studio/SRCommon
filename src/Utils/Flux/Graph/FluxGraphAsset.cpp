//
// Created by Monika on 21.08.2026.
//

#include <Utils/Flux/Graph/FluxGraphAsset.h>

#include <Codegen/FluxGraphAsset.generated.hpp>

namespace SR_FLUX_NS {
    const FluxProgram* FluxGraphAsset::Compile() const {
        if (!m_compiled) {
            m_program = m_graph.Compile().value_or(FluxProgram());
            m_compiled = true;
        }
        return &m_program;
    }
}

