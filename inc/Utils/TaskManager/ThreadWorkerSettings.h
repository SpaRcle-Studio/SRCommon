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
            Vector<ThreadWorkerStateCondition> startConditions;
            /// @property
            Vector<ThreadWorkerStateCondition> skipConditions;
            /// @property
            Vector<ThreadWorkerStateCondition> finishConditions;
        };

        struct SR_COMMON_DLL_API ThreadWorkerThread : public Serializable {
            SR_STRUCT()

            /// @property
            StringAtom name = "Thread";
            /// @property
            bool useThreads = true;
            /// @property
            Vector<ThreadWorkerSettingsState> states;

        };

        struct SR_COMMON_DLL_API ThreadWorkerThreadsVariant : public Serializable {
            SR_STRUCT()

            /// @property
            String description;
            /// @property
            std::set<StringAtom> featuresCondition;
            /// @property
            Vector<ThreadWorkerThread> threads;
            /// @property
            Vector<StringAtom> finalizeStates;

        };
    }

    struct SR_COMMON_DLL_API ThreadWorkerSettings : public Asset {
        SR_STRUCT()

        /// @property
        Vector<Details::ThreadWorkerThreadsVariant> variants;

    };
}

#endif //SR_ENGINE_UTILS_THREAD_WORKER_SETTINGS_H
