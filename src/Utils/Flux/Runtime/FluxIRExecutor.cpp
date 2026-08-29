//
// Created by Monika on 08.08.2026.
//

#include <Utils/Flux/Runtime/FluxIRExecutor.h>
#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Flux/Parser/FluxParser.h>
#include <Utils/Flux/Runtime/FluxRuntime.h>
#include <Utils/FileSystem/PathDataAccessor.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Resources/ResourceManager.h>

#include <Codegen/FluxIRExecutor.generated.hpp>

namespace SR_FLUX_NS {
    void FluxIRExecutor::OnAttached() {
        SR_TRACY_ZONE;
        ReloadProgram();
        Super::OnAttached();
    }

    void FluxIRExecutor::Awake() {
        if (m_runtime) {
            m_runtime->Emit("Awake", {}, UpdateMode::Any);
        }
        Super::Awake();
    }

    void FluxIRExecutor::Start() {
        if (m_runtime) {
            m_runtime->Emit("Start", {}, UpdateMode::Any);
        }
        Super::Start();
    }

    void FluxIRExecutor::Update(float_t dt) {
        if (m_runtime) {
            m_runtime->Emit("Update", {}, UpdateMode::Update);
            m_runtime->Update(dt, UpdateMode::Update);
        }
        Super::Update(dt);
    }

    void FluxIRExecutor::ReloadProgram() {
        m_program = new FluxProgram();
        m_runtime.Reset();

        if (m_programPath.empty()) {
            m_programWatcher.AutoFree();
            return;
        }

        String buffer;

        auto&& filePath = ResourceManager::Instance().GetResPath().Concat(m_programPath);
        if (!FileSystem::ReadFile(filePath, buffer) || !FluxParser::Instance().Parse(buffer, *m_program)) {
            SR_ERROR("FluxIRExecutor::ReloadProgram() : failed to parse program: {}", m_programPath);
            return;
        }

        m_programWatcher = new FileWatcher(filePath);
        m_programWatcher->SetCallBack([&](auto&&) {
            ReloadProgram();
        });

        m_runtime = new FluxRuntime(m_program.Get());
        m_runtime->Initialize();

        auto&& pComponent = GetEntity().StaticCast<Component>();
        m_runtime->SetStorage(0, Reflection::Value::Create(pComponent));
    }
}