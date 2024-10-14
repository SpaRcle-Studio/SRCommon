//
// Created by Monika on 21.09.2021.
//

#ifndef SR_ENGINE_UTILS_SERIALIZABLE_H
#define SR_ENGINE_UTILS_SERIALIZABLE_H

#include <Utils/TypeTraits/SRClass.h>

namespace SR_UTILS_NS {
    typedef uint64_t SavableFlags;

    enum SavableFlagBits {
        SAVABLE_FLAG_NONE = 1 << 0,
        SAVABLE_FLAG_ECS_NO_ID = 1 << 1,
    };

    struct SavableContext {
        SavableContext() = default;

        SavableContext(SR_HTYPES_NS::Marshal::Ptr pMarshal, SavableFlags flags)
            : pMarshal(pMarshal)
            , flags(flags)
        { }

        SR_HTYPES_NS::Marshal::Ptr pMarshal = nullptr;
        SavableFlags flags = SAVABLE_FLAG_NONE;
    };

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

    class ISerializer {

    };

    class IDeserializer {

    };

    class Serializable : public SRClass {
        SR_CLASS()
    public:
        void Save(ISerializer& serializer) const;
        void Load(IDeserializer& deserializer);

        void AddSerializationFlags(SerializationFlagsFlag flags) noexcept { m_flags |= flags; }
        void RemoveSerializationFlags(SerializationFlagsFlag flags) noexcept { m_flags &= ~flags; }

        SR_NODISCARD bool HasSerializationFlags(SerializationFlagsFlag flags) const noexcept;

        SR_DEPRECATED SR_NODISCARD virtual SR_HTYPES_NS::Marshal::Ptr SaveLegacy(SavableContext data) const;

    private:
        SerializationFlagsFlag m_flags = SerializationFlags::None;

    };
}

#endif //SR_ENGINE_UTILS_SERIALIZABLE_H
