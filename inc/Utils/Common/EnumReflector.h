//
// Created by Monika on 09.09.2022.
//

#if !defined(SR_ENGINE_ENUM_REFLECTOR_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_ENGINE_ENUM_REFLECTOR_H

#include <Utils/Types/StringAtom.h>
#include <Utils/Types/Function.h>
#include <Utils/Types/Vector.h>
#include <Utils/Types/FlatHashMap.h>
#include <Utils/Common/NonCopyable.h>

template <typename EnumType> struct EnumSelector {};
template<typename T> constexpr SR_UTILS_NS::EnumVariant GetEnumVariant(T) noexcept;
template<typename T> constexpr SR_UTILS_NS::SizeType GetEnumItemsCount(T) noexcept;

namespace SR_UTILS_NS {
    class EnumReflector;

    class EnumReflectorManager : public SR_UTILS_NS::NonCopyable {
        using Reflectors = SR_HTYPES_NS::FlatHashMap<SR_UTILS_NS::StringAtom, EnumReflector*>;
    public:
        EnumReflectorManager() = default;

        static EnumReflectorManager& Instance();

        void RegisterReflector(EnumVariant enumVariant, const void* pData, uint64_t typeSize, uint64_t count, const char* name, const char* body);
        void UnregisterReflector(const char* name);

        SR_NODISCARD EnumReflector* GetReflector(SR_UTILS_NS::StringAtom name) const;
        SR_NODISCARD const Reflectors& GetReflectors() const noexcept { return m_reflectors; }

    private:
        Reflectors m_reflectors;

    };

    template<typename EnumType> struct EnumTraits {
    public:
        using EnumResultType = typename std::conditional_t<std::is_enum_v<EnumType>, EnumType, void>;
        static constexpr bool IsDeclaredInNamespace = (GetEnumVariant(static_cast<const EnumResultType*>(nullptr)) != EnumVariant::Undefined);
        using EnumSelectorType = typename std::conditional_t<IsDeclaredInNamespace, const EnumResultType*, EnumSelector<EnumType>>;
        static constexpr bool IsEnum = GetEnumVariant(EnumSelectorType{}) != EnumVariant::Undefined;
        static constexpr bool IsFlags = GetEnumVariant(EnumSelectorType{}) == EnumVariant::Flags;
        static constexpr SizeType NumItems = GetEnumItemsCount(EnumSelectorType{});
    };

    class SR_COMMON_DLL_API EnumReflector : public NonCopyable {
    public:
        struct Enumerator {
            SR_UTILS_NS::StringAtom name;
            uint64_t hashName = 0;
            int64_t value = 0;
        };

    public:
        EnumReflector(EnumVariant enumVariant, const void* pData, uint64_t typeSize, uint64_t count, SR_UTILS_NS::StringAtom name, const char* body);
        ~EnumReflector() override;

    public:
        template<typename EnumType> static void ForEach(SR_HTYPES_NS::Function<void(EnumType)> func);

        template<typename EnumType> SR_NODISCARD static EnumReflector* GetReflector();
        template<typename EnumType> SR_NODISCARD static uint64_t Count();
        template<typename EnumType> SR_NODISCARD static SR_UTILS_NS::StringAtom ToStringAtom(EnumType value);
        template<typename EnumType> SR_NODISCARD static SR_UTILS_NS::StringAtom ToStringAtom(int64_t value);
        template<typename EnumType> SR_NODISCARD static EnumType FromString(const SR_UTILS_NS::StringAtom& value);
        template<typename EnumType> SR_NODISCARD static bool FromString(const SR_UTILS_NS::StringAtom& value, EnumType& result);
        template<typename EnumType> SR_NODISCARD static EnumType FromStringLowerCase(const std::string& value);
        template<typename EnumType> SR_NODISCARD static uint64_t GetIntegralTypeSize();

        template<typename EnumType> SR_NODISCARD static const Vector<StringAtom>& GetNames();
        template<typename EnumType> SR_NODISCARD static const Vector<Enumerator>& GetValues();
        template<typename EnumType> SR_NODISCARD static Vector<StringAtom> GetNamesFilter(const SR_HTYPES_NS::Function<bool(EnumType)>& filter);

        template<typename EnumType> SR_NODISCARD static int64_t GetIndex(EnumType value);
        template<typename EnumType> SR_NODISCARD static int64_t GetIndex(int64_t value);
        template<typename EnumType> SR_NODISCARD static EnumType At(uint64_t index);
        template<typename EnumType> SR_NODISCARD static int64_t AtAsInt(uint64_t index);
        template<typename EnumType> SR_NODISCARD static int64_t AsInt(EnumType value);
        template<typename EnumType> SR_NODISCARD static StringAtom GetName();
        template<typename EnumType> SR_NODISCARD static EnumVariant GetEnumVariant();

