//
// Created by Monika on 21.01.2025.
//

#ifndef SR_ENGINE_UTILS_REFLECTION_PROPERTY_H
#define SR_ENGINE_UTILS_REFLECTION_PROPERTY_H

#include <Utils/Reflection/Value.h>
#include <Utils/Common/StringAtomLiterals.h>

namespace SR_UTILS_NS::Reflection {
    class Property {
        using SetCallbackFn = void(*)(void* pOwner, const Value& value);
        using GetCallbackFn = Value(*)(void* pOwner);
        using ChangeCallbackFn = void(*)(void* pOwner);
    public:
        Property() = default;

        Property(const Property& other)
            : m_defaultValue(other.m_defaultValue.Clone())
            , m_name(other.m_name)
            , m_serializeName(other.m_serializeName)
            , m_displayName(other.m_displayName)
            , m_tooltip(other.m_tooltip)
            , m_inspector(other.m_inspector)
            , m_publicity(other.m_publicity)
            , m_setCallback(other.m_setCallback)
            , m_getCallback(other.m_getCallback)
            , m_onChangeCallback(other.m_onChangeCallback)
        { }

        Property(Property&& other) noexcept
            : m_defaultValue(std::move(other.m_defaultValue))
            , m_name(std::move(other.m_name))
            , m_serializeName(std::move(other.m_serializeName))
            , m_displayName(std::move(other.m_displayName))
            , m_tooltip(std::move(other.m_tooltip))
            , m_inspector(std::move(other.m_inspector))
            , m_publicity(other.m_publicity)
            , m_setCallback(other.m_setCallback)
            , m_getCallback(other.m_getCallback)
            , m_onChangeCallback(other.m_onChangeCallback)
        { }

        Property& operator=(const Property& other) {
            if (this != &other) {
                m_defaultValue = other.m_defaultValue.Clone();
                m_name = other.m_name;
                m_serializeName = other.m_serializeName;
                m_displayName = other.m_displayName;
                m_tooltip = other.m_tooltip;
                m_inspector = other.m_inspector;
                m_publicity = other.m_publicity;
                m_setCallback = other.m_setCallback;
                m_getCallback = other.m_getCallback;
                m_onChangeCallback = other.m_onChangeCallback;
            }
            return *this;
        }

        Property& operator=(Property&& other) noexcept {
            if (this != &other) {
                m_defaultValue = std::move(other.m_defaultValue);
                m_name = std::move(other.m_name);
                m_serializeName = std::move(other.m_serializeName);
                m_displayName = std::move(other.m_displayName);
                m_tooltip = std::move(other.m_tooltip);
                m_inspector = std::move(other.m_inspector);
                m_publicity = other.m_publicity;
                m_setCallback = other.m_setCallback;
                m_getCallback = other.m_getCallback;
                m_onChangeCallback = other.m_onChangeCallback;
            }
            return *this;
        }

        SR_NODISCARD Value Get(void* pOwner) const noexcept { return m_getCallback(pOwner); }
        void Set(void* pOwner, const Value& value) const noexcept { m_setCallback(pOwner, value); }

        SR_NODISCARD void OnChanged(void* pOwner) const noexcept {
            if (m_onChangeCallback) {
                m_onChangeCallback(pOwner);
            }
        }
        SR_NODISCARD StringAtom GetName() const noexcept { return m_name; }
        SR_NODISCARD StringAtom GetDisplayName() const noexcept { return m_displayName; }
        SR_NODISCARD StringAtom GetTooltip() const noexcept { return m_tooltip; }
        SR_NODISCARD StringAtom GetSerializeName() const noexcept { return m_serializeName; }
        SR_NODISCARD StringAtom GetInspector() const noexcept { return m_inspector; }
        SR_NODISCARD PropertyPublicity GetPublicity() const noexcept { return m_publicity; }
        SR_NODISCARD const Value& GetDefaultValue() const noexcept { return m_defaultValue; }
        SR_NODISCARD bool IsHidden() const noexcept {
            return m_publicity == PropertyPublicity::Hidden || m_publicity == PropertyPublicity::HiddenReadOnly;
        }
        SR_NODISCARD bool IsReadOnly() const noexcept {
            return m_publicity == PropertyPublicity::ReadOnly || m_publicity == PropertyPublicity::HiddenReadOnly;
        }

        Property& SetName(const StringAtom& name) noexcept { m_name = name; return *this; }
        Property& SetDisplayName(const StringAtom& displayName) noexcept { m_displayName = displayName; return *this; }
        Property& SetTooltip(const StringAtom& tooltip) noexcept { m_tooltip = tooltip; return *this; }
        Property& SetSerializeName(const StringAtom& serializeName) noexcept { m_serializeName = serializeName; return *this; }
        Property& SetPublicity(PropertyPublicity publicity) noexcept { m_publicity = publicity; return *this; }
        Property& SetSetter(SetCallbackFn callback) noexcept { m_setCallback = callback; return *this; }
        Property& SetGetter(GetCallbackFn callback) noexcept { m_getCallback = callback; return *this; }
        Property& SetDefaultValue(Value&& value) noexcept { m_defaultValue = std::move(value); return *this; }
        Property& SetChangeCallback(ChangeCallbackFn callback) noexcept { m_onChangeCallback = callback; return *this; }
        Property& SetInspector(const StringAtom& inspector) noexcept { m_inspector = inspector; return *this; }

    private:
        Reflection::Value m_defaultValue;
        SR_UTILS_NS::StringAtom m_name;
        SR_UTILS_NS::StringAtom m_serializeName;
        SR_UTILS_NS::StringAtom m_displayName;
        SR_UTILS_NS::StringAtom m_tooltip;
        SR_UTILS_NS::StringAtom m_inspector;
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
        if constexpr(SR_MATH_NS::IsNumber<Type>()) {
            return "Numeric"_atom;
        }
        if constexpr(SR_UTILS_NS::IsMathVector<Type>()) {
            return "Vector"_atom;
        }
        if constexpr(SR_UTILS_NS::IsMathSize<Type>()) {
            return "Size"_atom;
        }

        return SR_UTILS_NS::StringAtom();
    }
}

#endif //SR_ENGINE_UTILS_REFLECTION_PROPERTY_H
