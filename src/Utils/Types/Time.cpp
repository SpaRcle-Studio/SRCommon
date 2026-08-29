//
// Created by Nikita on 30.12.2020.
//

#include <Utils/Types/Time.h>

#include <Codegen/Time.generated.hpp>

namespace SR_HTYPES_NS {
    void Time::Update() {
        SR_TRACY_ZONE;

        m_timeInfo = TimeInfo(
            ClockT::now(),
            static_cast<uint64_t>(clock())
        );
    }
}