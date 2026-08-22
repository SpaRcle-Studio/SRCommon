//
// Created by Monika on 22.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_COMPONENT_H
#define SR_ENGINE_COMMON_FLUX_COMPONENT_H

#include <Utils/ECS/Component.h>
#include <Utils/Resources/ResourceRef.h>
#include <Utils/Flux/Graph/FluxGraphAsset.h>

namespace SR_FLUX_NS {
    struct FluxProgram;
    class FluxRuntime;

    /// @category(Scripting) @displayName(Flux)
    class FluxComponent : public Component {
        using Super = Component;
        SR_CLASS()
    public:
        void Update(float_t dt) override;

        SR_NODISCARD Path GetGraphPath() const;

    private:
        /// @method @editorButton @condition(This.m_graph.IsValid())
        void InspectGraph();

    private:
        /// @property
        ResourceRef<FluxGraphAsset> m_graph;

    private:
        Vector<Reflection::Value> m_callArguments;
        RawPointerHolder<FluxRuntime> m_runtime;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_COMPONENT_H
