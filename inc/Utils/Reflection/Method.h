//
// Created by Monika on 31.01.2026.
//

#ifndef SR_ENGINE_UTILS_REFLECTION_METHOD_H
#define SR_ENGINE_UTILS_REFLECTION_METHOD_H

#include <Utils/TypeTraits/TypeTraits.h>
#include <Utils/TypeTraits/SRClass.h>
#include <Utils/Reflection/Value.h>

namespace SR_UTILS_NS::Reflection {
    class Method {
        struct Parameter {
            StringAtom name;
            TypeInfo* pTypeInfo = nullptr;
        };
    public:
        using Owner = SRClass;
        using Params = std::span<Value*>;
        using FunctorNoReturnNoParams = void(*)(Owner&);
        using FunctorNoReturnWithParams = void(*)(Owner&, const Params&);
        using FunctorWithReturnNoParams = Value(*)(Owner&);
        using FunctorWithReturnWithParams = Value(*)(Owner&, const Params&);
        using MethodActiveCallbackFn = bool(*)(SRClass* pOwner);
    public:
        Method() = default;
        Method(const Method& other);
        Method(Method&& other) noexcept = default;
        Method& operator=(const Method& other);
        Method& operator=(Method&& other) noexcept = default;
        ~Method();

        SR_NODISCARD const Parameter& GetParam(uint32_t index) const;
        SR_NODISCARD StringAtom GetName() const;
        SR_NODISCARD StringAtom GetDisplayName() const;
        SR_NODISCARD uint32_t GetParamsCount() const;
        SR_NODISCARD bool HasReturn() const;
        SR_NODISCARD bool IsEditorButton() const;
        SR_NODISCARD bool IsActive(Owner& owner) const;
        SR_NODISCARD bool IsEvaluate() const;
        SR_NODISCARD const TypeInfo* GetReturnType() const;

        void InvokeVoid(Owner& owner) const;
        void InvokeVoid(Owner& owner, const Params& params) const;
        Value Invoke(Owner& owner, const Params& params) const;
        Value Invoke(Owner& owner) const;

        Method& SetNoReturnNoParams(const FunctorNoReturnNoParams& func);
        Method& SetNoReturnWithParams(const FunctorNoReturnWithParams& func);
        Method& SetWithReturnNoParams(const FunctorWithReturnNoParams& func);
        Method& SetWithReturnWithParams(const FunctorWithReturnWithParams& func);
        Method& SetName(StringAtom name);
        Method& SetReturnType(TypeInfo* pReturnTypeInfo);
        Method& SetEditorButton();
        Method& SetCondition(MethodActiveCallbackFn condition);
        Method& AddParam(StringAtom name, TypeInfo* pTypeInfo);
        Method& SetEvaluate();

    private:
        union {
            FunctorNoReturnNoParams m_noReturnNoParams;
            FunctorNoReturnWithParams m_noReturnWithParams;
            FunctorWithReturnNoParams m_withReturnNoParams;
            FunctorWithReturnWithParams m_withReturnWithParams;
        };
        MethodActiveCallbackFn m_methodActiveCallback = nullptr;

        StringAtom m_name;
        Vector<Parameter> m_params;
        mutable StringAtom m_displayName;
        TypeInfo* m_pReturnTypeInfo = nullptr;
        bool m_isEditorButton = false;
        bool m_isEvaluate = false;

    };

    void InvokeMethodVoid(Method::Owner& owner, StringAtom name);
    void InvokeMethodVoid(Method::Owner& owner, StringAtom name, const Method::Params& params);
    Value InvokeMethod(Method::Owner& owner, StringAtom name, const Method::Params& params);
    Value InvokeMethod(Method::Owner& owner, StringAtom name);
}

#endif //SR_ENGINE_UTILS_REFLECTION_METHOD_H
