//
// Created by Monika on 16.10.2024.
//

#ifndef SR_COMMON_SERIALIZATION_DESERIALIZER_H
#define SR_COMMON_SERIALIZATION_DESERIALIZER_H

#include <Utils/TypeTraits/TypeTraits.h>

namespace SR_UTILS_NS {
    class IDeserializer {
    public:
        enum class ReloadPointerReason : uint8_t {
            None,
            IsNull,
            HasDifferentType
        };

    public:
        virtual ~IDeserializer() = default;

        SR_NODISCARD virtual bool IsDefault(const SerializationId&) const noexcept = 0;
        SR_NODISCARD virtual bool ShouldSetDefaults(const SerializationId&) const noexcept = 0;
        SR_NODISCARD virtual bool ShouldSetDefaults() const noexcept = 0;
        SR_NODISCARD virtual bool AllowNewMapKeys() const noexcept = 0;
        SR_NODISCARD virtual bool IsPreserveMode() const noexcept = 0;
        SR_NODISCARD virtual bool AllowReloadPointer(ReloadPointerReason reason) const noexcept = 0;

        virtual bool NextItem(const SerializationId& key) noexcept = 0;

        virtual void BeginObject(const SerializationId& key) = 0;
        virtual void EndObject() = 0;

        virtual uint64_t BeginArray(const SerializationId& key, const char* associativeKey) = 0;
        virtual void EndArray() = 0;

        virtual void ReadString(std::string& value, const SerializationId& name) = 0;
        virtual void ReadString(SR_UTILS_NS::StringAtom& value, const SerializationId& name) = 0;
        virtual void ReadBool(bool& value, const SerializationId& name) = 0;
        virtual void ReadInt(int8_t& value, const SerializationId& name) = 0;
        virtual void ReadInt(int16_t& value, const SerializationId& name) = 0;
        virtual void ReadInt(int32_t& value, const SerializationId& name) = 0;
        virtual void ReadInt(int64_t& value, const SerializationId& name) = 0;
        virtual void ReadUInt(uint8_t& value, const SerializationId& name) = 0;
        virtual void ReadUInt(uint16_t& value, const SerializationId& name) = 0;
        virtual void ReadUInt(uint32_t& value, const SerializationId& name) = 0;
        virtual void ReadUInt(uint64_t& value, const SerializationId& name) = 0;
        virtual void ReadFloat(float_t& value, const SerializationId& name) = 0;
        virtual void ReadDouble(double_t& value, const SerializationId& name) = 0;

        virtual void ReportError(const SerializationId& name, const std::string& message) = 0;

    };
}

#endif //SR_COMMON_SERIALIZATION_DESERIALIZER_H
