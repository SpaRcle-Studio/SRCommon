//
// Created by Monika on 19.03.2022.
//

#ifndef SR_ENGINE_TASKMANAGER_H
#define SR_ENGINE_TASKMANAGER_H

#include <Utils/Common/Singleton.h>
#include <Utils/Common/NonCopyable.h>
#include <Utils/Types/Thread.h>
#include <Utils/Types/Function.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(TaskPriority, uint8_t,
        Unknown, Discardable, Low, Normal, High, Critical
    );

    enum class TaskState : uint8_t {
        Unknown, Waiting, Launched, Stopped, Completed, Failed
    };

    class SR_COMMON_DLL_API Task : public NonCopyable {
    public:
        using Ptr = std::shared_ptr<Task>;
        using State = TaskState;
        using StatePtr = std::atomic<State>*;
        using AtomicState = std::atomic<State>;

        using TaskFn = SR_HTYPES_NS::Function<bool(StatePtr)>;

    public:
        explicit Task(TaskFn fn, bool createThread, TaskPriority priority);
        ~Task() override;

    public:
        bool Run();
        bool Stop();

        void SetId(uint64_t id);

        SR_NODISCARD bool IsCompleted() const;
        SR_NODISCARD bool IsWaiting() const;
        SR_NODISCARD bool IsLaunched() const;
        SR_NODISCARD bool IsStopped() const;
        SR_NODISCARD State GetResult() const;
        SR_NODISCARD uint64_t GetId() const;
        SR_NODISCARD TaskPriority GetPriority() const;

    private:
        bool m_createThread = false;
        uint64_t m_id;
        Types::Thread::Ptr m_thread;
        TaskFn m_function;
        TaskPriority m_priority = TaskPriority::Normal;
        /// должен быть динамическим, иначе может потеряться ссылка при перемещении
        StatePtr m_state;

    };

    class SR_COMMON_DLL_API TaskManager : public Singleton<TaskManager> {
        SR_REGISTER_SINGLETON(TaskManager)
        using TaskFn = SR_HTYPES_NS::Function<void(std::atomic<Task::State>*)>;
        using TaskId = uint64_t;
    public:
        ~TaskManager() override;

    public:
        void Update();

        TaskId ExecuteAsync(const SR_HTYPES_NS::Function<void(Task::AtomicState&)>& function, TaskPriority priority);
        TaskId ExecuteParallel(const SR_HTYPES_NS::Function<void(Task::AtomicState&)>& function, TaskPriority priority);

        Task::State GetResult(TaskId taskId) const;
        bool IsActive(TaskId taskId) const;
        void Wait(TaskId taskId) const;

        void WaitForIdle();
        void RemoveDiscardable();

    private:
        SR_NODISCARD uint64_t GetUniqueId() const;
        void OnSingletonDestroy() override;
        void InitSingleton() override;

    private:
        SR_HTYPES_NS::Thread::Ptr m_thread = nullptr;
        std::atomic<bool> m_isRun;
        std::vector<Task::Ptr> m_tasks;
        /// Предполагается, что задач не будет слишком много,
        /// и не будет надобности в unordered set/map
        std::set<TaskId> m_ids;
        mutable std::map<TaskId, Task::State> m_results;
        std::set<Task*> m_delayedTasks;

    };
}

#endif // SR_ENGINE_TASKMANAGER_H
