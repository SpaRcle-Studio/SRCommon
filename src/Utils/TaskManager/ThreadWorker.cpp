//
// Created by Monika on 10.04.2024.
//

#include <Utils/TaskManager/ThreadWorker.h>
#include <Utils/Resources/ResourceManager.h>
#include <Utils/Debug.h>
#include <Utils/Platform/Platform.h>

#include <rapidyaml/src/ryml.hpp>
#include <rapidyaml/src/ryml_std.hpp>

namespace SR_UTILS_NS {
    ThreadWorkerStateBase::ThreadWorkerStateBase()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    void ThreadWorkerStateBase::AddStartCondition(StringAtom name, ThreadWorkerState state) {
        SRAssert2(m_startConditions.count(name) == 0, "ThreadWorkerStateBase::AddStartCondition() : start condition \"{}\" already exists!", name.ToStringRef());
        m_startConditions[name] = state;
    }

    void ThreadWorkerStateBase::AddFinishCondition(StringAtom name, ThreadWorkerState state) {
        SRAssert2(m_finishConditions.count(name) == 0, "ThreadWorkerStateBase::AddFinishCondition() : finish condition \"{}\" already exists!", name.ToStringRef());
        m_finishConditions[name] = state;
    }

    ThreadWorkerResult ThreadWorkerStateBase::Execute() {
        SR_TRACY_ZONE_S(GetName().ToStringRef().c_str());

        if (m_state == ThreadWorkerState::Idle) {
            bool isNeedToSkip = !m_skipConditions.empty();

            for (auto&& [name, state] : m_skipConditions) {
                isNeedToSkip &= GetThreadWorker()->GetThreadsWorker()->GetState(name) == state;
                if (!isNeedToSkip) {
                    break;
                }
            }

            if (isNeedToSkip) {
                return ThreadWorkerResult::Success;
            }

            for (auto&& [name, state] : m_startConditions) {
                if (GetThreadWorker()->GetThreadsWorker()->GetState(name) != state) {
                    return ThreadWorkerResult::Repeat;
                }
            }

            m_state = ThreadWorkerState::Working;
        }

        if (m_state == ThreadWorkerState::Working) {
            auto&& result = ExecuteImpl();
            if (result != ThreadWorkerResult::Success) {
                return result;
            }
            m_state = ThreadWorkerState::Ready;
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
        SRAssert2(m_skipConditions.count(name) == 0, "ThreadWorkerStateBase::AddSkipCondition() : skip condition \"{}\" already exists!", name.ToStringRef());
        m_skipConditions[name] = state;
    }

    SR_HTYPES_NS::DataStorage& ThreadWorkerStateBase::GetContext() {
        return GetThreadWorker()->GetThreadsWorker()->GetContext();
    }

    ThreadWorker::ThreadWorker(std::string name)
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
        , m_name(std::move(name))
    { }

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

        m_thread = SR_HTYPES_NS::Thread::Factory::Instance().Create(&ThreadWorker::Work, this);
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

    void ThreadWorker::Work() {
        while (m_isActive) {
            SR_TRACY_ZONE_S(m_name.c_str());

            if (m_currentState >= m_states.size()) {
                m_currentState = 0;
                continue;
            }

            switch (m_states[m_currentState]->Execute()) {
                case ThreadWorkerResult::Success:
                    break;
                case ThreadWorkerResult::Repeat:
                    continue;
                case ThreadWorkerResult::Break:
                    m_currentState = 0;
                    continue;
                default:
                    SRHalt("ThreadWorker::Work() : unknown result!");
                    continue;
            }

            ++m_currentState;
        }
    }

    ThreadsWorker::ThreadsWorker()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    ThreadsWorker::Ptr ThreadsWorker::Load(const SR_UTILS_NS::Path& path) {
        auto&& fullPath = SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat(path);
        if (!fullPath.Exists(Path::Type::File)) {
            SR_ERROR("ThreadsWorker::Load() : file \"{}\" not found!", fullPath.ToStringRef());
            return nullptr;
        }

        std::string fileContents = FileSystem::ReadAllText(fullPath.ToStringRef());
        if (fileContents.empty()) {
            SR_ERROR("ThreadsWorker::Load() : failed to read file \"{}\"", fullPath.ToStringRef());
            return nullptr;
        }

        ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(fileContents));
        if (tree.empty()) {
            SR_ERROR("ThreadsWorker::Load() : failed to parse file \"{}\"", fullPath.ToStringRef());
            return nullptr;
        }

        ryml::ConstNodeRef root = tree.crootref();
        if (!root.valid()) {
            SR_ERROR("ThreadsWorker::Load() : failed to get root node from file \"{}\"", fullPath.ToStringRef());
        }

        auto&& threadsNode = root.find_child("threads");
        if (!threadsNode.valid()) {
            SR_ERROR("ThreadsWorker::Load() : failed to get \"threads\" node from file \"{}\"", fullPath.ToStringRef());
            return nullptr;
        }

        ThreadsWorker::Ptr pThreadsWorker = new ThreadsWorker();

        for (auto&& threadNode : threadsNode) {
            auto&& threadName = threadNode.find_child("name");
            if (!threadName.valid()) {
                continue;
            }

            auto&& states = threadNode.find_child("states");
            if (!states.valid()) {
                continue;
            }

            std::string threadNameStr(threadName.val().begin(), threadName.val().end());
            ThreadWorker::Ptr pThreadWorker = new ThreadWorker(threadNameStr);

            static auto processCondition = [](int type, ryml::ConstNodeRef conditionNode, const ThreadWorkerStateBase::Ptr& pState) {
                if (!conditionNode.valid()) {
                    return;
                }

                for (auto&& state : SR_UTILS_NS::EnumReflector::GetValues<ThreadWorkerState>()) {
                    auto&& stateName = SR_UTILS_NS::StringUtils::ToLower(state.name);

                    if (auto&& stateNode = conditionNode.find_child(stateName.c_str()); stateNode.valid()) {
                        for (auto&& item : stateNode) {
                            switch (type) {
                            case 0:
                                pState->AddSkipCondition(std::string(item.val().begin(), item.val().end()), static_cast<ThreadWorkerState>(state.value));
                                break;
                            case 1:
                                pState->AddStartCondition(std::string(item.val().begin(), item.val().end()), static_cast<ThreadWorkerState>(state.value));
                                break;
                            case 2:
                                pState->AddFinishCondition(std::string(item.val().begin(), item.val().end()), static_cast<ThreadWorkerState>(state.value));
                                break;
                            default:
                                SRHalt("ThreadsWorker::Load() : unknown condition type!");
                            }
                        }
                    }
                }
            };

            for (auto&& state : states) {
                auto&& stateName = state.find_child("name");
                if (!stateName.valid()) {
                    continue;
                }

                std::string stateNameStr(stateName.val().begin(), stateName.val().end());

                ThreadWorkerStateBase::Ptr pState = ThreadWorkerStateRegistration::Instance().AllocateState(stateNameStr);
                if (!pState) {
                    SR_ERROR("ThreadsWorker::Load() : failed to allocate state \"{}\" for thread \"{}\"", stateNameStr, threadNameStr);
                    continue;
                }

                processCondition(0, state.find_child("start_condition"), pState);
                processCondition(1, state.find_child("skip_condition"), pState);
                processCondition(2, state.find_child("finish_condition"), pState);

                pThreadWorker->AddState(pState);
            }

            pThreadsWorker->AddThread(pThreadWorker);
        }

        return pThreadsWorker;
    }

