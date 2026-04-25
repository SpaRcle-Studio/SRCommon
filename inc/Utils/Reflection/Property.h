//
// Created by Monika on 21.01.2025.
//

#if !defined(SR_ENGINE_UTILS_REFLECTION_PROPERTY_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_ENGINE_UTILS_REFLECTION_PROPERTY_H

#include <Utils/Common/StringAtomLiterals.h>
#include <Utils/TypeTraits/TypeTraits.h>
#include <Utils/TypeTraits/SRClass.h>
#include <Utils/Types/RawPointerHolder.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Types/ArrayVector.h>

namespace SR_UTILS_NS::Reflection {
    class Value;

    template<typename T>
    constexpr bool ContainsSRClassV = IsSRClassV<InnerTypeT<T>>;

    class EditorPropertyParams {
    public:
        using EnumFilterFn = bool(*)(SR_UTILS_NS::SRClass* pClass, SR_UTILS_NS::StringAtom enumValue);
        using RangeType = std::optional<std::pair<float_t, float_t>>;

        EditorPropertyParams() = default;

        EditorPropertyParams& SetEnumFilter(EnumFilterFn filter) noexcept { m_enumFilter = filter; return *this; }
        EditorPropertyParams& SetRange(float_t min, float_t max) noexcept { m_range = std::make_pair(min, max); return *this; }
        EditorPropertyParams& SetNoHeader() noexcept { m_noHeader = true; return *this; }
        EditorPropertyParams& SetNotNull() noexcept { m_notNull = true; return *this; }
        EditorPropertyParams& SetDebugOnly() noexcept { m_debugOnly = true; return *this; }
        EditorPropertyParams& SetDisplayName(const StringAtom& displayName) noexcept { m_displayName = displayName; return *this; }
        EditorPropertyParams& SetTooltip(const StringAtom& tooltip) noexcept { m_tooltip = tooltip; return *this; }
        EditorPropertyParams& SetInspector(const StringAtom& inspector) noexcept { m_inspector = inspector; return *this; }
        EditorPropertyParams& SetGroup(const StringAtom& group) noexcept { m_group = group; return *this; }
        EditorPropertyParams& SetDragSpeed(float_t speed) noexcept { m_dragSpeed = speed; return *this; }
        EditorPropertyParams& SetCustomArg(const StringAtom& name, std::string_view value) noexcept {
            for (auto&& arg : m_customArgs) {
                if (arg.name == name) {
                    arg.value = value;
                    return *this;
                }
            }
            m_customArgs.emplace_back({ name, value });
            return *this;
        }

        SR_NODISCARD EnumFilterFn GetEnumFilter() const noexcept { return m_enumFilter; }
        SR_NODISCARD RangeType GetRange() const noexcept { return m_range; }
        SR_NODISCARD StringAtom GetDisplayName() const noexcept { return m_displayName; }
        SR_NODISCARD StringAtom GetTooltip() const noexcept { return m_tooltip; }
        SR_NODISCARD StringAtom GetGroup() const noexcept { return m_group; }
        SR_NODISCARD float_t GetDragSpeed() const noexcept { return m_dragSpeed; }
        SR_NODISCARD float_t GetEditorWidth() const noexcept { return m_editorWidth; }
        SR_NODISCARD StringAtom GetInspector() const noexcept { return m_inspector; }
        SR_NODISCARD bool IsNoHeader() const noexcept { return m_noHeader; }
        SR_NODISCARD bool IsNotNull() const noexcept { return m_notNull; }
        SR_NODISCARD bool IsDebugOnly() const noexcept { return m_debugOnly; }

        SR_NODISCARD std::string_view GetCustomArg(const StringAtom& name) const noexcept {
            for (auto&& arg : m_customArgs) {
                if (arg.name == name) {
                    return arg.value;
                }
            }
            return {};
        }

    private:
        struct CustomArg {
            StringAtom name;
            std::string_view value;
        };

        EnumFilterFn m_enumFilter = nullptr;
        SR_UTILS_NS::StringAtom m_displayName;
        SR_UTILS_NS::StringAtom m_tooltip;
        SR_UTILS_NS::StringAtom m_inspector;
        SR_UTILS_NS::StringAtom m_group;
        float_t m_editorWidth = 0.f;
        float_t m_dragSpeed = 1.f;
        RangeType m_range;
        bool m_noHeader = false;
        bool m_notNull = false;
        bool m_debugOnly = false;
        SR_HTYPES_NS::ArrayVector<CustomArg, 16> m_customArgs;

    };

    enum class PropertySRClassContainsMode {
        NotContains,
        SharedPointer,
        Contains,
        Inner
    };

    class Property {
        using SetCallbackFn = void(*)(SRClass* pOwner, const Value& value);
        using GetCallbackFn = Value(*)(SRClass* pOwner);
        using ChangeCallbackFn = void(*)(SRClass* pOwner);
        using PropertyActiveCallbackFn = bool(*)(SRClass* pOwner);
    public:
        Property() = default;

        SR_NODISCARD Value Get(SRClass* pOwner) const noexcept;
        void Set(SRClass* pOwner, const Value& value) const noexcept { m_setCallback(pOwner, value); }

