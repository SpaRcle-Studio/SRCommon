//
// Created by Monika on 01.10.2021.
//

#ifndef SR_ENGINE_ENUMERATIONS_H
#define SR_ENGINE_ENUMERATIONS_H

#include <Utils/Common/EnumReflector.h>

/// -----------------------------------------[Базовые с настройкой типа]------------------------------------------------

/// Declare an enumeration inside a class
#define SR_ENUM_T(enumName, type, ...)                                                                                  \
      SR_ENUM_DETAIL_MAKE(SR_UTILS_NS::EnumVariant::List, enum, class, enumName, #enumName, type, __VA_ARGS__ )         \
      class CodegenEnumIncludedChecked_##enumName {};                                                                   \
      //template<> struct CodegenEnumIncludedChecked_##enumName<enumName> : std::true_type {};                            \

/// Declare an enumeration inside a class
#define SR_ENUM_CLASS_T(enumName, type, ...)                                                                            \
      SR_ENUM_DETAIL_MAKE(SR_UTILS_NS::EnumVariant::List, enum class, class, enumName, #enumName, type, __VA_ARGS__ )   \
      class CodegenEnumIncludedChecked_##enumName {};                                                                   \
      //template<> struct CodegenEnumIncludedChecked_##enumName<enumName> : std::true_type {};                            \

/// Declare an enumeration inside a namespace
#define SR_ENUM_NS_T(enumName, type, ...)                                                                               \
      SR_ENUM_DETAIL_MAKE(SR_UTILS_NS::EnumVariant::List, enum, namespace, enumName, #enumName, type, __VA_ARGS__ )     \
      class CodegenEnumIncludedChecked_##enumName {};                                                                   \
      //template<> struct CodegenEnumIncludedChecked_##enumName<enumName> : std::true_type {};                            \

/// Declare an enumeration inside a namespace
#define SR_ENUM_NS_CLASS_T(enumName, type, ...)                                                                         \
      SR_ENUM_DETAIL_MAKE(SR_UTILS_NS::EnumVariant::List, enum class,                                                   \
            namespace, enumName, #enumName, type, __VA_ARGS__ )                                                         \
      typedef type SR_MACRO_CONCAT(enumName, Flag);                                                                     \
      class CodegenEnumIncludedChecked_##enumName {};                                                                   \
      //template<> struct CodegenEnumIncludedChecked_##enumName<enumName> : std::true_type {};                            \

/// Declare an enumeration inside a namespace
//#define SR_ENUM_NS_STRUCT_T_OLD(enumName, type, ...)                                                                  \
//      struct enumName : public SR_UTILS_NS::IEnumStructBase {                                                         \
//            SR_ENUM_DETAIL_MAKE(SR_UTILS_NS::EnumVariant::Flags, enum, class,                                         \
//                  SR_MACRO_CONCAT(enumName, T), #enumName, type, __VA_ARGS__ )                                        \
//            using TypeT = SR_MACRO_CONCAT(enumName, T);                                                               \
//            operator type() const noexcept { return TypeT(); }                                                        \
//      };                                                                                                              \
//      typedef type SR_MACRO_CONCAT(enumName, Flag);                                                                   \

/// Declare an enumeration inside a namespace
#define SR_ENUM_NS_STRUCT_T(enumName, type, ...)                                                                        \
      namespace SR_MACRO_CONCAT(enumName, EnumWrappper) { enum enumName : type; }                                       \
      struct SR_MACRO_CONCAT(enumName, EnumAliasWrappper) {                                                             \
            using enumName = SR_MACRO_CONCAT(enumName, EnumWrappper)::enumName; };                                      \
      using enumName = SR_MACRO_CONCAT(enumName, EnumAliasWrappper)::enumName;                                          \
      namespace SR_MACRO_CONCAT(enumName, EnumWrappper) {                                                               \
            SR_ENUM_DETAIL_MAKE(SR_UTILS_NS::EnumVariant::Flags, enum, namespace,                                       \
                  enumName, #enumName, type, __VA_ARGS__ )                                                              \
      }                                                                                                                 \
      class CodegenEnumIncludedChecked_##enumName {};                                                                   \
      //template<> struct CodegenEnumIncludedChecked_##enumName<enumName> : std::true_type {};                            \

      //inline enumName operator(int lhs) { return static_cast<enumName>(lhs); }                                          \
      //inline enumName operator(long lhs) { return static_cast<enumName>(lhs); }                                         \

/// ---------------------------------------[Для объявления внутри классов]----------------------------------------------

#define SR_ENUM_CLASS(enumName, ...) SR_ENUM_CLASS_T(enumName, int32_t, __VA_ARGS__)
#define SR_ENUM(enumName, ...) SR_ENUM_T(enumName, int32_t, __VA_ARGS__)

/// ----------------------------------[Для объявления внутри пространств имен]------------------------------------------

#define SR_ENUM_NS_CLASS(enumName, ...) SR_ENUM_NS_CLASS_T(enumName, int32_t, __VA_ARGS__)
#define SR_ENUM_NS(enumName, ...) SR_ENUM_NS_T(enumName, int32_t, __VA_ARGS__)

/// --------------------------------------------------------------------------------------------------------------------

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(BoolExt, int8_t,
        None  = -1,
        False = 0,
        True  = 1
    );

    SR_ENUM_NS_CLASS_T(PropertyPublicity, uint8_t,
        Public, /// любой может читать и писать
        Hidden, /// скрытое свойство, но можно читать и писать
        ReadOnly, /// только читать
        HiddenReadOnly, /// скрытое свойство, только читать
        Private /// только для сериализации
    );
}

#endif //SR_ENGINE_ENUMERATIONS_H
