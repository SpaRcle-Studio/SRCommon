//
// Created by Monika on 08.08.2026.
//

#include <Utils/Flux/Runtime/FluxIRExecutor.h>
#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Flux/Parser/FluxParser.h>
#include <Utils/FileSystem/PathDataAccessor.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Resources/ResourceManager.h>

#include <Codegen/FluxIRExecutor.generated.hpp>

namespace SR_FLUX_NS {
    void FluxIRExecutor::OnAttached() {
        FluxProgram program;
        String buffer;

        auto&& filePath = ResourceManager::Instance().GetResPath().Concat(m_programPath);
        if (!FileSystem::ReadFile(filePath, buffer) || !FluxParser::Instance().Parse(buffer, program)) {
            SR_ERROR("FluxIRExecutor::OnAttached() : failed to parse program: {}", m_programPath);
            Super::OnAttached();
            return;
        }

        Super::OnAttached();
    }
}