        void OnChanged(SRClass* pOwner) const noexcept {
            if (m_onChangeCallback) {
                m_onChangeCallback(pOwner);
            }
        }
        SR_NODISCARD StringAtom GetName() const noexcept { return m_name; }
        SR_NODISCARD StringAtom GetSerializeName() const noexcept { return m_serializeName; }
        SR_NODISCARD PropertyPublicity GetPublicity() const noexcept { return m_publicity; }
        SR_NODISCARD const Value& GetDefaultValue() const noexcept;
        SR_NODISCARD const Value& GetResetValue() const noexcept;
        SR_NODISCARD const EditorPropertyParams& GetEditorParams() const noexcept { return m_editorParams; }
        SR_NODISCARD bool HasExplicitSetter() const noexcept { return m_hasExplicitSetter; }

        SR_NODISCARD bool IsActive(SRClass* pOwner) const noexcept {
            if (pOwner && m_propertyActiveCallback) {
                return m_propertyActiveCallback(pOwner);
            }
            return true;
        }

        SR_NODISCARD bool IsHidden(SRClass* pOwner) const noexcept {
            if (!IsActive(pOwner)) {
                return true;
            }
            return m_publicity == PropertyPublicity::Hidden || m_publicity == PropertyPublicity::HiddenReadOnly;
        }

        SR_NODISCARD bool IsReadOnly() const noexcept {
            return m_publicity == PropertyPublicity::ReadOnly || m_publicity == PropertyPublicity::HiddenReadOnly;
        }

        SR_NODISCARD PropertySRClassContainsMode GetSRClassContainsMode() const noexcept { return m_srClassContainsMode; }

        Property& SetName(const StringAtom& name) noexcept { m_name = name; return *this; }
        Property& SetSerializeName(const StringAtom& serializeName) noexcept { m_serializeName = serializeName; return *this; }
        Property& SetPublicity(PropertyPublicity publicity) noexcept { m_publicity = publicity; return *this; }
        Property& SetSetter(SetCallbackFn callback) noexcept { m_setCallback = callback; return *this; }
        Property& SetGetter(GetCallbackFn callback) noexcept { m_getCallback = callback; return *this; }
        Property& SetDefaultValue(Value&& value) noexcept;
        Property& SetChangeCallback(ChangeCallbackFn callback) noexcept { m_onChangeCallback = callback; return *this; }
        Property& SetResetValue(Value&& value) noexcept;
        Property& SetEditorParams(const EditorPropertyParams& params) noexcept { m_editorParams = params; return *this; }
        Property& SetPropertyCondition(PropertyActiveCallbackFn callback) noexcept { m_propertyActiveCallback = callback; return *this; }
        Property& SetHasExplicitSetter(bool hasExplicitSetter) noexcept { m_hasExplicitSetter = hasExplicitSetter; return *this; }

        template<typename T> Property& CheckSRClass() {
            if constexpr (IsSRClassV<T>) {
                m_srClassContainsMode = PropertySRClassContainsMode::Contains;
            }
            else if constexpr (IsSharedPointerV<T>) {
                m_srClassContainsMode = PropertySRClassContainsMode::SharedPointer;
            }
            else if constexpr (ContainsSRClassV<T>) {
                m_srClassContainsMode = PropertySRClassContainsMode::Inner;
            }
            else {
                m_srClassContainsMode = PropertySRClassContainsMode::NotContains;
            }
            return *this;
        }

    private:
        EditorPropertyParams m_editorParams;
        mutable SR_HTYPES_NS::RawPointerHolder<Reflection::Value> m_defaultValue;
        mutable SR_HTYPES_NS::RawPointerHolder<Reflection::Value> m_resetValue;
        SR_UTILS_NS::StringAtom m_name;
        SR_UTILS_NS::StringAtom m_serializeName;
        PropertyPublicity m_publicity = PropertyPublicity::Public;
        SetCallbackFn m_setCallback = nullptr;
        GetCallbackFn m_getCallback = nullptr;
        ChangeCallbackFn m_onChangeCallback = nullptr;
        PropertyActiveCallbackFn m_propertyActiveCallback = nullptr;
        bool m_hasExplicitSetter = false;
        PropertySRClassContainsMode m_srClassContainsMode = PropertySRClassContainsMode::NotContains;
    };

    template<typename T> SR_UTILS_NS::StringAtom GetPropertyInspector() {
        using Type = SR_UTILS_NS::RemoveQualifiersT<T>;

        if constexpr (SR_MATH_NS::IsBool<Type>()) {
            return "Bool"_atom;
        }
        if constexpr (SR_MATH_NS::IsNumber<Type>()) {
            return "Numeric"_atom;
        }
        if constexpr (SR_UTILS_NS::IsMathVector<Type>()) {
            return "MathVector"_atom;
        }
        if constexpr (SR_UTILS_NS::IsMathSize<Type>()) {
            return "Size"_atom;
        }
        if constexpr (SR_UTILS_NS::IsSREnumV<Type>) {
            return "Enum"_atom;
        }
        if constexpr (SR_UTILS_NS::IsSTLVectorV<Type>) {
            return "Vector"_atom;
        }

        return SR_UTILS_NS::StringAtom();
    }

    std::string_view MakeSerializePropertyName(std::string_view name);
    SR_UTILS_NS::StringAtom MakeDisplayName(SR_UTILS_NS::StringAtom name);
}

#endif //SR_ENGINE_UTILS_REFLECTION_PROPERTY_H