        SR_MAYBE_UNUSED SR_NODISCARD std::optional<StringAtom> ToStringInternal(int64_t value) const;
        SR_MAYBE_UNUSED SR_NODISCARD std::optional<int64_t> FromStringInternal(const StringAtom& name) const;
        SR_MAYBE_UNUSED SR_NODISCARD std::optional<int64_t> FromStringLowerCaseInternal(const std::string& value) const;
        SR_MAYBE_UNUSED SR_NODISCARD std::optional<int64_t> GetIndexInternal(int64_t value) const;
        SR_MAYBE_UNUSED SR_NODISCARD std::optional<int64_t> AtInternal(uint64_t index) const;
        SR_MAYBE_UNUSED SR_NODISCARD const Vector<StringAtom>& GetNamesInternal() const;
        SR_MAYBE_UNUSED SR_NODISCARD const StringAtom& GetNameInternal() const;
        SR_MAYBE_UNUSED SR_NODISCARD uint64_t GetIntegralTypeSizeInternal() const;
        SR_MAYBE_UNUSED SR_NODISCARD int64_t ReadEnumValueFromPointerInternal(const void* pEnum) const;
        SR_MAYBE_UNUSED SR_NODISCARD EnumVariant GetEnumVariantInternal() const;
        SR_MAYBE_UNUSED void WriteEnumValueToPointerInternal(void* pEnum, int64_t value) const;

    private:
        static bool IsIdentChar(char c);
        static void ErrorInternal(const std::string& msg);
        static uint64_t GetAndRegisterHash(const char* name);

    private:
        struct Data
        {
            Vector<Enumerator> values;
            Vector<StringAtom> names;
            StringAtom enumName;
        }* m_data;

        uint64_t m_integralTypeSize = 0;
        EnumVariant m_enumVariant = EnumVariant::Undefined;
    };
}

namespace SR_UTILS_NS {
    template<typename EnumType> SR_UTILS_NS::StringAtom EnumReflector::ToStringAtom(EnumType value) {
        if (auto&& result = GetReflector<EnumType>()->ToStringInternal(static_cast<int64_t>(value)); result.has_value()) {
            return result.value();
        }

        ErrorInternal("EnumReflector::ToStringAtom() : unknown type! Value: " + std::to_string(static_cast<int64_t>(value)));

        return SR_UTILS_NS::StringAtom(); /// NOLINT
    }

    template<typename EnumType> SR_UTILS_NS::StringAtom EnumReflector::ToStringAtom(int64_t value) {
        if (auto&& result = GetReflector<EnumType>()->ToStringInternal(value); result.has_value()) {
            return result.value();
        }

        ErrorInternal("EnumReflector::ToStringAtom() : unknown type! Value: " + std::to_string(value));

        return SR_UTILS_NS::StringAtom(); /// NOLINT
    }

    template<typename EnumType> EnumType EnumReflector::FromString(const SR_UTILS_NS::StringAtom& value) {
        if (auto&& result = GetReflector<EnumType>()->FromStringInternal(value); result.has_value()) {
            return static_cast<EnumType>(result.value());
        }

        ErrorInternal("EnumReflector::FromString() : unknown type! Value: " + value.ToStringRef());

        return static_cast<EnumType>(0);
    }

    template<typename EnumType> bool EnumReflector::FromString(const SR_UTILS_NS::StringAtom& value, EnumType& result) {
        if (auto&& res = GetReflector<EnumType>()->FromStringInternal(value); res.has_value()) {
            result = static_cast<EnumType>(res.value());
            return true;
        }
        return false;
    }

    template<typename EnumType> const Vector<StringAtom>& EnumReflector::GetNames() {
        return GetReflector<EnumType>()->m_data->names;
    }

    template<typename EnumType> const Vector<EnumReflector::Enumerator>& EnumReflector::GetValues() {
        return GetReflector<EnumType>()->m_data->values;
    }

    template<typename EnumType> Vector<StringAtom> EnumReflector::GetNamesFilter(const SR_HTYPES_NS::Function<bool(EnumType)> &filter) {
        Vector<StringAtom> names;

        auto&& data = GetReflector<EnumType>()->m_data;

        for (uint64_t i = 0; i < Count<EnumType>(); ++i) {
            if (filter(data->values[i])) {
                names.emplace_back(data->names[i]);
            }
        }

        return names;
    }

    template<typename EnumType> int64_t EnumReflector::AsInt(EnumType value) {
        return static_cast<int64_t>(value);
    }

    template<typename EnumType> int64_t EnumReflector::GetIndex(EnumType value) {
        return GetIndex<EnumType>(static_cast<int64_t>(value));
    }

    template<typename EnumType> EnumType EnumReflector::FromStringLowerCase(const std::string &value) {
        if (auto&& result = GetReflector<EnumType>()->FromStringLowerCaseInternal(value); result.has_value()) {
            return static_cast<EnumType>(result.value());
        }
        ErrorInternal("EnumReflector::FromStringLowerCase() : unknown type! Value: " + value);
        return static_cast<EnumType>(0);
    }

