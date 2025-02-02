//
// Created by Monika on 21.01.2025.
//

#ifndef SR_ENGINE_UTILS_REFLECTION_PROPERTY_H
#define SR_ENGINE_UTILS_REFLECTION_PROPERTY_H

#include <Utils/Reflection/Value.h>
#include <Utils/Common/StringAtomLiterals.h>

namespace SR_UTILS_NS::Reflection {
    class EditorPropertyParams {
    public:
        EditorPropertyParams() = default;

        EditorPropertyParams& SetDisplayName(const StringAtom& displayName) noexcept { m_displayName = displayName; return *this; }
        EditorPropertyParams& SetTooltip(const StringAtom& tooltip) noexcept { m_tooltip = tooltip; return *this; }
        EditorPropertyParams& SetInspector(const StringAtom& inspector) noexcept { m_inspector = inspector; return *this; }
        EditorPropertyParams& SetDragSpeed(float_t speed) noexcept { m_dragSpeed = speed; return *this; }

        SR_NODISCARD StringAtom GetDisplayName() const noexcept { return m_displayName; }
        SR_NODISCARD StringAtom GetTooltip() const noexcept { return m_tooltip; }
        SR_NODISCARD float_t GetDragSpeed() const noexcept { return m_dragSpeed; }
        SR_NODISCARD float_t GetEditorWidth() const noexcept { return m_editorWidth; }
        SR_NODISCARD StringAtom GetInspector() const noexcept { return m_inspector; }

    private:
        SR_UTILS_NS::StringAtom m_displayName;
        SR_UTILS_NS::StringAtom m_tooltip;
        SR_UTILS_NS::StringAtom m_inspector;
        float_t m_editorWidth = 0.f;
        float_t m_dragSpeed = 1.f;

    };

    class Property {
        using SetCallbackFn = void(*)(void* pOwner, const Value& value);
        using GetCallbackFn = Value(*)(void* pOwner);
        using ChangeCallbackFn = void(*)(void* pOwner);
    public:
        Property() = default;

        //Property(const Property& other)
        //    : m_defaultValue(other.m_defaultValue.Clone())
        //    , m_name(other.m_name)
        //    , m_serializeName(other.m_serializeName)
        //    , m_editorParams(other.m_editorParams)
        //    , m_publicity(other.m_publicity)
        //    , m_setCallback(other.m_setCallback)
        //    , m_getCallback(other.m_getCallback)
        //    , m_onChangeCallback(other.m_onChangeCallback)
        //    , m_resetValue(other.m_resetValue.Clone())
        //{ }

        //Property(Property&& other) noexcept
        //    : m_defaultValue(std::move(other.m_defaultValue))
        //    , m_name(std::move(other.m_name))
        //    , m_serializeName(std::move(other.m_serializeName))
        //    , m_editorParams(std::move(other.m_editorParams))
        //    , m_publicity(other.m_publicity)
        //    , m_setCallback(other.m_setCallback)
        //    , m_getCallback(other.m_getCallback)
        //    , m_onChangeCallback(other.m_onChangeCallback)
        //    , m_resetValue(std::move(other.m_resetValue))
        //{ }

        //Property& operator=(const Property& other) {
        //    if (this != &other) {
        //        m_defaultValue = other.m_defaultValue.Clone();
        //        m_name = other.m_name;
        //        m_serializeName = other.m_serializeName;
        //        m_editorParams = other.m_editorParams;
        //        m_publicity = other.m_publicity;
        //        m_setCallback = other.m_setCallback;
        //        m_getCallback = other.m_getCallback;
        //        m_onChangeCallback = other.m_onChangeCallback;
        //        m_resetValue = other.m_resetValue.Clone();
        //    }
        //    return *this;
        //}

        //Property& operator=(Property&& other) noexcept {
        //    if (this != &other) {
        //        m_defaultValue = std::move(other.m_defaultValue);
        //        m_name = std::move(other.m_name);
        //        m_serializeName = std::move(other.m_serializeName);
        //        m_editorParams = std::move(other.m_editorParams);
        //        m_publicity = other.m_publicity;
        //        m_setCallback = other.m_setCallback;
        //        m_getCallback = other.m_getCallback;
        //        m_onChangeCallback = other.m_onChangeCallback;
        //        m_resetValue = std::move(other.m_resetValue);
        //    }
        //    return *this;
        //}

        SR_NODISCARD Value Get(void* pOwner) const noexcept { return m_getCallback(pOwner); }
        void Set(void* pOwner, const Value& value) const noexcept { m_setCallback(pOwner, value); }

        void OnChanged(void* pOwner) const noexcept {
            if (m_onChangeCallback) {
                m_onChangeCallback(pOwner);
            }
        }
        SR_NODISCARD StringAtom GetName() const noexcept { return m_name; }
        SR_NODISCARD StringAtom GetSerializeName() const noexcept { return m_serializeName; }
        SR_NODISCARD PropertyPublicity GetPublicity() const noexcept { return m_publicity; }
        SR_NODISCARD const Value& GetDefaultValue() const noexcept { return m_defaultValue; }
        SR_NODISCARD const Value& GetResetValue() const noexcept { return m_resetValue; }
        SR_NODISCARD const EditorPropertyParams& GetEditorParams() const noexcept { return m_editorParams; }

        SR_NODISCARD bool IsHidden() const noexcept {
            return m_publicity == PropertyPublicity::Hidden || m_publicity == PropertyPublicity::HiddenReadOnly;
        }
        SR_NODISCARD bool IsReadOnly() const noexcept {
            return m_publicity == PropertyPublicity::ReadOnly || m_publicity == PropertyPublicity::HiddenReadOnly;
        }

        Property& SetName(const StringAtom& name) noexcept { m_name = name; return *this; }
        Property& SetSerializeName(const StringAtom& serializeName) noexcept { m_serializeName = serializeName; return *this; }
        Property& SetPublicity(PropertyPublicity publicity) noexcept { m_publicity = publicity; return *this; }
        Property& SetSetter(SetCallbackFn callback) noexcept { m_setCallback = callback; return *this; }
        Property& SetGetter(GetCallbackFn callback) noexcept { m_getCallback = callback; return *this; }
        Property& SetDefaultValue(Value&& value) noexcept { m_defaultValue = std::move(value); return *this; }
        Property& SetChangeCallback(ChangeCallbackFn callback) noexcept { m_onChangeCallback = callback; return *this; }
        Property& SetResetValue(Value&& value) noexcept { m_resetValue = std::move(value); return *this; }
        Property& SetEditorParams(const EditorPropertyParams& params) noexcept { m_editorParams = params; return *this; }

    private:
        EditorPropertyParams m_editorParams;
        Reflection::Value m_defaultValue;
        Reflection::Value m_resetValue;
        SR_UTILS_NS::StringAtom m_name;
        SR_UTILS_NS::StringAtom m_serializeName;
        PropertyPublicity m_publicity = PropertyPublicity::Public;
        SetCallbackFn m_setCallback = nullptr;
        GetCallbackFn m_getCallback = nullptr;
        ChangeCallbackFn m_onChangeCallback = nullptr;
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
}

#endif //SR_ENGINE_UTILS_REFLECTION_PROPERTY_H