    void ThreadsWorker::AddThread(ThreadWorker::Ptr pThread) {
        pThread->SetThreadsWorker(this);
        for (auto&& pState : pThread->GetStates()) {
            if (m_states.count(pState->GetName()) == 1) {
                SR_ERROR("ThreadsWorker::AddThread() : state \"{}\" already exists!", pState->GetName().ToStringRef());
                continue;
            }
            m_states[pState->GetName()] = pState;
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

    bool ThreadWorkerStateRegistration::RegisterState(SR_UTILS_NS::StringAtom name, ThreadWorkerStateRegistration::AllocateFn&& allocateFn) {
        if (m_states.count(name) == 1) {
            /// уже зарегистрирован. возможно причина в использовании dll. считаем это нормой.
            return false;
        }

        if (!allocateFn) {
            SR_PLATFORM_NS::WriteConsoleError(SR_FORMAT("ThreadWorkerStateRegistration::RegisterState() : allocate function is nullptr!"));
            return false;
        }

        m_states[name] = std::move(allocateFn);

        return true;
    }

    ThreadWorkerStateBase::Ptr ThreadWorkerStateRegistration::AllocateState(SR_UTILS_NS::StringAtom name) {
        if (m_states.count(name) == 0) {
            SR_ERROR("ThreadWorkerStateRegistration::AllocateState() : state \"{}\" not found!", name.ToStringRef());
            return nullptr;
        }

        return m_states[name]();
    }
}