    template<typename EnumType> uint64_t EnumReflector::GetIntegralTypeSize() {
        return GetReflector<EnumType>()->GetIntegralTypeSizeInternal();
    }

    template<typename EnumType> int64_t EnumReflector::GetIndex(int64_t value) {
        if (auto&& result = GetReflector<EnumType>()->GetIndexInternal(value); result.has_value()) {
            return result.value();
        }
        ErrorInternal("EnumReflector::GetIndex() : unknown type! Value: " + std::to_string(static_cast<int64_t>(value)));
        return SR_ID_INVALID;
    }

    template<typename EnumType> EnumType EnumReflector::At(uint64_t index) {
        return static_cast<EnumType>(AtAsInt<EnumType>(index));
    }

    template<typename EnumType> int64_t EnumReflector::AtAsInt(uint64_t index) {
        if (auto&& result = GetReflector<EnumType>()->AtInternal(index); result.has_value()) {
            return result.value();
        }
        ErrorInternal("EnumReflector::At() : invalid index! Index: " + std::to_string(static_cast<int64_t>(index)));
        return 0;
    }

    template<typename EnumType> uint64_t EnumReflector::Count() {
        return GetReflector<EnumType>()->m_data->values.size();
    }

    template<typename EnumType> SR_UTILS_NS::StringAtom EnumReflector::GetName() {
        return GetReflector<EnumType>()->GetNameInternal();
    }

    template<typename EnumType> EnumVariant EnumReflector::GetEnumVariant() {
        return GetReflector<EnumType>()->GetEnumVariantInternal();
    }

    template <typename EnumType>
    void EnumReflector::ForEach(Types::Function<void(EnumType)> func) {
        for (auto&& item : GetReflector<EnumType>()->m_data->values) {
            func(static_cast<EnumType>(item.value));
        }
    }

    template<typename EnumType> EnumReflector* EnumReflector::GetReflector() {
        if constexpr (std::is_class_v<EnumType>) {
            if constexpr (std::is_enum_v<EnumType>) {
                return EnumReflectorManager::Instance().GetReflector(GetEnumName(EnumType()));
            }
            else {
                return EnumReflectorManager::Instance().GetReflector(GetEnumName(EnumType::TypeT()));
            }
        }
        else {
            return EnumReflectorManager::Instance().GetReflector(GetEnumName(EnumType()));
        }
    }
}

#define SR_ENUM_DETAIL_SPEC_namespace \
    extern "C"{/* Protection from being used inside a class body */} \
    SR_INLINE
#define SR_ENUM_DETAIL_SPEC_class friend
#define SR_ENUM_DETAIL_STR(x) #x
#define SR_ENUM_DETAIL_MAKE(enumVariant, enumClass, spec, enumName, enumNameStr, integral, ...)                         \
    enumClass enumName : integral {                                                                                     \
        __VA_ARGS__, SR_MACRO_CONCAT(enumName, MAX)                                                                     \
    };                                                                                                                  \
    struct CODEGEN_ENUM_DETAILS_STRUCT_##enumName {                                                                     \
        SR_UTILS_NS::EnumVariant CODEGEN_ENUM_VARIANT = enumVariant;                                                    \
        uint64_t CODEGEN_ENUM_COUNT = SR_COUNT_ARGS(__VA_ARGS__);                                                       \
        const char* CODEGEN_ENUM_NAME = SR_EXPAND_AND_STRINGIFY(enumName);                                              \
        const char* CODEGEN_ENUM_VA_ARGS = SR_ENUM_DETAIL_STR((__VA_ARGS__));                                           \
        const char* CODEGEN_ENUM_TYPE = SR_EXPAND_AND_STRINGIFY(integral);                                              \
        const char* CODEGEN_ENUM_CLASS = SR_EXPAND_AND_STRINGIFY(enumClass);                                            \
    };                                                                                                                  \
    inline constexpr SR_UTILS_NS::EnumVariant GetEnumVariant(const enumName*) noexcept {                                \
        return enumVariant;                                                                                             \
    }                                                                                                                   \
    inline constexpr SR_UTILS_NS::SizeType GetEnumItemsCount(const enumName*) noexcept {                                \
        return SR_COUNT_ARGS(__VA_ARGS__);                                                                              \
    }                                                                                                                   \
    SR_ENUM_DETAIL_SPEC_##spec SR_UTILS_NS::StringAtom GetEnumName(enumName) noexcept {                                 \
        static const SR_UTILS_NS::StringAtom name(SR_EXPAND_AND_STRINGIFY(enumName));                                   \
        return name;                                                                                                    \
    }                                                                                                                   \

#endif //SR_ENGINE_ENUMREFLECTOR_H
