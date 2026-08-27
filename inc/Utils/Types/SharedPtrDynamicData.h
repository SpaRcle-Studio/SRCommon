//
// Created by Monika on 07.02.2026.
//

#ifndef SR_ENGINE_SHARED_PTR_DYNAMIC_DATA_H
#define SR_ENGINE_SHARED_PTR_DYNAMIC_DATA_H

#include <Utils/Common/AssertFwd.h>
#include <Utils/Common/TypeInfo.h>
#include <Utils/TypeTraits/TypeTraits.h>
/// SRClass must be a complete type here: the class getter installed by InitBasic<T>() is the single
/// source of truth about the SRClass part of the object at runtime, and it must be installed the same
/// way in every translation unit. A forward declaration used to make it depend on the include order.
#include <Utils/TypeTraits/SRClass.h>

//#define SR_SHARED_PTR_TRACE

#ifdef SR_SHARED_PTR_TRACE
    #include <Utils/Platform/Stacktrace.h>
#endif

namespace SR_UTILS_NS {
    enum class SharedPtrPolicy : uint8_t {
        Automatic, Manually
    };
}

namespace SR_HTYPES_NS {
    class SharedPtrDynamicData;

    class SR_COMMON_DLL_API SharedPtrBase {
    public:
        SharedPtrBase();
        explicit SharedPtrBase(SharedPtrDynamicData* data);
        virtual ~SharedPtrBase();

    public:
        const SharedPtrDynamicData* GetPtrData() const; /// NOLINT(modernize-use-nodiscard)
        SharedPtrDynamicData* GetPtrData();
        SR_NODISCARD virtual SRClass* GetSRClass() const = 0;
        SR_NODISCARD virtual bool Valid() const = 0;
        virtual void Reset() = 0;
        virtual void IncrementPointer() = 0;
        virtual void DecrementPointer() = 0;
        virtual void SetPointerFromBase(SharedPtrBase* pBase) = 0;

        SR_NODISCARD uint64_t GetStrongCount() const;

    protected:
        SharedPtrDynamicData* m_data = nullptr;
        bool m_basicManually = false;

    };

    class SharedPtrDynamicDataCounter : public SR_UTILS_NS::NonCopyable {
    public:
        SR_NODISCARD uint64_t GetCount() const;

        static SharedPtrDynamicDataCounter& Instance();

        void Increment(SharedPtrDynamicData* pData);
        void Decrement(SharedPtrDynamicData* pData);

        SR_MAYBE_UNUSED static bool CheckMemoryLeaks();

    public:
        SR_NODISCARD const std::unordered_set<SharedPtrDynamicData*>& GetData() const;

    private:
        std::atomic<uint64_t> m_count = 0;
        std::unordered_set<SharedPtrDynamicData*> m_data;

    };

    struct SharedPtrDynamicData {
        SharedPtrDynamicData(uint64_t strongCount, uint64_t weakCount, bool valid, SR_UTILS_NS::SharedPtrPolicy policy);
        ~SharedPtrDynamicData();

        SR_NODISCARD SR_UTILS_NS::StringAtom GetDebugTrace() const;
        SR_NODISCARD uint64_t GetStrongCount() const;

        void IncrementStrong();
        void DecrementStrong();
        void IncrementWeak();
        void DecrementWeak();

        std::atomic<uint64_t> strongCount = 0;
        std::atomic<uint64_t> weakCount = 0;
        bool valid = false;
        bool deallocated = false;
        SR_UTILS_NS::SharedPtrPolicy policy = SR_UTILS_NS::SharedPtrPolicy::Automatic;
        void (*deleter)(void*) = nullptr;
        SRClass* (*classGetter)(void*) = nullptr;
        void* (*fromBaseGetter)(SharedPtrBase*) = nullptr;

        template<typename T> void InitBasic() {
            deleter = [](void* p) {
                delete static_cast<T*>(p);
            };
            fromBaseGetter = [](SharedPtrBase* p) -> void* {
                if constexpr (std::is_base_of_v<SharedPtrBase, T>) {
                    return static_cast<void*>(static_cast<T*>(p));
                }
                else {
                    return nullptr;
                }
            };
            if constexpr (std::is_base_of_v<SRClass, T>) {
                classGetter = [](void* p) -> SRClass* {
                    T* ptr = static_cast<T*>(p);
                    return static_cast<SRClass*>(ptr);
                };
            }
            else if constexpr (std::is_polymorphic_v<T>) {
                classGetter = [](void* p) -> SRClass* {
                    T* ptr = static_cast<T*>(p);
                    return dynamic_cast<SRClass*>(ptr);
                };
            }
        }

    #ifdef SR_SHARED_PTR_TRACE
        SR_UTILS_NS::StringAtom debugTrace;
    #endif

    };
}

#endif //SR_ENGINE_SHARED_PTR_DYNAMIC_DATA_H
