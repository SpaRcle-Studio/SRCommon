//
// Created by Monika on 01.10.2025.
//

#ifndef SR_ENGINE_UTILS_THREAD_WORKER_SETTINGS_H
#define SR_ENGINE_UTILS_THREAD_WORKER_SETTINGS_H

#include <Utils/Resources/Asset.h>
#include <Utils/Common/Enumerations.h>
#include <Utils/Types/Set.h>
#include <Utils/Types/Vector.h>

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

            bool operator==(const ThreadWorkerStateCondition& other) const noexcept {
                return name == other.name && state == other.state;
            }

            bool operator!=(const ThreadWorkerStateCondition& other) const noexcept {
                return !(*this == other);
            }
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

            bool operator==(const ThreadWorkerSettingsState& other) const noexcept {
                return name == other.name && startConditions == other.startConditions &&
                    skipConditions == other.skipConditions && finishConditions == other.finishConditions;
            }

            bool operator!=(const ThreadWorkerSettingsState& other) const noexcept {
                return !(*this == other);
            }
        };

        struct SR_COMMON_DLL_API ThreadWorkerThread : public Serializable {
            SR_STRUCT()

            /// @property
            StringAtom name = "Thread";
            /// @property
            bool useThreads = true;
            /// @property
            Vector<ThreadWorkerSettingsState> states;

            bool operator==(const ThreadWorkerThread& other) const noexcept {
                return name == other.name && useThreads == other.useThreads && states == other.states;
            }

            bool operator!=(const ThreadWorkerThread& other) const noexcept {
                return !(*this == other);
            }
        };

        struct SR_COMMON_DLL_API ThreadWorkerThreadsVariant : public Serializable {
            SR_STRUCT()

            /// @property
            String description;
            /// @property
            Set<StringAtom> featuresCondition;
            /// @property
            Vector<ThreadWorkerThread> threads;
            /// @property
            Vector<StringAtom> finalizeStates;

            bool operator==(const ThreadWorkerThreadsVariant& other) const noexcept {
                return description == other.description && featuresCondition == other.featuresCondition &&
                    threads == other.threads && finalizeStates == other.finalizeStates;
            }

            bool operator!=(const ThreadWorkerThreadsVariant& other) const noexcept {
                return !(*this == other);
            }
        };
    }

    struct SR_COMMON_DLL_API ThreadWorkerSettings : public Asset {
        SR_STRUCT()

        /// @property
        Vector<Details::ThreadWorkerThreadsVariant> variants;

    };
}

#endif //SR_ENGINE_UTILS_THREAD_WORKER_SETTINGS_H
