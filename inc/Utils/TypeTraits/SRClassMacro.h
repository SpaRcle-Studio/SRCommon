//
// Created by Monika on 30.11.2025.
//

#ifndef SR_ENGINE_UTILS_SR_CLASS_MACRO_H
#define SR_ENGINE_UTILS_SR_CLASS_MACRO_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class SRClassMeta;
    class StringAtom;
    class SRClass;
}

namespace Codegen {
    template<class T> struct SRClassMetaTemplate;
}

#define SR_CLASS_BASE()                                                                                                 \
    private:                                                                                                            \
        template<typename TSerializeType> friend struct ::Codegen::SRClassMetaTemplate;                                 \
        friend class SR_UTILS_NS::SRClass;                                                                              \
                                                                                                                        \
    public:                                                                                                             \
        static const SR_UTILS_NS::SRClassMeta* GetMetaStatic() noexcept;                                                \
        const SR_UTILS_NS::SRClassMeta* GetMeta() const noexcept override {                                             \
            auto&& pStaticMetaFromClass = GetMetaStatic();                                                              \
            SRAssert2(pStaticMetaFromClass, std::string("No static meta for: ") + typeid(*this).name());                \
            return pStaticMetaFromClass;                                                                                \
        }                                                                                                               \
                                                                                                                        \
        static SR_UTILS_NS::StringAtom GetClassStaticName() noexcept;                                                   \
        static SRClass* AllocateStatic() noexcept;                                                                      \


#define SR_CLASS()                                                                                                      \
        SR_CLASS_BASE()                                                                                                 \
    private:                                                                                                            \


#define SR_STRUCT()                                                                                                     \
        SR_CLASS_BASE()                                                                                                 \
    public:                                                                                                             \


#define SR_CLASS_REGISTER_PROPERTY_BASE(className, propertyName, propertyType)                                          \

#define SR_VIRTUAL_PROPERTY                                                                                             \
    typedef bool SR_COMBINE(SR_CODEGEN_VIRTUAL_PROP_LINE_, __LINE__);                                                   \

//#define SR_VIRTUAL_PROPERTY void SR_COMBINE(SR_CODEGEN_VIRTUAL_PROP_ANCHOR_LINE_, __LINE__)() {}

#endif //SR_ENGINE_UTILS_SR_CLASS_MACRO_H
