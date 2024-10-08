//
// Created by Monika on 21.09.2021.
//

#ifndef SR_ENGINE_UTILS_ISAVABLE_H
#define SR_ENGINE_UTILS_ISAVABLE_H

#include <Utils/Types/Marshal.h>
#include <Utils/TypeTraits/SRClass.h>
#include <Utils/Resources/Xml.h>

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

    struct SavableLoadData {

    };

    class SR_DLL_EXPORT ISerializable {
    protected:
        ISerializable() = default;
        virtual ~ISerializable() = default;

    public:
        void AddSerializationFlags(SerializationFlagsFlag flags) noexcept { m_flags |= flags; }
        void RemoveSerializationFlags(SerializationFlagsFlag flags) noexcept { m_flags &= ~flags; }

        SR_NODISCARD bool HasSerializationFlags(SerializationFlagsFlag flags) const noexcept;

        SR_DEPRECATED SR_NODISCARD virtual SR_HTYPES_NS::Marshal::Ptr SaveLegacy(SavableContext data) const;

    private:
        SerializationFlagsFlag m_flags = SerializationFlags::None;

    };
}

#endif //SR_ENGINE_UTILS_ISAVABLE_H
