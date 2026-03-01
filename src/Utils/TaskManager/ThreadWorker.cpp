//
// Created by Monika on 10.04.2024.
//

#include <Utils/TaskManager/ThreadWorker.h>

#include <Utils/Resources/ResourceManager.h>
#include <Utils/Resources/Yaml.h>

#include <Utils/Platform/Platform.h>
#include <Utils/Types/Time.h>
#include <Utils/Types/Thread.h>
#include <Utils/Types/DataStorage.h>
#include <Utils/Common/Features.h>
#include <Utils/TypeTraits/Factory.h>
#include <Utils/TaskManager/ThreadWorkerSettings.h>

#include <Codegen/ThreadWorker.generated.hpp>

#include <rapidyaml/src/ryml.hpp>
#include <rapidyaml/src/ryml_std.hpp>

namespace SR_UTILS_NS {
    ThreadWorkerStateBase::ThreadWorkerStateBase()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        , m_state(ThreadWorkerState::Idle)
    { }

    void ThreadWorkerStateBase::AddStartCondition(StringAtom name, ThreadWorkerState state) {
        SRAssert2(m_startConditions.count(name) == 0, "ThreadWorkerStateBase::AddStartCondition() : start condition \"{}\" already exists!", name);
        m_startConditions[name] = state;
    }

    void ThreadWorkerStateBase::AddFinishCondition(StringAtom name, ThreadWorkerState state) {
        SRAssert2(m_finishConditions.count(name) == 0, "ThreadWorkerStateBase::AddFinishCondition() : finish condition \"{}\" already exists!", name);
        m_finishConditions[name] = state;
    }

    ThreadWorkerResult ThreadWorkerStateBase::TryExecute() {
        if (m_state == ThreadWorkerState::Idle) {
            bool isNeedToSkip = !m_skipConditions.empty();

            for (auto&& [name, state] : m_skipConditions) {
                isNeedToSkip &= GetThreadWorker()->GetThreadsWorker()->GetState(name) == state;
                if (!isNeedToSkip) {
                    break;
                }
            }

            if (isNeedToSkip) {
                return ThreadWorkerResult::Skip;
            }

            for (auto&& [name, state] : m_startConditions) {
                if (GetThreadWorker()->GetThreadsWorker()->GetState(name) != state) {
                    return ThreadWorkerResult::Repeat;
                }
            }

            m_state = ThreadWorkerState::Working;
        }
        return ThreadWorkerResult::Success;
    }

    ThreadWorkerResult ThreadWorkerStateBase::Execute() {
        SR_TRACY_ZONE_S(GetMeta()->GetFactoryName().c_str());

        if (m_state == ThreadWorkerState::Working) {
            auto&& result = ExecuteImpl();
            if (result != ThreadWorkerResult::Success) {
                return result;
            }
            m_state = ThreadWorkerState::Ready;
        }
        else {
            SRHalt("ThreadWorkerStateBase::Execute() : state is not working!");
        }

        for (auto&& [name, state] : m_finishConditions) {
            if (GetThreadWorker()->GetThreadsWorker()->GetState(name) != state) {
                return ThreadWorkerResult::Repeat;
            }
        }

        m_state = ThreadWorkerState::Idle;

        return ThreadWorkerResult::Success;
    }

    void ThreadWorkerStateBase::AddSkipCondition(SR_UTILS_NS::StringAtom name, ThreadWorkerState state) {
        SRAssert2(m_skipConditions.count(name) == 0, "ThreadWorkerStateBase::AddSkipCondition() : skip condition \"{}\" already exists!", name);
        m_skipConditions[name] = state;
    }

    SR_HTYPES_NS::DataStorage& ThreadWorkerStateBase::GetContext() {
        return GetThreadWorker()->GetThreadsWorker()->GetContext();
    }

    ThreadsWorker* ThreadWorkerStateBase::GetThreadsWorker() const {
        return GetThreadWorker()->GetThreadsWorker();
    }

    void ThreadWorkerStateBase::Finalize() {
        SR_TRACY_ZONE_S(GetMeta()->GetFactoryName().c_str());
        FinalizeImpl();
    }

    ThreadWorker::ThreadWorker(std::string name)
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        , m_name(std::move(name))
    { }

    ThreadWorker::~ThreadWorker() = default;

    void ThreadWorker::AddState(ThreadWorkerStateBase::Ptr pState) {
        if (GetThreadsWorker()) {
            SRHalt("ThreadWorker::AddState() : adding state to created thread worker is not allowed!");
            return;
        }

        pState->SetThreadWorker(this);
        m_states.emplace_back(std::move(pState));
    }

