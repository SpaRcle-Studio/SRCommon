//
// Created by Monika on 22.08.2026.
//

#include <Utils/Flux/Runtime/FluxComponent.h>
#include <Utils/Flux/Runtime/FluxRuntime.h>
#include <Utils/Common/SubscriptionMessage.h>
#include <Utils/Events/Broadcaster.h>

#include <Codegen/FluxComponent.generated.hpp>

namespace SR_FLUX_NS {
    void FluxComponent::Update(float_t dt) {
        if (!m_runtime) {
            if (auto&& pGraph = m_graph.GetResource()) {
                if (auto&& pProgram = pGraph->Compile()) {
                    m_runtime = new FluxRuntime(pProgram);
                    auto&& pComponent = GetEntity().StaticCast<Component>();
                    m_runtime->SetStorage(0, Reflection::Value::Create(pComponent));
                }
            }
        }

        if (m_runtime) {
            if (!m_runtime->IsEmitted("Update")) {
                m_callArguments.resize(1);
                m_callArguments[0] = Reflection::Value::Create(dt);
                m_runtime->Emit("Update", m_callArguments);
            }
            m_runtime->Update(dt);
        }

        Super::Update(dt);
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
}
