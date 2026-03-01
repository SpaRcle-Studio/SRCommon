//
// Created by Monika on 01.10.2025.
//

#ifndef SR_ENGINE_UTILS_THREAD_WORKER_SETTINGS_H
#define SR_ENGINE_UTILS_THREAD_WORKER_SETTINGS_H

#include <Utils/Resources/Asset.h>
#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(ThreadWorkerState, uint8_t,
        Idle, Working, Ready
    );

    namespace Details {
        struct SR_COMMON_DLL_API ThreadWorkerStateCondition : public Serializable {
            SR_STRUCT()

            /// @property
            StringAtom name;
            /// @property
            ThreadWorkerState state = ThreadWorkerState::Idle;
        };

        struct SR_COMMON_DLL_API ThreadWorkerSettingsState : public Serializable {
            SR_STRUCT()

            /// @property
            StringAtom name;
            /// @property
            std::vector<ThreadWorkerStateCondition> startConditions;
            /// @property
            std::vector<ThreadWorkerStateCondition> skipConditions;
            /// @property
            std::vector<ThreadWorkerStateCondition> finishConditions;
        };

        struct SR_COMMON_DLL_API ThreadWorkerThread : public Serializable {
            SR_STRUCT()

            /// @property
            StringAtom name = "Thread";
            /// @property
            bool useThreads = true;
            /// @property
            std::vector<ThreadWorkerSettingsState> states;

        };

        struct SR_COMMON_DLL_API ThreadWorkerThreadsVariant : public Serializable {
            SR_STRUCT()

            /// @property
            std::string description;
            /// @property
            std::set<StringAtom> featuresCondition;
            /// @property
            std::vector<ThreadWorkerThread> threads;
            /// @property
            std::vector<StringAtom> finalizeStates;

        };
    }

    struct SR_COMMON_DLL_API ThreadWorkerSettings : public Asset {
        SR_STRUCT()

        /// @property
        std::vector<Details::ThreadWorkerThreadsVariant> variants;

    };
}

#endif //SR_ENGINE_UTILS_THREAD_WORKER_SETTINGS_H