    void ThreadWorker::Start() {
        SRAssert(!m_isActive);
        m_isActive = true;

        if (m_useThreads) {
            if (!SR_HTYPES_NS::Thread::Factory::Instance().Create(m_thread, &ThreadWorker::Work, this)) {
                SRHalt("ThreadWorker::Start() : failed to create thread!");
                return;
            }

            m_thread->SetName(m_name);
        }
    }

    void ThreadWorker::Stop() {
        SRAssert(m_isActive);
        m_isActive = false;

        if (m_thread) {
            if (m_thread->Joinable()) {
                m_thread->Join();
            }
            m_thread->Free();
            m_thread = nullptr;
        }
    }

    void ThreadWorker::CheckFinalize() {
        for (auto&& pState : m_states) {
            if (GetThreadsWorker()->CheckFinalize(pState->GetMeta()->GetFactoryName())) {
                SR_LOG("ThreadWorker::Work() : finalize state \"{}\"", pState->GetMeta()->GetFactoryName());
                pState->Finalize();
            }
        }
    }

    void ThreadWorker::Work() {
        SR_TRACY_THREAD_NAME(m_name.c_str());

        while (true) {
            SR_TRACY_ZONE_S(m_name.c_str());
            SR_TRACY_FRAME_MARK_N(m_name.c_str());

            if (m_useThreads) {
                m_thread->Synchronize();
            }

            if (!m_isActive) {
                break;
            }

            if (!GetThreadsWorker()->IsAlive()) {
                CheckFinalize();
                if (!m_useThreads) {
                    break;
                }
                continue;
            }

            Update();

            if (!m_useThreads) {
                break;
            }
        }
    }

    void ThreadWorker::Update() {
        while (m_currentState < m_states.size()) {
            if (!m_isActive || !GetThreadsWorker()->IsAlive()) {
                return;
            }

            bool skip = false;
            {
                SR_TRACY_ZONE_N("Wait");
                SR_TRACY_ZONE_COLOR(0xFF0000);

                std::atomic<bool> wait = true;
                while (wait) {
                    switch (m_states[m_currentState]->TryExecute()) {
                        case ThreadWorkerResult::Success:
                            wait = false;
                            break;
                        case ThreadWorkerResult::Repeat:
                            break;
                        case ThreadWorkerResult::Skip:
                            skip = true;
                            break;
                        default:
                            SRHalt("ThreadWorker::Work() : unknown result!");
                            break;
                    }
                }
            }

            if (!skip) {
                switch (m_states[m_currentState]->Execute()) {
                    case ThreadWorkerResult::Success:
                    case ThreadWorkerResult::Skip:
                        break;
                    case ThreadWorkerResult::Repeat:
                        continue;
                    case ThreadWorkerResult::Break:
                        m_currentState = 0;
                        return;
                    default:
                        SRHalt("ThreadWorker::Work() : unknown result!");
                        continue;
                }
            }

            ++m_currentState;
        }

        if (m_currentState >= m_states.size()) {
            m_currentState = 0;
        }
    }

