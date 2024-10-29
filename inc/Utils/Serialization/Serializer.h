//
// Created by Monika on 16.10.2024.
//

#ifndef SR_COMMON_SERIALIZATION_SERIALIZER_H
#define SR_COMMON_SERIALIZATION_SERIALIZER_H

#include <Utils/TypeTraits/TypeTraits.h>

namespace SR_UTILS_NS {
    class ISerializer {
    public:
        virtual ~ISerializer() = default;

        SR_NODISCARD virtual bool IsWriteDefaults() const noexcept { return m_isNeedWriteDefaults; }
        SR_NODISCARD virtual bool IsEditorAllowed() const noexcept { return m_isEditorAllowed; }
        SR_NODISCARD virtual bool IsAllowEmptyElementsInArray() const noexcept { return m_isAllowEmptyElementsInArray; }

        void SetWriteDefaults(const bool value) noexcept { m_isNeedWriteDefaults = value; }
        void SetEditorAllowed(const bool value) noexcept { m_isEditorAllowed = value; }
        void SetAllowEmptyElementsInArray(const bool value) noexcept { m_isAllowEmptyElementsInArray = value; }

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

        virtual void BeginItem(const SerializationId& id) = 0;
        virtual void EndItem() = 0;

        virtual void BeginObject(const SerializationId& id) = 0;
        virtual void EndObject() = 0;

        virtual void BeginArray(uint64_t size, const SerializationId& id) = 0;
        virtual void EndArray() = 0;

        SR_NODISCARD virtual bool SaveToFile(const SR_UTILS_NS::Path& path) const { return false; }

    private:
        bool m_isAllowEmptyElementsInArray = true;
        bool m_isNeedWriteDefaults = false;
        bool m_isEditorAllowed = false;

    };
}

#endif //SR_COMMON_SERIALIZATION_SERIALIZER_H
