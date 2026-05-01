//
// Created by Monika on 22.02.2026
//

#include <Utils/TaskManager/TaskManager.h>
#include <Utils/Platform/Platform.h>

namespace SR_UTILS_NS {
    Task::Task(TaskFn fn, bool createThread, TaskPriority priority)
        : m_createThread(createThread)
        , m_id(SR_UINT64_MAX)
        , m_function(std::move(fn))
        , m_state(new std::atomic<State>())
        , m_thread(nullptr)
        , m_priority(priority)
    {
        m_state->store(State::Waiting);
    }

    Task::~Task() {
        SRAssert(!m_state || m_state->load() == State::Completed);
        SRAssert(!m_thread || !m_thread->Joinable());

        if (m_thread) {
            if (m_thread->Joinable()) {
                m_thread->Detach();
            }

            m_thread->Free();
            m_thread = nullptr;
        }

        if (m_state) {
            delete m_state;
        }
    }

    bool Task::Stop() {
        if (m_state->load() != State::Launched || !m_thread->Joinable()) {
            SRAssert(false);
            return false;
        }

        if (m_thread) {
            m_thread->Detach();
            m_thread->Free();
            m_thread = nullptr;
        }

        m_state->store(State::Stopped);

        return true;
    }

    bool Task::Run() {
        if (m_state->load() != State::Waiting || !m_function || (m_thread && m_thread->Joinable())) {
            SRAssert(false);
            return false;
        }

        m_state->store(State::Launched);

        if (m_createThread) {
            SR_HTYPES_NS::Thread::Factory::Instance().Create(m_thread, m_function, m_state);
            m_thread->SetName("Task");
        }
        else {
            m_function(m_state);
        }

        return true;
    }

    bool Task::IsCompleted() const {
        const State state = m_state->load();
        return (state == State::Completed || state == State::Failed || state == State::Stopped) && (!m_thread || !m_thread->Joinable());
    }

    Task::State Task::GetResult() const {
        return m_state->load();
    }

    uint64_t Task::GetId() const {
        return m_id;
    }

    TaskPriority Task::GetPriority() const {
        return m_priority;
    }

    bool Task::IsWaiting() const {
        return m_state->load() == State::Waiting;
    }

    bool Task::IsLaunched() const {
        return m_state->load() == State::Launched;
    }

    void Task::SetId(uint64_t id) {
        m_id = id;
    }

    TaskManager::~TaskManager() {
        if (!m_tasks.empty() || !m_ids.empty()) {
            SRAssert(false);

            for (const Task::Ptr& pTask : m_tasks) {
                pTask->Stop();
            }
        }

        m_tasks.clear();
        m_ids.clear();

        SRAssert(!m_thread);
    }

    uint64_t TaskManager::GetUniqueId() const {
        static std::atomic<uint64_t> idCounter = 0;
        return ++idCounter;
    }

    void TaskManager::InitSingleton() {
        if (m_isRun.load()) {
            SR_ERROR("TaskManager::InitSingleton() : task manager is already ran!");
            return;
        }

        m_isRun.store(true);

        SR_INFO("TaskManager::InitSingleton() : run task manager thread...");

        SR_HTYPES_NS::Thread::Factory::Instance().Create(m_thread, [this]() {
            std::set<Task*> delayedTasks;

            while (m_isRun.load()) {
                SR_TRACY_ZONE_N("TaskManager");

                bool isEmpty = false;
                {
                    SR_LOCK_GUARD;
                    isEmpty = m_tasks.empty();
                }

                if (isEmpty) {
                    SR_TRACY_ZONE_COLOR(0x000088);
                    delayedTasks.clear();
                    SR_PLATFORM_NS::Sleep(5);
                    continue;
                }

                SR_TRACY_ZONE_COLOR(0x008800);

                TaskPriority highestPriority = TaskPriority::Unknown;
                Task::Ptr pTaskWithHighestPriority = nullptr;
                {
                    SR_LOCK_GUARD;

                    for (auto&& pTask : m_tasks) {
                        if (pTask->IsWaiting() && pTask->GetPriority() > highestPriority && !delayedTasks.count(pTask.get())) {
                            highestPriority = pTask->GetPriority();
                        }
                    }

                    if (highestPriority == TaskPriority::Unknown) {
                        delayedTasks.clear();
                        continue;
                    }

                    for (auto&& pTask : m_tasks) {
                        if (pTask->IsWaiting() && pTask->GetPriority() == highestPriority) {
                            pTaskWithHighestPriority = pTask;
                            break;
                        }
                    }
                }

                if (pTaskWithHighestPriority) {
                    pTaskWithHighestPriority->Run();
                    if (pTaskWithHighestPriority->IsCompleted()) {
                        SR_LOCK_GUARD;
                        m_results.insert(std::make_pair(pTaskWithHighestPriority->GetId(), pTaskWithHighestPriority->GetResult()));
                        m_ids.erase(pTaskWithHighestPriority->GetId());
                        auto&& pIt = std::ranges::find(m_tasks, pTaskWithHighestPriority); /// удаляем задачу из списка задач, так как она уже завершилась
                        if (pIt != m_tasks.end()) {
                            m_tasks.erase(pIt);
                        }
                        else {
                            SRHalt("TaskManager::InitSingleton() : task is completed, but not found in the list of tasks!");
                        }
                    }
                    else {
                        delayedTasks.insert(pTaskWithHighestPriority.get());
                    }
                }
            }
        });

        m_thread->SetName("Task manager");

        if (!m_thread->Joinable()) {
            SR_ERROR("TaskManager::InitSingleton() : failed to run a thread!");
        }

        Singleton::InitSingleton();
    }

