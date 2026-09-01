//
// Created by Monika on 22.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_COMPONENT_H
#define SR_ENGINE_COMMON_FLUX_COMPONENT_H

#include <Utils/ECS/Component.h>
#include <Utils/Resources/ResourceRef.h>
#include <Utils/Flux/Graph/FluxGraphAsset.h>
#include <Utils/Flux/Runtime/FluxExecution.h>

namespace SR_FLUX_NS {
    struct FluxProgram;
    class FluxRuntime;

    /// @category(Scripting) @displayName(Flux)
    class FluxComponent : public Component {
        using Super = Component;
        SR_CLASS()
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<FluxComponent>;

    public:
        void Update(float_t dt) override;
        void FixedUpdate() override;

        SR_NODISCARD Path GetGraphPath() const;
        SR_NODISCARD const SR_HTYPES_NS::SharedPtr<FluxRuntime>& GetRuntime() const;

    private:
        /// @method @editorButton @condition(This.m_graph.IsValid())
        void InspectGraph();
        /// @method @evaluate @dontPack
        const Reflection::Value& GetVariable(StringAtom name) const;

        void DoUpdate(float_t dt, UpdateMode updateMode);
        void InitializeRuntime();

    private:
        /// @property
        ResourceRef<FluxGraphAsset> m_graph;
        /// @property
        Map<StringAtom, Reflection::Value> m_variables;

    private:
        Vector<Reflection::Value> m_callArguments;
        SR_HTYPES_NS::SharedPtr<FluxRuntime> m_runtime;
        Subscription m_onReloadedSubscription;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_COMPONENT_H
