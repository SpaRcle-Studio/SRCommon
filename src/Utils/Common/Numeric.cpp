//
// Created by Monika on 26.04.2025.
//

#include <Utils/Common/Numeric.h>

namespace SR_UTILS_NS {
    Random::Random()
        : m_generator(clock())
        , m_randomDevice()
        , m_e2(m_randomDevice())
        , m_dist(std::llround(std::pow(2, 61))
        , std::llround(std::pow(2, 62)))
    { }

    void Random::Initialize() {
        srand(time(NULL)); /// NOLINT
    }

    SR_NODISCARD float_t Random::Float(float_t minimum, float_t maximum) {
        std::uniform_real_distribution<float_t> distribution(minimum, maximum);
        return distribution(m_generator);
    }

    SR_NODISCARD int32_t Random::Int32Range(int32_t minimum, int32_t maximum) {
        std::uniform_int_distribution<int32_t> distribution(minimum, maximum);
        return distribution(m_generator);
    }

    SR_NODISCARD int64_t Random::Int64() {
        return m_dist(m_e2);
    }

    SR_NODISCARD uint64_t Random::UInt64() {
        return static_cast<uint64_t>(m_dist(m_e2));
    }

    SR_NODISCARD int32_t Random::Int32() {
        return static_cast<int32_t>(m_dist(m_e2));
    }

    SR_NODISCARD uint32_t Random::UInt32() {
        return static_cast<uint32_t>(m_dist(m_e2));
    }
}