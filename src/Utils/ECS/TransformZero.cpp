//
// Created by Monika on 26.12.2022.
//

#include <Utils/ECS/TransformZero.h>

namespace SR_UTILS_NS {
    Transform::Ptr TransformZero::Copy() const {
        return new TransformZero();
    }

    Transform::Ptr TransformHolder::Copy() const {
        return new TransformHolder();
    }
}