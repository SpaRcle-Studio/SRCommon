//
// Created by Monika on 17.11.2021.
//

#ifndef SR_ENGINE_THREAD_H
#define SR_ENGINE_THREAD_H

#include <Utils/Common/ThreadUtils.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Types/Function.h>
#include <Utils/Types/LockGuard.h>
#include <Utils/Types/String.h>

#define SR_THREAD_SAFE_CHECKS 1

/** Warning: этот метод очень медленный! */
#define SR_THIS_THREAD (SR_HTYPES_NS::Thread::Factory::Instance().GetThisThread())

namespace SR_HTYPES_NS {
    class DataStorage;
    class Thread;

    struct ThreadImpl {
    #ifdef SR_THREADS_ALLOWED
        std::thread thread;
    #endif
        SR_HTYPES_NS::Function<bool()> threadBody;
        std::atomic<bool> nameChanged = false;
        std::atomic<bool> isCreated = false;
        std::atomic<bool> isRan = false;

        mutable std::shared_mutex mutex;
        mutable std::atomic<const SR_HTYPES_NS::Function<bool()>*> function = nullptr;
        mutable std::atomic<bool> executeResult = false;
    };

    class SR_COMMON_DLL_API Thread : public NonCopyable {
    public:
        using Ptr = Thread*;
        using ThreadId = SR_UTILS_NS::StringAtom;
        using ThreadsMap = std::map<ThreadId, Thread::Ptr>;

        SR_NODISCARD static ThreadId EmptyThreadId();
        SR_NODISCARD ThreadImpl& GetImpl() const;

        class Factory : public Singleton<Factory> {
            SR_REGISTER_SINGLETON(Factory)
            friend class Thread;
        protected:
            Factory();
            ~Factory() override;

        public:
            void SetMainThread();
            void PrintThreads();
            void DeInitialize();

            SR_NODISCARD Ptr GetMainThread();
            SR_NODISCARD Ptr GetThisThread();
            SR_NODISCARD Ptr TryGetThisThread();
            SR_NODISCARD uint32_t GetThreadsCount();

            template<class Functor, typename... Args> bool Create(Ptr& pThread, Functor&& fn, Args&&... args);

            bool IsSingletonCanBeDestroyed() const override { return false; }

        private:
            void Remove(Thread* pThread);

        private:
            ThreadsMap m_threads = ThreadsMap();
            Thread* m_main = nullptr;

        };

    private:
        Thread();

    #ifdef SR_THREADS_ALLOWED
        explicit Thread(std::thread&& thread);
    #endif
        explicit Thread(ThreadId id);

        ~Thread() override;

    public:
        SR_NODISCARD bool Joinable() const;
        SR_NODISCARD ThreadId GetId() const;
        SR_NODISCARD DataStorage* GetContext();

        void SetName(const std::string& name);

        void Synchronize();

        template<class Functor, typename... Args> SR_NODISCARD bool Run(Functor&& fn);

        bool Execute(const SR_HTYPES_NS::Function<bool()>& function) const;

        void Join();
        bool TryJoin();
        void Free();
        bool HasId() const;
        void Detach();
        void SetPriority(ThreadPriority priority);

        static void Sleep(uint64_t milliseconds);

    private:
        ThreadId m_id;
        String m_name;
        DataStorage* m_context = nullptr;
        mutable ThreadImpl* m_impl = nullptr;

    };

    template<class Functor, typename... Args>
    bool Thread::Factory::Create(Thread::Ptr& pThread, Functor&& fn, Args&&... args)  {
    #ifdef SR_THREADS_ALLOWED
        SR_LOG("Thread::Factory::Create() : creating new thread...");
    #else
        SR_LOG("Thread::Factory::Create() : creating new fake thread...");
    #endif

        SR_LOCK_GUARD;

        pThread = new Thread();

    #ifdef SR_THREADS_ALLOWED
        std::thread thread([fn = std::forward<Functor>(fn), pThread, argsTuple = std::make_tuple(args...)]() mutable {
            while (!pThread->GetImpl().isCreated || !pThread->HasId()) {
                pThread->m_id = SR_UTILS_NS::GetThreadId(pThread->GetImpl().thread);
            }

            pThread->GetImpl().threadBody = [fn = std::forward<Functor>(fn), argsTuple]() mutable {
                return std::apply(fn, std::forward<decltype(argsTuple)>(argsTuple));
            };

            while (pThread->GetImpl().threadBody()) {
                SR_NOOP;
            }
        });

        while (!pThread->HasId()) {
            pThread->m_id = SR_UTILS_NS::GetThreadId(thread);
        }
    #else
        pThread->GetImpl().threadBody = [fn = std::forward<Functor>(fn), argsTuple = std::make_tuple(args...)]() mutable -> bool {
            return std::apply(fn, std::forward<decltype(argsTuple)>(argsTuple));
        };
    #endif

        m_threads.insert(std::make_pair(pThread->GetId(), pThread));

    #ifdef SR_THREADS_ALLOWED
        pThread->GetImpl().thread = std::move(thread);
    #endif
        pThread->GetImpl().isRan = true;
        pThread->GetImpl().isCreated = true;

        SR_LOG("Thread::Factory::Create() : thread \"{}\" created.", pThread->m_id.c_str());

        return true;
    }
}

#endif //SR_ENGINE_THREAD_H
