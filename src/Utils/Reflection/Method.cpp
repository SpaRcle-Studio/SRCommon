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
        InvokeVoid(owner, {});
    }

    void Method::InvokeVoid(Owner& owner, const Params& params) const {
        SR_TRACY_ZONE;
        if (!m_pReturnTypeInfo && GetParamsCount() == params.size()) {
            static_cast<void>(Invoke(owner, params));
        }
        else {
            SR_ERROR("Method::InvokeVoid() : invalid method invoke! Name: {}", m_name);
        }
    }

    Value Method::Invoke(Owner& owner, const Params& params) const {
        SR_TRACY_ZONE;
        if (!m_pThunk || GetParamsCount() != params.size()) {
            SR_ERROR("Method::Invoke() : invalid method invoke! Name: {}", m_name);
            return {};
        }

        void** pArguments = params.empty() ? nullptr : static_cast<void**>(SR_STACK_ALLOC(params.size_bytes()));
        for (uint32_t i = 0; i < params.size(); ++i) {
            pArguments[i] = GetArgumentData(*params[i], i);
        }
        Value result;
        if (HasReturn() && !ReturnsReference()) {
            result = Value::CreateDefault(m_pReturnTypeInfo);
        }
        InvokeInto(owner, pArguments, result);
        if (m_dontPack && result && result.GetTypeInfo().kind == TypeKind::Value) {
            if (result.IsRef()) {
                return *result.Cast<Value>();
            }
            return std::move(*result.Cast<Value>());
        }
        return result;
    }

    Value Method::Invoke(Owner& owner) const {
        return Invoke(owner, {});
    }


    void Method::InvokeRaw(Owner& owner, void* const* pArgs, void* pReturn) const {
        SR_TRACY_ZONE;
        m_pThunk(owner, pArgs, pReturn);
    }

    void* Method::GetArgumentData(Value& value, const uint32_t index) const {
        if (m_params[index].pTypeInfo->kind == TypeKind::Value &&
            m_params[index].isReference && m_params[index].isConst) {
            return &value;
        }
        return value.Data();
    }

    void Method::InvokeInto(Owner& owner, void* const* pArgs, Value& result) const {
        if (!HasReturn()) {
            InvokeRaw(owner, pArgs, nullptr);
            return;
        }
        if (m_returnsReference) {
            const void* pReference = nullptr;
            InvokeRaw(owner, pArgs, &pReference);
            Value reference;
            reference.m_typeInfo = m_pReturnTypeInfo;
            reference.m_storage = ReflectedValue::MakeFromPointer(const_cast<void*>(pReference),
                m_returnIsConst ? ReflectedValueStorageType::ConstReference : ReflectedValueStorageType::Reference);
            result = std::move(reference);
            return;
        }
        if (!result || &result.GetTypeInfo() != m_pReturnTypeInfo || result.IsRef()) {
            SRHalt("Method::InvokeInto() : return storage must be prepared before invocation! Method: {}", m_name);
            return;
        }
        if (result) {
            InvokeRaw(owner, pArgs, result.Data());
        }
        else {
            SR_ERROR("Method::InvokeInto() : cannot construct return type! Method: {}", m_name);
        }
    }

    Method& Method::SetThunk(Thunk pThunk) {
        m_pThunk = pThunk;
        return *this;
    }

    Method& Method::SetReturnReference(const bool isReference, const bool isConst) {
        m_returnsReference = isReference;
        m_returnIsConst = isConst;
        return *this;
    }

    Method& Method::SetDontPack() {
        m_dontPack = true;
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

    Method& Method::AddParam(StringAtom name, const TypeInfo* pTypeInfo, const bool isReference, const bool isConst) {
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

    Method& Method::SetReturnType(const TypeInfo* pReturnTypeInfo) {
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
