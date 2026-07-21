//
// Created by Monika on 18.03.2022.
//

#include <Utils/Types/Thread.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Types/DataStorage.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_HTYPES_NS {
    Thread::Factory::Factory()
        : Singleton<Factory>()
    { }

    Thread::Factory::~Factory() {
        DeInitialize();
    }

    Thread::~Thread() {
        if (Joinable()) {
            SRHalt("Thread::~Thread() : thread is joinable! Thread: {}", m_id);
        }
        if (m_context) {
            delete m_context;
            m_context = nullptr;
        }
    }

    Thread::Thread(Thread::ThreadId id)
        : m_id(id)
    {
    #ifdef SR_THREADS_ALLOWED
        GetImpl().thread = std::thread();
    #endif
        m_context = new DataStorage();
    }

    Thread::Thread()
    #ifdef SR_THREADS_ALLOWE
        : Thread(std::thread())
    { }
    #else
    {
        GetImpl().isRan = false;
        m_context = new DataStorage();
    }
    #endif

    void Thread::Sleep(uint64_t milliseconds) {
        SR_TRACY_ZONE;
        Platform::Sleep(milliseconds);
    }

    void Thread::SetPriority(ThreadPriority priority) {
    #ifdef SR_THREADS_ALLOWED
        Platform::SetThreadPriority(reinterpret_cast<void*>(GetImpl().thread.native_handle()), priority);
    #endif
    }

    bool Thread::TryJoin() {
        if (Joinable()) {
            Join();
            return true;
        }

        return false;
    }

    Thread::Ptr Thread::Factory::GetMainThread() {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;
        SRAssert2(m_main, "Main thread isn't initialized!");
        return m_main;
    }

    Thread::Ptr Thread::Factory::GetThisThread() {
        SR_SCOPED_LOCK;

        auto&& pThisThread = TryGetThisThread();
        if (pThisThread) {
            return pThisThread;
        }

    #ifdef SR_DEBUG
        SR_MAYBE_UNUSED std::string threads;
        for (auto&& [id, pThread] : m_threads) {
            threads.append("\tThread [" + id.ToStringRef() + "]\n");
        }
        SRHalt("Thread::Factory::GetThisThread() : unknown thread!\n" + threads);
    #endif

        return nullptr;
    }

    void Thread::Factory::Remove(Thread* pThread) {
        SR_SCOPED_LOCK;

        SRAssert2(pThread, "Thread::Factory::Remove() : thread is null!");
        SR_LOG("Thread::Free() : free \"{}\" thread...", pThread->GetId().c_str());

        if (pThread == m_main) {
            delete m_main;
            m_main = nullptr;
        }
        else {
            m_threads.erase(pThread->GetId());
        }
    }

    Thread::ThreadId Thread::GetId() const {
        std::lock_guard lock(GetImpl().mutex);
        return m_id;
    }

    void Thread::Free() {
        Factory::Instance().Remove(this);
        delete this;
    }

    void Thread::Synchronize() {
        SR_TRACY_ZONE;
        std::lock_guard lock(GetImpl().mutex);

        auto&& pThread = Thread::Factory::Instance().GetThisThread();
        if (!pThread) {
            SRHalt("Thread::Synchronize() : unknown thread!");
            return;
        }

        if (GetId() != pThread->GetId()) {
            SRHalt("Synchronization can only be performed by the owner thread!");
            return;
        }

        if (GetImpl().nameChanged) {
            SR_TRACY_THREAD_NAME(m_name.c_str());
            GetImpl().nameChanged = false;
        }

        if (GetImpl().function) {
            GetImpl().executeResult = (*GetImpl().function)();
            GetImpl().function = nullptr;
        }
    }

    bool Thread::Execute(const SR_HTYPES_NS::Function<bool()>& function) const {
        if (GetId() == SR_UTILS_NS::GetThisThreadId()) {
            return function();
        }

        /// сначала дожидаемся предыдущей работы. Операция атомарная.
        while (GetImpl().function) {
            SR_TRACY_ZONE_N("Thread::Execute - wait previous");
            SR_NOOP;
        }

        /// синхронно записываем
        {
            std::lock_guard lock(GetImpl().mutex);
            GetImpl().function = &function;
        }

        /// синхронно ждем выволнения работы. Операция атомарная.
        while (GetImpl().function) {
            SR_TRACY_ZONE_N("Thread::Execute - wait");
            SR_NOOP;
        }

        return GetImpl().executeResult;
    }

    void Thread::SetName(StringView name) {
        std::lock_guard lock(GetImpl().mutex);
        m_name = name;
        GetImpl().nameChanged = true;
    }

    bool Thread::HasId() const {
        std::lock_guard lock(GetImpl().mutex);

        if (m_id.empty()) {
            return false;
        }

        if (m_id == "0") {
            return false;
        }

        if (m_id == EmptyThreadId()) {
            return false;
        }

        return true;
    }

    void Thread::Detach() {
    #ifdef SR_THREADS_ALLOWED
        GetImpl().thread.detach();
    #endif
    }

    void Thread::Join() {
    #ifdef SR_THREADS_ALLOWED
        GetImpl().thread.join();
    #endif
    }

    bool Thread::Joinable() const {
    #ifdef SR_THREADS_ALLOWED
        return GetImpl().thread.joinable();
    #else
        return false;
    #endif
    }

    DataStorage* Thread::GetContext() {
        return m_context;
    }

    Thread::ThreadId Thread::EmptyThreadId() {
        static const auto id = SR_UTILS_NS::StringAtom("[EMPTY]");
        return id;
    }

    ThreadImpl& Thread::GetImpl() const {
        if (!m_impl) {
            m_impl = new ThreadImpl();
        }
        return *m_impl;
    }

    void Thread::SetId(Thread::ThreadId id) {
        std::lock_guard lock(GetImpl().mutex);
        m_id = id;
    }

    uint32_t Thread::Factory::GetThreadsCount() {
        SR_SCOPED_LOCK;
        return m_threads.size();
    }

    void Thread::Factory::SetMainThread() {
        SR_LOCK_GUARD;

        SRAssert2(!m_main, "Main thread already initialized!");

        SR_LOG("Thread::Factory::SetMainThread() : initializing main thread...");

        m_main = new Thread(SR_UTILS_NS::GetThisThreadId());

        SR_LOG("Thread::Factory::SetMainThread() : main thread id: \"{}\"", m_main->GetId());
    }

    void Thread::Factory::PrintThreads() {
        SR_LOCK_GUARD;

        if (m_threads.empty()) {
            return;
        }

        std::string log = "Thread::Factory::PrintThreads() : threads:\n";

        for (auto&& [id, pThread] : m_threads) {
            if (pThread == m_main) {
                log += "\tThread [Main]\n";
            }
            else if (!pThread->m_name.empty()) {
                log += "\tThread [" + id.ToStringRef() + "] - " + pThread->m_name + "\n";
            }
            else {
                log += "\tThread [" + id.ToStringRef() + "]\n";
            }
        }

        SR_SYSTEM_LOG(log);
    }

    Thread::Ptr Thread::Factory::TryGetThisThread() {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        auto&& threadId = SR_UTILS_NS::GetThisThreadId();

        if (auto&& pIt = m_threads.find(threadId); pIt != m_threads.end()) {
            return pIt->second;
        }

        auto&& pMain = GetMainThread();
        if (pMain && threadId == pMain->GetId()) {
            return pMain;
        }

        return nullptr;
    }

    void Thread::Factory::DeInitialize() {
        SR_LOCK_GUARD;
        SRAssert(m_threads.empty() && "Thread::Factory::~Factory() : not all threads were freed!");
        if (m_main) {
            delete m_main;
            m_main = nullptr;
        }
    }
}

