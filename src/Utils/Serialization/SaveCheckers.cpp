//
// Created by Monika on 11.12.2025.
//

#include <Utils/Types/SharedPtr.h>
#include <Utils/Serialization/Serializable.h>
#include <Utils/Serialization/SerializationTraits.h>
#include <Utils/Serialization/SerializationFlags.h>

namespace SR_UTILS_NS {
    #include <Utils/Serialization/SaveCheckers.inl.h>

    bool SerializableCanBeSavedImpl(const Serializable& value) {
        return !value.HasSerializationFlags(SR_UTILS_NS::SerializationFlags::DontSave);
    }
}