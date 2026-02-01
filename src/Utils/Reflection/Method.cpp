//
// Created by Monika on 31.01.2026.
//

#include <Utils/Reflection/Method.h>
#include <Utils/TypeTraits/SRClassMeta.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS::Reflection {
    StringAtom Method::GetName() const {
        return m_name;
    }

    void Method::InvokeVoid(Owner& owner) const {
        if (m_noReturnNoParams) {
            m_noReturnNoParams(owner);
        }
        else {
            SR_ERROR("Method::InvokeVoid() : invalid method invoke! Name: {}", m_name);
        }
    }

    void Method::InvokeVoid(Owner& owner, const Params& params) const {
        if (m_noReturnWithParams) {
            m_noReturnWithParams(owner, params);
        }
        else {
            SR_ERROR("Method::InvokeVoid() : invalid method invoke! Name: {}", m_name);
        }
    }

    Value Method::Invoke(Owner& owner, const Params& params) const {
        if (m_withReturnWithParams) {
            return m_withReturnWithParams(owner, params);
        }
        else {
            SR_ERROR("Method::Invoke() : invalid method invoke! Name: {}", m_name);
            return {};
        }
    }

    Value Method::Invoke(Owner& owner) const {
        if (m_withReturnNoParams) {
            return m_withReturnNoParams(owner);
        }
        else {
            SR_ERROR("Method::Invoke() : invalid method invoke! Name: {}", m_name);
            return {};
        }
    }


    Method& Method::SetNoReturnNoParams(const FunctorNoReturnNoParams& func) {
        m_noReturnNoParams = func;
        return *this;
    }

    Method& Method::SetNoReturnWithParams(const FunctorNoReturnWithParams& func) {
        m_noReturnWithParams = func;
        return *this;
    }

    Method& Method::SetWithReturnNoParams(const FunctorWithReturnNoParams& func) {
        m_withReturnNoParams = func;
        return *this;
    }

    Method& Method::SetWithReturnWithParams(const FunctorWithReturnWithParams& func) {
        m_withReturnWithParams = func;
        return *this;
    }

    Method& Method::SetName(SR_UTILS_NS::StringAtom name) {
        m_name = name;
        return *this;
    }

    Method& Method::SetParamsCount(uint32_t count) {
        m_paramsCount = count;
        return *this;
    }

    Method& Method::SetHasReturn(bool hasReturn) {
        m_hasReturn = hasReturn;
        return *this;
    }

    void InvokeMethodVoid(Method::Owner& owner, StringAtom name) {
        SR_TRACY_ZONE;
        if (auto&& pMethod = owner.GetMeta()->FindMethod(name)) {
            pMethod->InvokeVoid(owner);
        }
        else {
            SR_ERROR("InvokeMethodVoid() : method {} not found!", name);
        }
    }

    void InvokeMethodVoid(Method::Owner& owner,StringAtom name, const Method::Params& params) {
        SR_TRACY_ZONE;
        if (auto&& pMethod = owner.GetMeta()->FindMethod(name)) {
            pMethod->InvokeVoid(owner, params);
        }
        else {
            SR_ERROR("InvokeMethodVoid() : method {} not found!", name);
        }
    }

    Value InvokeMethod(Method::Owner& owner, StringAtom name, const Method::Params& params) {
        SR_TRACY_ZONE;
        if (auto&& pMethod = owner.GetMeta()->FindMethod(name)) {
            return pMethod->Invoke(owner, params);
        }
        else {
            SR_ERROR("InvokeMethod() : method {} not found!", name);
            return {};
        }
    }

    Value InvokeMethod(Method::Owner& owner, StringAtom name) {
        SR_TRACY_ZONE;
        if (auto&& pMethod = owner.GetMeta()->FindMethod(name)) {
            return pMethod->Invoke(owner);
        }
        else {
            SR_ERROR("Invoke() : method {} not found!", name);
            return {};
        }
    }

}