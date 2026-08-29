//
// Created by Monika on 22.08.2026.
//

#include <Utils/Flux/Runtime/FluxComponent.h>
#include <Utils/Flux/Runtime/FluxRuntime.h>
#include <Utils/Flux/Runtime/FluxUtils.h>
#include <Utils/Common/SubscriptionMessage.h>
#include <Utils/Events/Broadcaster.h>
#include <Utils/Types/Time.h>

#include <Codegen/FluxComponent.generated.hpp>

namespace SR_FLUX_NS {
    void FluxComponent::InitializeRuntime() {
        if (!m_runtime) {
            SR_TRACY_ZONE;
            m_onReloadedSubscription.Reset();
            if (auto&& pGraph = m_graph.GetResource()) {
                if (auto&& pProgram = pGraph->Compile()) {
                    m_runtime = new FluxRuntime(pProgram);
                    m_runtime->Emit("Start", {}, UpdateMode::Any);
                }
                m_onReloadedSubscription = pGraph->Subscribe(IResource::RELOAD_DONE_EVENT, [this](auto&&) {
                    m_runtime.Reset();
                });
            }
        }
    }

    void FluxComponent::DoUpdate(float_t dt, UpdateMode updateMode) {
        SR_TRACY_ZONE;

        InitializeRuntime();

        const StringView eventName = updateMode == UpdateMode::FixedUpdate ? "FixedUpdate" : "Update";

        if (m_runtime) {
            FluxUtils::Instance().SetActiveFluxComponent(this);
            if (!m_runtime->IsEmitted(eventName) && !m_runtime->IsEmitted("Start")) {
                if (updateMode == UpdateMode::Update) {
                    m_callArguments.resize(1);
                    m_callArguments[0] = Reflection::Value::Create(dt);
                }
                else {
                    m_callArguments.clear();
                }
                m_runtime->Emit(eventName, m_callArguments, updateMode);
            }
            m_runtime->Update(dt, updateMode);
            FluxUtils::Instance().SetActiveFluxComponent(nullptr);
        }
    }

    void FluxComponent::Update(float_t dt) {
        DoUpdate(dt, UpdateMode::Update);
        Super::Update(dt);
    }

    void FluxComponent::FixedUpdate() {
        DoUpdate(SR_HTYPES_NS::Time::Instance().FixedDeltaTime(), UpdateMode::FixedUpdate);
        Super::FixedUpdate();
    }

    void FluxComponent::InspectGraph() {
        SubscriptionMessage message;
        message.SetStringAtom("ClassName", FluxComponent::GetClassStaticName());
        message.SetInt("EntityId", GetEntityId());
        Broadcaster::Instance().Broadcast(Events::EVENT_DO_INSPECT_ENTITY_ID, message);
    }

    Path FluxComponent::GetGraphPath() const {
        if (auto&& pGraph = m_graph.GetResource()) {
            return pGraph->GetResourcePath();
        }
        return {};
    }

    const Reflection::Value& FluxComponent::GetVariable(StringAtom name) const {
        if (auto&& pIt = m_variables.find(name); pIt != m_variables.end()) {
            return pIt->second;
        }
        static Reflection::Value EMPTY_VALUE;
        return EMPTY_VALUE;
    }
}
