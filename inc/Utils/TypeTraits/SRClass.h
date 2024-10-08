//
// Created by Monika on 02.04.2024.
//

#ifndef SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H
#define SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H

#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    /// Флаги для сериализатора объектов
    SR_ENUM_NS_STRUCT_T(SerializationFlags, uint64_t,
        None = 1 << 0,
        Compress = 1 << 1,
        Editor = 1 << 2,
        NoUID = 1 << 3
    )

    /// Флаги самого сериализируемого объекта
    SR_ENUM_NS_STRUCT_T(ObjectSerializationFlags, uint64_t,
        None = 1 << 0,
        DontSave = 1 << 1,
        DontSaveRecursive = 1 << 2
    )

    class SRClass {
    public:
        virtual ~SRClass() = default;

    };
}

#define SR_CLASS(className, inherits)                                                                                   \
    private:                                                                                                            \
        void operator=(const className& p_rval) { }                                                                     \
        friend class SR_UTILS_NS::SRClass;                                                                              \
                                                                                                                        \
    private:                                                                                                            \

#endif //SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H
