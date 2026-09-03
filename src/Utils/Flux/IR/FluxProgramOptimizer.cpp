//
// Created by Monika on 03.09.2026.
//

#include <Utils/Flux/IR/FluxProgramOptimizer.h>
#include <Utils/Flux/IR/FluxProgram.h>

namespace SR_FLUX_NS {
    /**
    example of useless operations:
        call FluxUtils.GetActiveFluxComponent
        mv %0 %1 <-- this
        call FluxComponent.GetGameObject %1
        mv %0 %1 <-- this
        call GameObject.GetTransform %1
        mv %0 %1 <-- NOT this
        call Input.GetMouseDrag
        mv %0 %3 <-- this
        call MathUtils.GetFVector2X %3
        mv %0 %4 <-- this
        call MathUtils.MultiplyFloat %4 @12
        mv %0 %4 <-- this
        call Transform.RotateXYZ %1 @11 %4 @11

        mv %0 %2 <-- this
        cp %2 %0 <-- this
        br branch_true_15

        call MathUtils.SubtractFloat $0 %5
        mv %0 %5 <-- this
        cp %5 $0 <-- NOT this
    */
    bool RemoveUselessMoveAndCopyOperations(FluxProgram& program) {
        return false;
    }

    bool OptimizeProgram(FluxProgram& program) {
        return false;
    }
}