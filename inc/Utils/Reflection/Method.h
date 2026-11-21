//
// Created by Monika on 31.01.2026.
//

#ifndef SR_ENGINE_UTILS_REFLECTION_METHOD_H
#define SR_ENGINE_UTILS_REFLECTION_METHOD_H

#include <Utils/TypeTraits/TypeTraits.h>
#include <Utils/TypeTraits/SRClass.h>
#include <Utils/Reflection/Value.h>

namespace SR_UTILS_NS::Reflection {
    class SR_COMMON_DLL_API Method {
        struct Parameter {
            StringAtom name;
            const TypeInfo* pTypeInfo = nullptr;
            /// TypeInfo хранит тип со снятыми cv-квалификаторами и ссылкой, поэтому способ
            /// передачи параметра приходится запоминать отдельно
            bool isReference = false;
            bool isConst = false;

            /// параметр принимается по неконстантной ссылке, то есть метод может его изменить
            SR_NODISCARD bool IsOutput() const noexcept { return isReference && !isConst; }
        };
    public:
        using Owner = SRClass;
        using Params = std::span<Value*>;
        typedef void(*Thunk)(Owner&, void* const*, void*);
        using MethodActiveCallbackFn = bool(*)(SRClass* pOwner);
    public:
        Method() = default;
        Method(const Method& other) = default;
        Method(Method&& other) noexcept = default;
        Method& operator=(const Method& other) = default;
        Method& operator=(Method&& other) noexcept = default;
        ~Method() = default;

        SR_NODISCARD const Parameter& GetParam(uint32_t index) const;
        SR_NODISCARD StringAtom GetName() const;
        SR_NODISCARD StringAtom GetDisplayName() const;
        SR_NODISCARD uint32_t GetParamsCount() const;
        SR_NODISCARD bool HasReturn() const;
        SR_NODISCARD bool IsEditorButton() const;
        SR_NODISCARD bool IsActive(Owner& owner) const;
        SR_NODISCARD bool IsEvaluate() const;
        /// принимается ли параметр по неконстантной ссылке - такой параметр является выходным
        SR_NODISCARD bool IsOutputParam(uint32_t index) const;
        SR_NODISCARD const TypeInfo* GetReturnType() const;

        void InvokeVoid(Owner& owner) const;
        void InvokeVoid(Owner& owner, const Params& params) const;
        Value Invoke(Owner& owner, const Params& params) const;
        Value Invoke(Owner& owner) const;

        /// Value results are assigned into a live object; reference results write a const void*.
        void InvokeRaw(Owner& owner, void* const* pArgs, void* pReturn) const;
        void InvokeInto(Owner& owner, void* const* pArgs, Value& result) const;
        SR_NODISCARD void* GetArgumentData(Value& value, uint32_t index) const;
        SR_NODISCARD bool IsReturnsReference() const { return m_returnsReference; }
        SR_NODISCARD bool IsReturnsConstReference() const;
        SR_NODISCARD bool IsDontPack() const { return m_dontPack; }
        SR_NODISCARD bool IsConst() const noexcept { return m_isConst; }
        Method& SetConstMethod(bool value) noexcept {
            m_isConst = value;
            return *this;
        }
        Method& SetThunk(Thunk pThunk);
        Method& SetReturnReference(bool isReference, bool isConst);
        Method& SetDontPack();

        template<typename R, typename F> static void StoreResult(void* pReturn, F&& function) {
            if constexpr (std::is_reference_v<R>) {
                *static_cast<const void**>(pReturn) = std::addressof(function());
            }
            else {
                static_assert(std::is_default_constructible_v<std::remove_cv_t<R>>,
                    "Reflected value returns require a default constructor.");
                static_assert(std::is_assignable_v<std::remove_cv_t<R>&, R>,
                    "Reflected value returns require assignment.");
                *static_cast<std::remove_cv_t<R>*>(pReturn) = function();
            }
        }

        Method& SetName(StringAtom name);
        Method& SetReturnType(const TypeInfo* pReturnTypeInfo);
        Method& SetEditorButton();
        Method& SetCondition(MethodActiveCallbackFn condition);
        Method& AddParam(StringAtom name, const TypeInfo* pTypeInfo, bool isReference = false, bool isConst = false);
        Method& SetEvaluate();

    private:
        Thunk m_pThunk = nullptr;
        MethodActiveCallbackFn m_methodActiveCallback = nullptr;

        StringAtom m_name;
        Vector<Parameter> m_params;
        mutable StringAtom m_displayName;
        const TypeInfo* m_pReturnTypeInfo = nullptr;
        bool m_isEditorButton = false;
        bool m_isEvaluate = false;
        bool m_returnsReference = false;
        bool m_returnIsConst = false;
        bool m_dontPack = false;
        bool m_isConst = false;

    };

    SR_COMMON_DLL_API void InvokeMethodVoid(Method::Owner& owner, StringAtom name);
    SR_COMMON_DLL_API void InvokeMethodVoid(Method::Owner& owner, StringAtom name, const Method::Params& params);
    SR_COMMON_DLL_API Value InvokeMethod(Method::Owner& owner, StringAtom name, const Method::Params& params);
    SR_COMMON_DLL_API Value InvokeMethod(Method::Owner& owner, StringAtom name);
}

#endif //SR_ENGINE_UTILS_REFLECTION_METHOD_H
