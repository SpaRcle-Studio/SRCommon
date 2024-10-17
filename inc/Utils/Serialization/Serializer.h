//
// Created by Monika on 16.10.2024.
//

#ifndef SR_COMMON_SERIALIZATION_SERIALIZER_H
#define SR_COMMON_SERIALIZATION_SERIALIZER_H

#include <Utils/TypeTraits/TypeTraits.h>

namespace SR_UTILS_NS {
    class ISerializer {
    public:
        enum class SpecialWriteType : uint8_t {
            None,
            PointerType,
            PointerEditorOnly,
            MapKey,
            ArrayIndex
        };

    public:
        virtual ~ISerializer() = default;

        SR_NODISCARD virtual bool IsWriteDefaults() const noexcept = 0;

        virtual void WriteString(std::string_view value, const SerializationId& name) = 0;
        virtual void WriteBool(bool value, const SerializationId& name) = 0;
        virtual void WriteInt(int8_t value, const SerializationId& name) = 0;
        virtual void WriteInt(int16_t value, const SerializationId& name) = 0;
        virtual void WriteInt(int32_t value, const SerializationId& name) = 0;
        virtual void WriteInt(int64_t value, const SerializationId& name) = 0;
        virtual void WriteUInt(uint8_t value, const SerializationId& name) = 0;
        virtual void WriteUInt(uint16_t value, const SerializationId& name) = 0;
        virtual void WriteUInt(uint32_t value, const SerializationId& name) = 0;
        virtual void WriteUInt(uint64_t value, const SerializationId& name) = 0;
        virtual void WriteFloat(float_t value, const SerializationId& name) = 0;
        virtual void WriteDouble(double_t value, const SerializationId& name) = 0;

        virtual void BeginObject(const SerializationId& name) = 0;
        virtual void EndObject() = 0;

        virtual void BeginSpecialWrite(SpecialWriteType type) = 0;
        virtual void EndSpecialWrite() = 0;

        virtual void BeginArray(uint64_t size, const SerializationId& name, const char* associativeKey) = 0;
        virtual void EndArray() = 0;

    };
}

#endif //SR_COMMON_SERIALIZATION_SERIALIZER_H
