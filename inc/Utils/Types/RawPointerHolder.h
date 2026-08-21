//
// Created by Monika on 30.11.2025.
//

#ifndef SR_ENGINE_UTILS_RAW_POINTER_HOLDER_H
#define SR_ENGINE_UTILS_RAW_POINTER_HOLDER_H

#include <Utils/Common/AssertFwd.h>
#include <Utils/TypeTraits/TypeTraits.h>

namespace SR_UTILS_NS {
    template<typename T> class RawPointerHolder {
        using DeleterFn = void(*)(T*);
        using CopyFn = T*(*)(const T&);
    public:
        RawPointerHolder() = default;

        template<typename Y> RawPointerHolder(Y* ptr)
            : m_ptr(ptr)
        {
            if (m_ptr) {
                if constexpr (SR_UTILS_NS::IsCompleteTypeV<Y>) {
                    m_deleter = [](T* p) { delete static_cast<Y*>(p); };
                    if constexpr (std::is_copy_constructible_v<Y>) {
                        m_copier = [](const T& obj) { return new Y(static_cast<const Y&>(obj)); };
                    }
                    else {
                        m_copier = nullptr;
                    }
                }
                else {
                    SRHalt("RawPointerHolder() : Type T must be complete to use default deleter and allocator!");
                }
            }
        }

        ~RawPointerHolder() {
            if (m_deleter && m_ptr) {
                m_deleter(m_ptr);
            }
            m_ptr = nullptr;
            m_deleter = nullptr;
            m_copier = nullptr;
        }

        RawPointerHolder(const RawPointerHolder& other) {
            if (other.m_ptr) {
                if (other.m_copier) {
                    m_ptr = other.m_copier(*other.m_ptr);
                    m_deleter = other.m_deleter;
                    m_copier = other.m_copier;
                }
                else {
                    SRHalt("RawPointerHolder() : copier function is not set for non-nullptr pointer!");
                }
            }
        }

        RawPointerHolder& operator=(const RawPointerHolder& other) {
            if (other.m_ptr && !other.m_copier) {
                SRHalt("RawPointerHolder::operator=() : copier function is not set for non-nullptr pointer!");
                return *this;
            }

            if (this != &other) {
                if (m_deleter && m_ptr) {
                    m_deleter(m_ptr);
                }
                m_ptr = (other.m_ptr && other.m_copier) ? m_copier(*other.m_ptr) : nullptr;
                m_deleter = other.m_deleter;
                m_copier = other.m_copier;
            }
            return *this;
        }

        RawPointerHolder(RawPointerHolder&& other) noexcept
            : m_ptr(other.m_ptr)
            , m_deleter(other.m_deleter)
            , m_copier(other.m_copier)
        {
            other.m_ptr = nullptr;
            other.m_deleter = nullptr;
            other.m_copier = nullptr;
        }

        RawPointerHolder& operator=(RawPointerHolder&& other) noexcept {
            if (this != &other) {
                if (m_deleter && m_ptr) {
                    m_deleter(m_ptr);
                }
                m_ptr = other.m_ptr;
                m_deleter = other.m_deleter;
                m_copier = other.m_copier;
                other.m_ptr = nullptr;
                other.m_deleter = nullptr;
                other.m_copier = nullptr;
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

        void Reset() noexcept {
            if (m_deleter && m_ptr) {
                m_deleter(m_ptr);
            }
            m_ptr = nullptr;
            m_deleter = nullptr;
            m_copier = nullptr;
        }

    private:
        T* m_ptr = nullptr;
        DeleterFn m_deleter = nullptr;
        CopyFn m_copier = nullptr;

    };
}

#endif //SR_ENGINE_UTILS_RAW_POINTER_HOLDER_H