    ThreadsWorker::ThreadsWorker()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    ThreadsWorker::Ptr ThreadsWorker::Load(const SR_UTILS_NS::Path& path) {
        SR_TRACY_ZONE;

        SR_LOG("ThreadsWorker::Load() : loading threads worker from file \"{}\"...", path);

        auto&& pSettings = SR_UTILS_NS::Asset::Load<ThreadWorkerSettings>(path);
        if (!pSettings) {
            SR_ERROR("ThreadsWorker::Load() : failed to load settings from file \"{}\"", path);
            return nullptr;
        }

        std::optional<Details::ThreadWorkerThreadsVariant> settingsVariant;
        for (auto&& variant : pSettings->variants) {
            bool isSuitable = std::ranges::all_of(variant.featuresCondition, [](const auto& featureCondition) {
                return SR_UTILS_NS::Features::Instance().Enabled(featureCondition, false);
            });

            if (isSuitable) {
                settingsVariant = variant;
                break;
            }
        }

        if (!settingsVariant) {
            SR_ERROR("ThreadsWorker::Load() : failed to find suitable variant in file \"{}\"! Check features conditions!", path);
            return nullptr;
        }

        SR_LOG("ThreadsWorker::Load() : found suitable threads variant, description: \"{}\"", settingsVariant.value().description);

        ThreadsWorker::Ptr pThreadsWorker = new ThreadsWorker();

        for (auto&& finalize : settingsVariant.value().finalizeStates) {
            if (!SR_UTILS_NS::Factory::Instance().GetType(finalize)) {
                SR_ERROR("ThreadsWorker::Load() : failed to find finalize state \"{}\" in file \"{}\"!", finalize, path);
                continue;
            }
            pThreadsWorker->m_finalize.emplace_back(finalize);
        }

        for (const Details::ThreadWorkerThread& thread : settingsVariant.value().threads) {
            ThreadWorker::Ptr pThreadWorker = new ThreadWorker(thread.name);
            pThreadWorker->SetUseThreads(thread.useThreads);

            for (const Details::ThreadWorkerSettingsState& stateName : thread.states) {
                auto&& pState = SR_UTILS_NS::Factory::Instance().Create<ThreadWorkerStateBase>(stateName.name);
                if (!pState) {
                    SR_ERROR("ThreadsWorker::Load() : failed to allocate state \"{}\" for thread \"{}\" in file \"{}\"!", stateName.name, thread.name, path);
                    continue;
                }

                for (const auto& condition : stateName.startConditions) {
                    pState->AddStartCondition(condition.name, condition.state);
                }

                for (const auto& condition : stateName.skipConditions) {
                    pState->AddSkipCondition(condition.name, condition.state);
                }

                for (const auto& condition : stateName.finishConditions) {
                    pState->AddFinishCondition(condition.name, condition.state);
                }

                pThreadWorker->AddState(pState);
            }

            pThreadsWorker->AddThread(pThreadWorker);
        }

        return pThreadsWorker;
    }

    void ThreadsWorker::AddThread(ThreadWorker::Ptr pThread) {
        pThread->SetThreadsWorker(this);
        for (auto&& pState : pThread->GetStates()) {
            if (m_states.count(pState->GetMeta()->GetFactoryName()) == 1) {
                SR_ERROR("ThreadsWorker::AddThread() : state \"{}\" already exists!", pState->GetMeta()->GetFactoryName());
                continue;
            }
            m_states[pState->GetMeta()->GetFactoryName()] = pState;
        }
        m_threadWorkers.emplace_back(std::move(pThread));
    }

    void ThreadsWorker::Start() {
        SR_TRACY_ZONE;
        SRAssert(!m_isActive);
        m_isActive = true;

        for (auto&& pThread : m_threadWorkers) {
            pThread->Start();
        }
    }

    void ThreadsWorker::Stop() {
        SR_TRACY_ZONE;

        SR_LOG("ThreadsWorker::Stop() : stopping threads...");

        if (!m_finalize.empty()) {
            SR_LOG("ThreadsWorker::Stop() : waiting for finalize {} states...", m_finalize.size());
        }

        m_isAlive = false;

        while (!m_finalize.empty()) {
            Execute();
        }

        SRAssert(m_isActive);
        m_isActive = false;

        for (auto&& pThread : m_threadWorkers) {
            pThread->Stop();
        }
    }

    ThreadWorkerState ThreadsWorker::GetState(SR_UTILS_NS::StringAtom name) const {
        auto&& pIt = m_states.find(name);
        if (pIt == m_states.end()) {
            SR_ERROR("ThreadsWorker::GetState() : state \"{}\" not found!", name.ToStringRef());
            return ThreadWorkerState::Idle;
        }

        return pIt->second->GetState();
    }

    const ThreadWorkerStateBase::Ptr& ThreadsWorker::GetWorkerState(SR_UTILS_NS::StringAtom name) const {
        auto&& pIt = m_states.find(name);
        static ThreadWorkerStateBase::Ptr nullPtr = nullptr;
        if (pIt == m_states.end()) {
            return nullPtr;
        }

        return pIt->second;
    }

    bool ThreadsWorker::IsAlive() const {
        return m_isAlive;
    }

    bool ThreadsWorker::CheckFinalize(SR_UTILS_NS::StringAtom name) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        if (m_finalize.empty()) {
            return false;
        }

        if (m_finalize.front() == name) {
            m_finalize.pop_front();
            return true;
        }

        return false;
    }

    SR_HTYPES_NS::DataStorage& ThreadsWorker::GetContext() {
        if (!m_context) {
            m_context = new SR_HTYPES_NS::DataStorage();
        }
        return *m_context;
    }

    void ThreadsWorker::Execute() {
        SR_TRACY_ZONE;

        for (auto&& pThread : m_threadWorkers) {
            if (!pThread->IsNeedUseThreads()) {
                pThread->Work();
            }
        }
    }
}

