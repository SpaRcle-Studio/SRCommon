//
// Created by Monika on 28.09.2021.
//

#ifndef SR_ENGINE_SINGLETON_H
#define SR_ENGINE_SINGLETON_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/Common/Breakpoint.h>

#include <Utils/Platform/Stacktrace.h>

#include <Utils/Types/SafePtrLockGuard.h>
#include <Utils/Types/Map.h>

#define SR_REGISTER_SINGLETON(className)                                                                                \
    private:                                                                                                            \
        friend class SR_UTILS_NS::Singleton<className>;                                                                 \
        static uint64_t GetStaticSingletonHashName() {                                                                  \
            static uint64_t staticName = SR_UTILS_NS::SingletonManager::GetOrAddSingletonHashName(#className);          \
            return staticName;                                                                                          \
        }                                                                                                               \
        uint64_t GetSingletonHashName() const noexcept final { return GetStaticSingletonHashName(); };                  \

#define SR_REGISTER_TEMPLATE_SINGLETON(className, T)                                                                    \
    private:                                                                                                            \
        friend class SR_UTILS_NS::Singleton<className>;                                                                 \
        /** TODO: replace typeid(T).name() with something more stable **/                                               \
        SR_NODISCARD static SR_UTILS_NS::StringAtom GetStaticTemplatedSingletonName() noexcept {                        \
            return SR_FORMAT("{}<{}>", #className, typeid(T).name());                                                   \
        }                                                                                                               \
        static SR_UTILS_NS::StringAtom GetStaticSingletonName() { return GetStaticTemplatedSingletonName(); }           \
        SR_UTILS_NS::StringAtom GetSingletonName() const noexcept final { return GetStaticSingletonName(); };           \

namespace SR_UTILS_NS {
    class SingletonManager;
    template<typename T> class Singleton;

    class SR_COMMON_DLL_API SingletonBase : public NonCopyable  {
        friend class SingletonManager;
    public:
        ~SingletonBase() override;

    protected:
        SingletonBase();

    protected:
        virtual uint64_t GetSingletonHashName() const noexcept = 0;
        virtual void OnSingletonDestroy();
        virtual void InitSingleton();
        virtual bool IsSingletonCanBeDestroyed() const;

    protected:
        mutable std::recursive_mutex m_mutex;

    };

    class SR_COMMON_DLL_API SingletonManager : public NonCopyable {
    public:
        void* GetSingleton(uint64_t hashName) noexcept;
        std::recursive_mutex& GetCreationMutex(uint64_t hashName);
        void DestroyAll();
        void Remove(uint64_t hashName);

        template<typename T> void Register(Singleton<T>* pSingleton);

        SR_NODISCARD static uint64_t GetOrAddSingletonHashName(const char* name);

    private:
        void RegisterInternal(uint64_t hashName, void* pSingleton, SingletonBase* pSingletonBase);

    private:
        struct SingletonInfo {
            uint64_t hashName = 0;
            void* pSingleton = nullptr;
            SingletonBase* pSingletonBase = nullptr;
        };
        ska::flat_hash_map<uint64_t, SingletonInfo> m_singletons;
        mutable std::recursive_mutex m_mutex;
        std::map<uint64_t, std::recursive_mutex> m_creationMutexes;

    };

    SR_COMMON_DLL_API SingletonManager* GetSingletonManager() noexcept;

    template<typename T> class Singleton : public SingletonBase {
    protected:
        Singleton();
        ~Singleton() override = default;

    public:
        SR_MAYBE_UNUSED static T& Instance() noexcept;

        SR_MAYBE_UNUSED SR_NODISCARD static SR_HTYPES_NS::SingletonRecursiveLockGuard<Singleton<T>*> ScopeLockSingleton();
        SR_MAYBE_UNUSED static bool IsSingletonInitialized() noexcept;
        SR_MAYBE_UNUSED static void DestroySingleton();
        SR_MAYBE_UNUSED static void LockSingleton() noexcept;
        SR_MAYBE_UNUSED static void UnlockSingleton() noexcept;
        SR_MAYBE_UNUSED static std::recursive_mutex& GetMutex() noexcept;

    private:
        static Singleton<T>* GetSingleton() noexcept;

    };

    /// =============================================== Implementation =================================================

    template<typename T> Singleton<T> *Singleton<T>::GetSingleton() noexcept {
        void* p = GetSingletonManager()->GetSingleton(T::GetStaticSingletonHashName());
        return reinterpret_cast<Singleton<T>*>(p);
    }

    template<typename T> std::recursive_mutex &Singleton<T>::GetMutex() noexcept {
        return Instance().m_mutex;
    }

    template<typename T> void Singleton<T>::UnlockSingleton() noexcept {
        if (auto&& pSingleton = GetSingleton()) {
            pSingleton->m_mutex.unlock();
        }
        else {
            std::cerr << "Singleton isn't initialized!\n";
            std::cerr << GetStacktrace() << std::endl;
            SR_MAKE_BREAKPOINT;
        }
    }

    template<typename T> void Singleton<T>::LockSingleton() noexcept {
        GetMutex().lock();
    }

    template<typename T> T &Singleton<T>::Instance() noexcept {
        auto&& pSingleton = GetSingleton();

        if (!pSingleton) {
            std::lock_guard lock(GetSingletonManager()->GetCreationMutex(T::GetStaticSingletonHashName()));

            pSingleton = GetSingleton();

            if (!pSingleton) {
                pSingleton = new T();
                GetSingletonManager()->Register<T>(pSingleton);
                pSingleton->InitSingleton();
                return *static_cast<T*>(pSingleton);
            }
        }

        return *static_cast<T*>(pSingleton);
    }

    template<typename T> void Singleton<T>::DestroySingleton() {
        if (auto&& pSingleton = GetSingleton()) {
            if (!pSingleton->IsSingletonCanBeDestroyed()) {
                return;
            }

            pSingleton->OnSingletonDestroy();
            GetSingletonManager()->Remove(T::GetStaticSingletonHashName());
            delete pSingleton;
        }
    }

    template<typename T> bool Singleton<T>::IsSingletonInitialized() noexcept {
        return GetSingleton();
    }

    template<typename T>
    Types::SingletonRecursiveLockGuard<Singleton<T> *> Singleton<T>::ScopeLockSingleton() {
        return SR_HTYPES_NS::SingletonRecursiveLockGuard<Singleton<T>*>(&Instance());
    }

    template<typename T> Singleton<T>::Singleton()
        : SingletonBase()
    {
        if (GetSingleton()) {
            std::cerr << "Singleton already exists!\n";
            std::cerr << GetStacktrace() << std::endl;
            SR_MAKE_BREAKPOINT;
        }
    }

    template<typename T> void SingletonManager::Register(Singleton<T> *pSingleton) {
        std::lock_guard lock(m_mutex);
        RegisterInternal(pSingleton->GetSingletonHashName(), (void*)pSingleton, dynamic_cast<SingletonBase*>(pSingleton));
    }
}

#endif //SR_ENGINE_SINGLETON_H
