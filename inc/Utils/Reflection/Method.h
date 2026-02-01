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
    public:
        using Owner = SRClass;
        using Params = std::span<Value*>;
        using FunctorNoReturnNoParams = SR_HTYPES_NS::Function<void(Owner&)>;
        using FunctorNoReturnWithParams = SR_HTYPES_NS::Function<void(Owner&, const Params&)>;
        using FunctorWithReturnNoParams = SR_HTYPES_NS::Function<Value(Owner&)>;
        using FunctorWithReturnWithParams = SR_HTYPES_NS::Function<Value(Owner&, const Params&)>;
    public:
        SR_NODISCARD StringAtom GetName() const;

        void InvokeVoid(Owner& owner) const;
        void InvokeVoid(Owner& owner, const Params& params) const;
        Value Invoke(Owner& owner, const Params& params) const;
        Value Invoke(Owner& owner) const;

        Method& SetNoReturnNoParams(const FunctorNoReturnNoParams& func);
        Method& SetNoReturnWithParams(const FunctorNoReturnWithParams& func);
        Method& SetWithReturnNoParams(const FunctorWithReturnNoParams& func);
        Method& SetWithReturnWithParams(const FunctorWithReturnWithParams& func);
        Method& SetName(StringAtom name);
        Method& SetParamsCount(uint32_t count);
        Method& SetHasReturn(bool hasReturn);

    private:
        FunctorNoReturnNoParams m_noReturnNoParams;
        FunctorNoReturnWithParams m_noReturnWithParams;
        FunctorWithReturnNoParams m_withReturnNoParams;
        FunctorWithReturnWithParams m_withReturnWithParams;

        StringAtom m_name;
        uint32_t m_paramsCount = 0;
        bool m_hasReturn = false;

    };

    void InvokeMethodVoid(Method::Owner& owner, StringAtom name);
    void InvokeMethodVoid(Method::Owner& owner, StringAtom name, const Method::Params& params);
    Value InvokeMethod(Method::Owner& owner, StringAtom name, const Method::Params& params);
    Value InvokeMethod(Method::Owner& owner, StringAtom name);
}

#endif //SR_ENGINE_UTILS_REFLECTION_METHOD_H
