//
// Created by Monika on 30.11.2025.
//

#ifndef SR_ENGINE_UTILS_RAW_POINTER_HOLDER_H
#define SR_ENGINE_UTILS_RAW_POINTER_HOLDER_H

#include <Utils/stdInclude.h>

namespace SR_HTYPES_NS {
    template<typename T> class RawPointerHolder {
    public:
        RawPointerHolder(T* ptr = nullptr)
            : m_ptr(ptr)
        { }

        ~RawPointerHolder() {
            delete m_ptr;
            m_ptr = nullptr;
        }

        RawPointerHolder(const RawPointerHolder& other) {
            if (other.m_ptr) {
                m_ptr = new T(*other.m_ptr);
            }
        }

        RawPointerHolder& operator=(const RawPointerHolder& other) {
            if (this != &other) {
                delete m_ptr;
                m_ptr = other.m_ptr ? new T(*other.m_ptr) : nullptr;
            }
            return *this;
        }

        RawPointerHolder(RawPointerHolder&& other) noexcept
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        RawPointerHolder& operator=(RawPointerHolder&& other) noexcept {
            if (this != &other) {
                delete m_ptr;
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
            }
            return *this;
        }

        T* Get() const noexcept {
            return m_ptr;
        }

        T& operator*() const noexcept {
            return *m_ptr;
        }

        T* operator->() const noexcept {
            return m_ptr;
        }

        bool operator==(const RawPointerHolder& other) const noexcept {
            return m_ptr == other.m_ptr;
        }

        bool operator!=(const RawPointerHolder& other) const noexcept {
            return m_ptr != other.m_ptr;
        }

        operator bool() const noexcept {
            return m_ptr != nullptr;
        }

    private:
        T* m_ptr = nullptr;

    };
}

#endif //SR_ENGINE_UTILS_RAW_POINTER_HOLDER_H
