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

        mutable std::recursive_mutex mutex;
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

        void SetName(StringView name);

        void Synchronize();

        bool Execute(const SR_HTYPES_NS::Function<bool()>& function) const;

        void Join();
        bool TryJoin();
        void Free();
        bool HasId() const;
        void Detach();
        void SetPriority(ThreadPriority priority);

        static void Sleep(uint64_t milliseconds);

    private:
        void SetId(ThreadId id);

    private:
        ThreadId m_id;
        String m_name;
        DataStorage* m_context = nullptr;
        mutable ThreadImpl* m_impl = nullptr;

    };

    template<class Functor, typename... Args>
    bool Thread::Factory::Create(Thread::Ptr& pThread, Functor&& fn, Args&&... args)  {
        SR_TRACY_ZONE;

    #ifdef SR_THREADS_ALLOWED
        SR_LOG("Thread::Factory::Create() : creating new thread...");
    #else
        SR_LOG("Thread::Factory::Create() : creating new fake thread...");
    #endif

        SR_LOCK_GUARD;

        pThread = new Thread();

    #ifdef SR_THREADS_ALLOWED
        pThread->GetImpl().thread = std::thread([fn = std::forward<Functor>(fn), pThread, argsTuple = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            SR_TRACY_ZONE_N("Thread");

            while (!pThread->GetImpl().isCreated.load(std::memory_order_acquire)) {
                SR_NOOP;
            }

            pThread->SetId(SR_UTILS_NS::GetThreadId(pThread->GetImpl().thread));
            pThread->GetImpl().threadBody = [fn = std::forward<Functor>(fn), argsTuple]() mutable {
                return std::apply(fn, std::forward<decltype(argsTuple)>(argsTuple));
            };

            SR_LOG("Thread::Factory::Create() : thread \"{}\" created.", pThread->GetId());
            SR_TRACY_ZONE_TEXT(pThread->GetId());

            while (!pThread->GetImpl().isRan.load(std::memory_order_acquire)) {
                SR_NOOP;
            }

            while (pThread->GetImpl().threadBody()) {
                SR_NOOP;
            }
        });

        pThread->GetImpl().isCreated.store(true, std::memory_order_release);

        while (!pThread->HasId()) {
            SR_NOOP;
        }
    #else
        static uint32_t threadCounter = 0;
        pThread->SetId("FakeThread_{}"_format(threadCounter++));
        pThread->GetImpl().isCreated = true;
        pThread->GetImpl().threadBody = [fn = std::forward<Functor>(fn), argsTuple = std::make_tuple(std::forward<Args>(args)...)]() mutable -> bool {
            return std::apply(fn, std::forward<decltype(argsTuple)>(argsTuple));
        };
    #endif

        if (m_threads.find(pThread->GetId()) != m_threads.end()) {
            SRHaltTerminate("Thread::Factory::Create() : thread with id \"{}\" already exists!", pThread->GetId());
        }

        m_threads[pThread->GetId()] = pThread;
        pThread->GetImpl().isRan.store(true, std::memory_order_release);

        return true;
    }
}

#endif //SR_ENGINE_THREAD_H