    TaskManager::TaskId TaskManager::ExecuteAsync(const SR_HTYPES_NS::Function<void()>& function, TaskPriority priority) {
        SR_LOCK_GUARD;

        auto&& pTask = new Task([function](std::atomic<Task::State>* pState) {
            function();
            pState->store(Task::State::Completed);
        }, false, priority);

        const uint64_t uniqueId = GetUniqueId();
        pTask->SetId(uniqueId);
        m_ids.insert(uniqueId);
        m_tasks.emplace_back(pTask);

        return uniqueId;
    }

    TaskManager::TaskId TaskManager::ExecuteParallel(const Function<void()> &function, TaskPriority priority) {
        SR_LOCK_GUARD;

        auto&& pTask = new Task([function](std::atomic<Task::State>* pState) {
            function();
            pState->store(Task::State::Completed);
        }, true, priority);

        const uint64_t uniqueId = GetUniqueId();
        pTask->SetId(uniqueId);
        m_ids.insert(uniqueId);
        m_tasks.emplace_back(pTask);

        return uniqueId;
    }

    Task::State SR_UTILS_NS::TaskManager::GetResult(uint64_t taskId) const {
        if (m_ids.count(taskId) == 1) {
            return Task::State::Launched;
        }

        return Task::State::Unknown;
    }

    void TaskManager::OnSingletonDestroy() {
        m_isRun = false;

        if (m_thread) {
            m_thread->TryJoin();
            m_thread->Free();
            m_thread = nullptr;
        }

        Singleton::OnSingletonDestroy();
    }

    bool TaskManager::IsActive(TaskManager::TaskId taskId) const {
        SR_TRACY_ZONE;
        SR_SCOPED_LOCK;
        return m_ids.count(taskId) == 1;
    }

    void TaskManager::Update() {
        SR_TRACY_ZONE;

        Task::Ptr pCriticalTask = nullptr;

        {
            SR_LOCK_GUARD;

            bool hasAnyLaunchedTask = false;

            for (auto&& pTask : m_tasks) {
                if (pTask->IsLaunched()) {
                    hasAnyLaunchedTask = true;
                    break;
                }
            }

            if (!hasAnyLaunchedTask) {
                return;
            }

            for (auto pIt = m_tasks.begin(); pIt != m_tasks.end();) {
                auto&& pTask = *pIt;

                TaskPriority priority = pTask->GetPriority();
                if (priority == TaskPriority::Critical) {
                    if (pTask->IsLaunched()) {
                        return;
                    }

                    if (pTask->IsWaiting()) {
                        pCriticalTask = pTask;
                        m_tasks.erase(pIt);
                        break;
                    }
                }

                ++pIt;
            }
        }

        /// Если есть критическая задача и поток занят чем то менее важным,
        /// то выполняем её в главном потоке, так как она является самой важной задачей в данный момент.
        if (pCriticalTask) {
            SR_LOG("TaskManager::Update() : run critical task with id {} in main thread...", pCriticalTask->GetId());

            pCriticalTask->Run();
            if (pCriticalTask->IsCompleted()) {
                SR_LOCK_GUARD;
                m_results.insert(std::make_pair(pCriticalTask->GetId(), pCriticalTask->GetResult()));
                m_ids.erase(pCriticalTask->GetId());
            }
            else {
                SR_LOCK_GUARD;
                m_tasks.emplace_back(pCriticalTask);
            }
        }
    }

    void TaskManager::WaitForIdle() {
        SR_TRACY_ZONE;
        for (;;) {
            Update();

            bool isIdle = true;

            {
                SR_LOCK_GUARD;
                for (auto&& pTask : m_tasks) {
                    if (!pTask->IsCompleted()) {
                        isIdle = false;
                        break;
                    }
                }
            }

            if (isIdle) {
                return;
            }

            SR_PLATFORM_NS::Sleep(5);
        }
    }

    void TaskManager::RemoveDiscardable() {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        for (auto pIt = m_tasks.begin(); pIt != m_tasks.end();) {
            auto&& pTask = *pIt;
            if (pTask->GetPriority() == TaskPriority::Discardable) {
                if (pTask->IsLaunched()) {
                    pTask->Stop();
                }

                SR_LOG("TaskManager::RemoveDiscardable() : remove discardable task with id {}...", pTask->GetId());
                m_ids.erase(pTask->GetId());
                pIt = m_tasks.erase(pIt);
            }
            else {
                ++pIt;
            }
        }
    }
}