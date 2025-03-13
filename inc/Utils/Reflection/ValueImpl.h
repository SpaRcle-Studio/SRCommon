//
// Created by Monika on 31.01.2025.
//

#ifndef SR_UTILS_TYPE_TRAITS_VALUE_IMPL_H
#define SR_UTILS_TYPE_TRAITS_VALUE_IMPL_H

#include <Utils/Common/Enumerations.h>
#include <Utils/Common/TypeInfo.h>
#include <Utils/Types/StringAtom.h>

namespace SR_UTILS_NS::Reflection {
    /*SR_ENUM_NS_CLASS_T(ValueTypeKind, uint8_t,
        None,
        /// trivial types
        Numeric, Boolean,
        /// strings
        String, StringAtom, UnicodeString,
        /// containers
        Array, Map, Set,
        /// reflected enums
        Enum,
        /// Reflected objects
        Object,
        /// Pointer to reflected object
        Ptr
    );*/

    /*class ValueImpl : public SR_UTILS_NS::NonCopyable, public SR_UTILS_NS::NonMovable, public SR_UTILS_NS::ICloneable {
    public:
        SR_NODISCARD virtual StandardType GetType() const noexcept = 0;

        SR_NODISCARD bool IsReference() const noexcept { return m_pReference != nullptr; }

    protected:
        SR_NODISCARD void* GetReference() const noexcept { return m_pReference; }

    private:
        void* m_pReference = nullptr;

    };

    class ValueImplArray : public ValueImpl {
    private:
        std::vector<ValueImpl*> m_values;
        StandardType m_containerType = StandardType::Unknown;

    };

    class ValueImplMap : public ValueImpl {
    private:
        std::map<SRHashType, ValueImpl*> m_values;
        StandardType m_containerType = StandardType::Unknown;

    };

    class ValueImplVector : public ValueImpl {
    private:
        StandardType m_vectorType = StandardType::Unknown;
        uint8_t m_dimension = 0;

    };*/
}

#endif //SR_UTILS_TYPE_TRAITS_VALUE_IMPL_H
