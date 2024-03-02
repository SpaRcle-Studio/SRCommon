//
// Created by Monika on 17.11.2021.
//

#ifndef GAMEENGINE_THREAD_H
#define GAMEENGINE_THREAD_H

#include <Utils/Debug.h>
#include <Utils/Common/ThreadUtils.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Types/Function.h>

#define SR_THREAD_SAFE_CHECKS 1

/** Warning: этот метод очень медленный! */
#define SR_THIS_THREAD (SR_HTYPES_NS::Thread::Factory::Instance().GetThisThread())

#define SR_LOCK_GUARD std::lock_guard<std::recursive_mutex> codegen_lock(m_mutex)
#define SR_LOCK_GUARD_INHERIT(baseClass) std::lock_guard<std::recursive_mutex> codegen_lock(baseClass::m_mutex)
#define SR_SCOPED_LOCK std::lock_guard<std::recursive_mutex> codegen_lock(m_mutex)
#define SR_WRITE_LOCK std::lock_guard<std::shared_mutex> SR_MACRO_CONCAT(codegen_write_lock, SR_LINE)(m_mutex)
#define SR_READ_LOCK std::shared_lock<std::shared_mutex> SR_MACRO_CONCAT(codegen_read_lock, SR_LINE)(m_mutex)

namespace SR_HTYPES_NS {
    class DataStorage;
    class Thread;

    class SR_DLL_EXPORT Thread : public NonCopyable {
        friend class Factory;
    public:
        using Ptr = Thread*;
        using ThreadId = std::string;
        using ThreadsMap = std::unordered_map<ThreadId, Thread::Ptr>;

        class SR_DLL_EXPORT Factory : public Singleton<Factory> {
            SR_REGISTER_SINGLETON(Factory)
            friend class Thread;
        protected:
            Factory()
                : Singleton<Factory>()
            { }
            ~Factory() override = default;

        public:
            void SetMainThread();
            void PrintThreads();

            SR_NODISCARD Ptr GetMainThread();
            SR_NODISCARD Ptr GetThisThread();
            SR_NODISCARD Ptr TryGetThisThread();
            SR_NODISCARD Ptr Create(std::thread thread);
            SR_NODISCARD Ptr Create(const std::function<void()>& fn);
            SR_NODISCARD uint32_t GetThreadsCount();

            SR_NODISCARD Ptr CreateEmpty();

            template<class Functor, typename... Args> SR_NODISCARD Ptr Create(Functor&& fn, Args&&... args) {
                std::thread thread(fn, std::forward<Args>(args)...);
                return Create(std::move(thread));
            }

        private:
            void Remove(Thread* pThread);

        private:
            ThreadsMap m_threads = ThreadsMap();
            Thread* m_main = nullptr;

        };

    private:
        Thread();

        explicit Thread(std::thread&& thread);
        explicit Thread(ThreadId id);

        ~Thread() override;

    public:
        SR_NODISCARD bool Joinable() const { return m_thread.joinable(); }
        SR_NODISCARD ThreadId GetId();
        SR_NODISCARD DataStorage* GetContext() { return m_context; }

        void SetName(const std::string& name);

        void Synchronize();

        template<class Functor, typename... Args> SR_NODISCARD bool Run(Functor&& fn) {
            if (Joinable()) {
                SRHalt("Thread::Run() : thread is already running!");
                return false;
            }

            Factory::LockSingleton();

            auto&& thread = std::thread([function = std::forward<Functor>(fn), this]() {
                while (!m_isCreated || m_id == "0" || m_id.empty()) {
                    m_id = SR_UTILS_NS::GetThreadId(m_thread);
                }
                Factory::Instance().m_threads.insert(std::make_pair(m_id, this));
                SR_LOG("Thread::Run() : run thread \"{}\"",  m_id);
                while (!m_isRan) {
                    SR_NOOP;
                }
                function();
            });

            m_thread = std::move(thread);
            SR_LOG("Thread::Run() : thread is moved");
            m_isCreated = true;

            Factory::UnlockSingleton();

            m_isRan = true;

            return true;
        }

        bool Execute(const SR_HTYPES_NS::Function<bool()>& function) const;

        void Join() {
            SR_LOG("Thead::Join() : join thread \"{}\" with id \"{}\"...", m_name, m_id);
            m_thread.join();
        }

        bool TryJoin();

        void Free();

        void Detach() { m_thread.detach(); }

        void SetPriority(ThreadPriority priority);

        static void Sleep(uint64_t milliseconds);

    private:
        std::thread m_thread;
        ThreadId m_id;
        std::string m_name;
        DataStorage* m_context = nullptr;

        std::atomic<bool> m_isCreated = false;
        std::atomic<bool> m_isRan = false;

        mutable std::shared_mutex m_mutex;
        mutable std::atomic<const SR_HTYPES_NS::Function<bool()>*> m_function = nullptr;
        mutable std::atomic<bool> m_executeResult = false;

    };
}

#endif //GAMEENGINE_THREAD_H
