//
// Created by Monika on 20.01.2025.
//

#include <Utils/Reflection/Value.h>

namespace SR_UTILS_NS::Reflection {
    Value::~Value() {
        Destroy();
    }

    Value::Value(const Value& other)
        : m_deleter(other.m_deleter)
        , m_copier(other.m_copier)
        , m_type(other.m_type)
        , m_isReference(false)
        , m_isConst(other.m_isConst)
    {
        if (other.m_data && m_copier) {
            m_copier(m_data, other.m_data);
        }
    }

    Value::Value(Value&& other) noexcept {
        m_type = SR_EXCHANGE(other.m_type, {});
        m_isReference = SR_EXCHANGE(other.m_isReference, {});
        m_isConst = SR_EXCHANGE(other.m_isConst, {});
        m_deleter = SR_EXCHANGE(other.m_deleter, {});
        m_copier = SR_EXCHANGE(other.m_copier, {});
        m_data = SR_EXCHANGE(other.m_data, {});
    }

    Value& Value::operator=(const Value& other) {
        if (this != &other) {
            m_type = other.m_type;
            m_isReference = false;
            m_isConst = other.m_isConst;
            m_deleter = other.m_deleter;
            m_copier = other.m_copier;

            if (other.m_data && m_copier) {
                m_copier(m_data, other.m_data);
            }
        }
        return *this;
    }

    Value& Value::operator=(Value&& other) noexcept {
        if (this != &other) {
            m_type = SR_EXCHANGE(other.m_type, {});
            m_isReference = SR_EXCHANGE(other.m_isReference, {});
            m_isConst = SR_EXCHANGE(other.m_isConst, {});
            m_deleter = SR_EXCHANGE(other.m_deleter, {});
            m_copier = SR_EXCHANGE(other.m_copier, {});
            m_data = SR_EXCHANGE(other.m_data, {});
        }
        return *this;
    }

    Value Value::Clone() const {
        Value result;
        result.m_type = m_type;
        result.m_isReference = false;
        result.m_isConst = false;
        result.m_deleter = m_deleter;
        result.m_copier = m_copier;

        if (m_data && m_copier) {
            m_copier(result.m_data, m_data);
        }

        return result;
    }

    void Value::Destroy() {
        if (!m_data || m_isReference || !m_deleter) {
            return;
        }

        m_deleter(m_data);
        m_data = nullptr;
    }
}
