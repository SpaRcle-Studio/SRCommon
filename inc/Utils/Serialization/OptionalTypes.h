//
// Created by Monika on 26.07.2025.
//

#ifndef SR_COMMON_SERIALIZATION_OPTIONAL_TYPES_H
#define SR_COMMON_SERIALIZATION_OPTIONAL_TYPES_H

#include <Utils/Serialization/Serializable.h>

namespace SR_UTILS_NS {
    /*class SR_COMMON_DLL_API OptionalFloat : public Serializable {
        using Super = Serializable;
        SR_CLASS()
    public:
        OptionalFloat();
        OptionalFloat(float_t value);

        ~OptionalFloat() override;

        OptionalFloat(const OptionalFloat& other);
        OptionalFloat(OptionalFloat&& other) noexcept;

        OptionalFloat& operator=(const OptionalFloat& other);
        OptionalFloat& operator=(OptionalFloat&& other) noexcept;

        SR_NODISCARD bool IsSet() const noexcept;

        operator float_t() const noexcept;
        operator float_t&() noexcept;

        OptionalFloat& operator=(float_t value) noexcept;

    private:
        /// @property
        float m_value = 0.0f;
        /// @property
        bool m_isSet = false;

    };*/
}

#endif //SR_COMMON_SERIALIZATION_OPTIONAL_TYPES_H
