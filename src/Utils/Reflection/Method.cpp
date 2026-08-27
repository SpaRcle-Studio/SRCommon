//
// Created by Monika on 31.01.2026.
//

#include <Utils/Reflection/Method.h>
#include <Utils/TypeTraits/SRClassMeta.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS::Reflection {
    Method::Method(const Method& other)
        : m_name(other.m_name)
        , m_displayName(other.m_displayName)
        , m_isEditorButton(other.m_isEditorButton)
        , m_isEvaluate(other.m_isEvaluate)
        , m_methodActiveCallback(other.m_methodActiveCallback)
        , m_noReturnNoParams(other.m_noReturnNoParams)
    {
        m_params.reserve(other.m_params.size());
        for (auto&& param : other.m_params) {
            m_params.emplace_back(Parameter{ param.name, CopyTypeInfo(param.pTypeInfo), param.isReference, param.isConst });
        }
        m_pReturnTypeInfo = CopyTypeInfo(other.m_pReturnTypeInfo);
    }

    Method& Method::operator=(const Method& other) {
        if (this != &other) {
            m_name = other.m_name;
            m_displayName = other.m_displayName;
            m_pReturnTypeInfo = CopyTypeInfo(other.m_pReturnTypeInfo);
            m_isEditorButton = other.m_isEditorButton;
            m_isEvaluate = other.m_isEvaluate;
            m_methodActiveCallback = other.m_methodActiveCallback;
            m_noReturnNoParams = other.m_noReturnNoParams;

            for (auto&& param : m_params) {
                FreeTypeInfo(param.pTypeInfo);
            }
            m_params.clear();

            m_params.reserve(other.m_params.size());
            for (auto&& param : other.m_params) {
                m_params.emplace_back(Parameter{ param.name, CopyTypeInfo(param.pTypeInfo), param.isReference, param.isConst });
            }
        }
        return *this;
    }

    Method::~Method() {
        for (auto&& param : m_params) {
            FreeTypeInfo(param.pTypeInfo);
        }
    }

    StringAtom Method::GetName() const {
        return m_name;
    }

    uint32_t Method::GetParamsCount() const {
        return m_params.size();
    }

    bool Method::HasReturn() const {
        return m_pReturnTypeInfo;
    }

    bool Method::IsEditorButton() const {
        return m_isEditorButton;
    }

    StringAtom Method::GetDisplayName() const {
        if (m_displayName.empty()) {
            SR_TRACY_ZONE;
            m_displayName = Reflection::MakeDisplayName(m_name);
        }
        return m_displayName;
    }

    void Method::InvokeVoid(Owner& owner) const {
        if (!m_pReturnTypeInfo && GetParamsCount() == 0) {
            m_noReturnNoParams(owner);
        }
        else {
            SR_ERROR("Method::InvokeVoid() : invalid method invoke! Name: {}", m_name);
        }
    }

    void Method::InvokeVoid(Owner& owner, const Params& params) const {
        if (!m_pReturnTypeInfo && GetParamsCount() > 0) {
            m_noReturnWithParams(owner, params);
        }
        else {
            SR_ERROR("Method::InvokeVoid() : invalid method invoke! Name: {}", m_name);
        }
    }

    Value Method::Invoke(Owner& owner, const Params& params) const {
        if (m_pReturnTypeInfo && GetParamsCount() > 0) {
            return m_withReturnWithParams(owner, params);
        }
        else {
            SR_ERROR("Method::Invoke() : invalid method invoke! Name: {}", m_name);
            return {};
        }
    }

    Value Method::Invoke(Owner& owner) const {
        if (m_pReturnTypeInfo && GetParamsCount() == 0) {
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

    Method& Method::SetEditorButton() {
        m_isEditorButton = true;
        return *this;
    }

    Method& Method::SetCondition(MethodActiveCallbackFn condition) {
        m_methodActiveCallback = condition;
        return *this;
    }

    bool Method::IsActive(Method::Owner& owner) const {
        if (m_methodActiveCallback) {
            return m_methodActiveCallback(&owner);
        }
        return true;
    }

    Method& Method::AddParam(StringAtom name, TypeInfo* pTypeInfo, const bool isReference, const bool isConst) {
        m_params.emplace_back(Parameter{ name, pTypeInfo, isReference, isConst });
        return *this;
    }

    const Method::Parameter& Method::GetParam(uint32_t index) const {
        if (index >= m_params.size()) {
            SRHalt("Method::GetParam() : index {} is out of range! Method: {}", index, m_name);
            static Parameter emptyParam;
            return emptyParam;
        }
        return m_params[index];
    }

    Method& Method::SetEvaluate() {
        m_isEvaluate = true;
        return *this;
    }

    bool Method::IsEvaluate() const {
        return m_isEvaluate;
    }

    bool Method::IsOutputParam(const uint32_t index) const {
        return index < m_params.size() && m_params[index].IsOutput();
    }

    Method& Method::SetReturnType(TypeInfo* pReturnTypeInfo) {
        m_pReturnTypeInfo = pReturnTypeInfo;
        return *this;
    }

    const TypeInfo* Method::GetReturnType() const {
        return m_pReturnTypeInfo;
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

    void InvokeMethodVoid(Method::Owner& owner, StringAtom name, const Method::Params& params) {
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