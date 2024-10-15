//
// Created by Monika on 02.04.2024.
//

#ifndef SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H
#define SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H

#include <Utils/Common/Enumerations.h>
#include <Utils/Types/Marshal.h>
#include <Utils/TypeTraits/ClassDB.h>

namespace SR_UTILS_NS {
    class ISerializer {

    };

    class IDeserializer {

    };

    class XMLSerializer : public ISerializer {

    };

    class XMLDeserializer : public IDeserializer {

    };

    class SRClassMeta {
    public:
        virtual ~SRClassMeta() = default;

        virtual void Save(SR_UTILS_NS::ISerializer& serializer) const = 0;
        virtual void Load(SR_UTILS_NS::IDeserializer& deserializer) const = 0;
    };

    struct PropertyInfo {
        StringAtom name;
        StringAtom className;

        bool operator<(const PropertyInfo& p_info) const {
            return name < p_info.name;
        }
    };

    class SRClass {
    public:
        virtual ~SRClass() = default;

    public:
        static SR_UTILS_NS::StringAtom GetClassStaticName() {
            static SR_UTILS_NS::StringAtom name = SR_GET_CLASS_NAME();
            return name;
        }

    private:
        inline static const bool SR_CODEGEN_INITIALIZE = SR_UTILS_NS::ClassDB::Instance().RegisterNewClass(GetClassStaticName());

    };

    template<class T> static T* PostAllocationInitialize(T* pObject) {
        pObject->InitializeClass();
        return pObject;
    }
}

#define SR_CLASS()                                                                                                      \
    public:                                                                                                             \
        static bool RegisterPropertiesCodegen();                                                                        \
    private:                                                                                                            \
        void SR_CLANG_CODEGEN_MARKER() { }                                                                              \
                                                                                                                        \
        template<typename TSerializeType> friend struct ::Codegen::SRClassMetaTemplate;                                 \
        friend class SR_UTILS_NS::SRClass;                                                                              \
                                                                                                                        \
    public:                                                                                                             \
        static const SR_UTILS_NS::SRClassMeta* GetMetaStatic() noexcept;                                                \
        virtual const SR_UTILS_NS::SRClassMeta* GetMeta() const noexcept {                                              \
            auto&& pStaticMetaFromClass = GetMetaStatic();                                                              \
            SRAssert2(pStaticMetaFromClass, std::string("No static meta for: ") + typeid(*this).name());                \
            return pStaticMetaFromClass;                                                                                \
        }                                                                                                               \
                                                                                                                        \
        static SR_UTILS_NS::StringAtom GetClassStaticName() {                                                           \
            static SR_UTILS_NS::StringAtom name = SR_GET_CLASS_NAME();                                                  \
            return name;                                                                                                \
        }                                                                                                               \
    private:                                                                                                            \
        SR_INLINE_STATIC const bool SR_CODEGEN_INITIALIZE =                                                             \
            SR_UTILS_NS::ClassDB::Instance().RegisterNewClass(GetClassStaticName())                                     \
            && RegisterPropertiesCodegen();                                                                             \
    private:


#define SR_CLASS_REGISTER_PROPERTY_BASE(className, propertyName, propertyType)                                          \



template<class T, typename ...Args> static T* SRNew(Args&& ...args) {
    return PostAllocationInitialize(new T(std::forward<Args>(args)...));
}

namespace Codegen {
    template<class T> struct SRClassMetaTemplate : public SR_UTILS_NS::SRClassMeta { };
}

#endif //SR_ENGINE_UTILS_TYPE_TRAITS_SR_CLASS_H